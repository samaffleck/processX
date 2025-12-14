#include "gui_menu.h"
#include "gui_common.h"
#include <hello_imgui/runner_params.h>
#include <imgui.h>
#include "themes.h"
#include "hello_imgui/icons_font_awesome_6.h"
#include <cstring>

#ifdef EMSCRIPTEN
#include <emscripten.h>
#include <string>

// Forward declarations for save functions
extern "C" {
  const char* GetFlowsheetJSONPtr();
}

// Static state for save dialog
static bool show_save_dialog = false;
static char change_message_buffer[512] = "";
static std::string current_flowsheet_id = "";
static std::string current_flowsheet_name = "";

// Static state for message popups
static bool show_message_popup = false;
static std::string message_title = "";
static std::string message_text = "";
static bool is_error_message = false;

// Static state for exit confirmation
static bool show_exit_confirmation = false;

// Global state for flowsheet info and lock status (set by JS)
static std::string js_flowsheet_id = "";
static std::string js_flowsheet_name = "";
static int js_lock_status = 0; // 0 = not locked, 1 = locked by current user, 2 = locked by other user

// JS function to get current flowsheet ID (uses global state set by SetCurrentFlowsheetInfo)
const char* GetCurrentFlowsheetId() {
  return js_flowsheet_id.empty() ? nullptr : js_flowsheet_id.c_str();
}

// Get lock status
int GetLockStatus() {
  return js_lock_status;
}

// JS function to toggle lock (calls parent window's lock handler)
EM_JS(void, toggle_lock_js, (), {
  try {
    parent.postMessage({
      type: 'toggleLock'
    }, window.location.origin);
    console.log('[C++ Lock] Toggled lock');
  } catch (e) {
    console.error('[C++ Lock] Failed to toggle lock:', e);
  }
});

// JS function to show lock result message
EM_JS(void, show_lock_message_js, (const char* message, int is_error), {
  const msg = UTF8ToString(message);
  const error = is_error !== 0;
  
  if (typeof Module._OnLockResult === 'function') {
    Module._OnLockResult(msg, error);
  }
});

// JS function to redirect to dashboard
EM_JS(void, redirect_to_dashboard_js, (), {
  try {
    parent.postMessage({
      type: 'redirectToDashboard'
    }, window.location.origin);
    console.log('[C++ Exit] Redirecting to dashboard');
  } catch (e) {
    console.error('[C++ Exit] Failed to redirect:', e);
  }
});

// JS function to make API call to save flowsheet (callback-based for async)
EM_JS(void, save_flowsheet_api_js, (const char* flowsheet_id, const char* name, const char* description, const char* change_message, const char* flowsheet_json, int is_update), {
  const id = UTF8ToString(flowsheet_id);
  const flowsheetName = UTF8ToString(name);
  const flowsheetDesc = UTF8ToString(description);
  const changeMsg = UTF8ToString(change_message);
  const jsonStr = UTF8ToString(flowsheet_json);
  const update = is_update !== 0;
  
  console.log('[C++ Save] Making API call:', {
    id: id || '(new)',
    name: flowsheetName,
    changeMessage: changeMsg,
    nameLength: flowsheetName ? flowsheetName.length : 0,
    jsonLength: jsonStr ? jsonStr.length : 0,
    update: update
  });
  
  // Validate inputs
  if (!flowsheetName || flowsheetName.trim().length === 0) {
    console.error('[C++ Save] Error: Name is empty or whitespace');
    if (typeof Module._OnSaveComplete === 'function') {
      Module._OnSaveComplete(0, 'Name is required', '', '');
    }
    return;
  }
  
  if (!changeMsg || changeMsg.trim().length === 0) {
    console.error('[C++ Save] Error: Change message is empty or whitespace');
    if (typeof Module._OnSaveComplete === 'function') {
      Module._OnSaveComplete(0, 'Change message is required', '', '');
    }
    return;
  }
  
  if (!jsonStr || jsonStr.length === 0) {
    console.error('[C++ Save] Error: Flowsheet JSON is empty');
    if (typeof Module._OnSaveComplete === 'function') {
      Module._OnSaveComplete(0, 'Flowsheet data is empty', '', '');
    }
    return;
  }
  
  const url = update && id ? `/api/flowsheets/${id}` : '/api/flowsheets';
  const method = update && id ? 'PATCH' : 'POST';
  
  const requestBody = {
    name: flowsheetName.trim(),
    description: flowsheetDesc ? flowsheetDesc.trim() : '',
    data: jsonStr,
    dataFormat: 'json_string',
    changeDescription: changeMsg.trim() // Always include change description (required)
  };
  
  console.log('[C++ Save] Request body:', {
    name: requestBody.name,
    nameLength: requestBody.name.length,
    changeDescription: requestBody.changeDescription,
    dataLength: requestBody.data.length,
    hasData: !!requestBody.data
  });
  
  fetch(url, {
    method: method,
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify(requestBody),
  })
  .then(response => {
    if (!response.ok) {
      return response.text().then(text => {
        throw new Error(`API error: ${response.status} ${text}`);
      });
    }
    return response.json();
  })
  .then(result => {
    console.log('[C++ Save] Save successful:', result);
    // Call C++ callback with success
    if (typeof Module._OnSaveComplete === 'function') {
      Module._OnSaveComplete(1, '', result.flowsheet ? result.flowsheet.id : '', result.flowsheet ? result.flowsheet.name : ''); 
    }
    // Also notify parent window (React component) so it can update state
    try {
      parent.postMessage({
        type: 'flowsheetSaved',
        flowsheetId: result.flowsheet ? result.flowsheet.id : null,
        flowsheetName: result.flowsheet ? result.flowsheet.name : null
      }, window.location.origin);
    } catch (e) {
      console.error('[C++ Save] Failed to notify parent:', e);
    }
    // Don't show alert - C++ will show popup
  })
  .catch(error => {
    console.error('[C++ Save] Save failed:', error);
    const errorMsg = error.message || 'Unknown error';
    // Call C++ callback with failure
    if (typeof Module._OnSaveComplete === 'function') {
      Module._OnSaveComplete(0, errorMsg); // 0 = failure, error message
    }
    // Don't show alert - C++ will show popup
  });
});

// C++ function to show save dialog (ImGui modal)
void ShowSaveDialog() {
  if (!show_save_dialog) {
    return;
  }
  
  // Open popup if not already open
  if (!ImGui::IsPopupOpen("Save Flowsheet")) {
    ImGui::OpenPopup("Save Flowsheet");
  }
  
  // Center the dialog
  ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
  ImGui::SetNextWindowSize(ImVec2(500, 200), ImGuiCond_Appearing);
  
  if (ImGui::BeginPopupModal("Save Flowsheet", &show_save_dialog, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove)) {
    std::string title = current_flowsheet_id.empty() ? std::string(ICON_FA_FLOPPY_DISK) + " Save New Flowsheet" : std::string(ICON_FA_FLOPPY_DISK) + " Update Flowsheet";
    ImGui::Text("%s", title.c_str());
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    ImGui::Text("Change Message (required):");
    ImGui::Spacing();
    
    // Input for change message
    ImGui::InputTextMultiline("##ChangeMessage", change_message_buffer, sizeof(change_message_buffer), 
                              ImVec2(480, 80), ImGuiInputTextFlags_None);
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    // Buttons
    bool can_save = strlen(change_message_buffer) > 0;
    
    if (!can_save) {
      ImGui::BeginDisabled();
    }
    
    if (ImGui::Button((std::string(ICON_FA_FLOPPY_DISK) + " Save").c_str(), ImVec2(100, 0))) {
      // Get flowsheet JSON
      const char* json_ptr = GetFlowsheetJSONPtr();
      if (!json_ptr || strlen(json_ptr) == 0) {
        ImGui::OpenPopup("Error");
      } else {
        // Determine if this is an update or new file
        bool is_update = !current_flowsheet_id.empty();

        // Debug logging before save
        EM_ASM({
          console.log('💾 [C++ Save Button] About to call save_flowsheet_api_js with:', {
            current_flowsheet_id: UTF8ToString($0),
            current_flowsheet_name: UTF8ToString($1),
            is_update: $2 !== 0,
            id_empty: UTF8ToString($0).length === 0,
            will_create_new: UTF8ToString($0).length === 0
          });
        }, current_flowsheet_id.c_str(), current_flowsheet_name.c_str(), is_update ? 1 : 0);

        // Call JS to save
        save_flowsheet_api_js(
          current_flowsheet_id.empty() ? nullptr : current_flowsheet_id.c_str(),
          current_flowsheet_name.empty() ? "Untitled Flowsheet" : current_flowsheet_name.c_str(),
          "", // description
          change_message_buffer,
          json_ptr,
          is_update ? 1 : 0
        );

        // Close dialog
        show_save_dialog = false;
        change_message_buffer[0] = '\0';
        ImGui::CloseCurrentPopup();
      }
    }
    
    if (!can_save) {
      ImGui::EndDisabled();
    }
    
    ImGui::SameLine();
    if (ImGui::Button((std::string(ICON_FA_XMARK) + " Cancel").c_str(), ImVec2(100, 0))) {
      show_save_dialog = false;
      change_message_buffer[0] = '\0';
      ImGui::CloseCurrentPopup();
    }
    
    // Error popup
    if (ImGui::BeginPopupModal("Error", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
      ImGui::Text("Failed to get flowsheet data. Please try again.");
      if (ImGui::Button("OK")) {
        ImGui::CloseCurrentPopup();
      }
      ImGui::EndPopup();
    }
    
    ImGui::EndPopup();
  }
}

// C++ function to trigger save dialog
void TriggerSaveDialog() {
  // Use global state set by JS
  current_flowsheet_id = js_flowsheet_id;
  current_flowsheet_name = js_flowsheet_name.empty() ? "Untitled Flowsheet" : js_flowsheet_name;

  // Debug logging
  EM_ASM({
    console.log('💾 [C++ TriggerSaveDialog] Opening save dialog with:', {
      js_flowsheet_id: UTF8ToString($0),
      js_flowsheet_name: UTF8ToString($1),
      current_flowsheet_id: UTF8ToString($2),
      current_flowsheet_name: UTF8ToString($3),
      is_update: UTF8ToString($2).length > 0
    });
  }, js_flowsheet_id.c_str(), js_flowsheet_name.c_str(),
     current_flowsheet_id.c_str(), current_flowsheet_name.c_str());

  // Reset change message buffer
  change_message_buffer[0] = '\0';

  // Show dialog
  show_save_dialog = true;
  ImGui::OpenPopup("Save Flowsheet");
}

// C++ function to get lock status text
std::string GetLockStatusText() {
  int status = js_lock_status;
  if (status == 1) {
    return "Unlock";
  } else if (status == 2) {
    return "Locked by Other";
  } else {
    return "Lock";
  }
}

// C++ function to toggle lock
void ToggleLock() {
  toggle_lock_js();
}

// C++ function to show exit confirmation
void ShowExitConfirmation() {
  show_exit_confirmation = true;
}

// C++ function to exit (redirect to dashboard) - called after confirmation
void ExitToDashboard() {
  redirect_to_dashboard_js();
}

// C++ function to show message popup
void ShowMessagePopup(const std::string& title, const std::string& message, bool is_error) {
  message_title = title;
  message_text = message;
  is_error_message = is_error;
  show_message_popup = true;
}

// C++ callback to set current flowsheet info (called from JS)
extern "C" {
  EMSCRIPTEN_KEEPALIVE
  void SetCurrentFlowsheetInfo(const char* flowsheet_id, const char* flowsheet_name) {
    js_flowsheet_id = flowsheet_id ? std::string(flowsheet_id) : "";
    js_flowsheet_name = flowsheet_name ? std::string(flowsheet_name) : "";

    // Debug logging
    EM_ASM({
      console.log('🔧 [C++ SetCurrentFlowsheetInfo] Called with:', {
        id: UTF8ToString($0),
        name: UTF8ToString($1),
        id_empty: UTF8ToString($0).length === 0,
        name_empty: UTF8ToString($1).length === 0
      });
    }, flowsheet_id ? flowsheet_id : "", flowsheet_name ? flowsheet_name : "");
  }
  
  EMSCRIPTEN_KEEPALIVE
  void SetLockStatus(int status) {
    js_lock_status = status;
  }
  
  // C++ callback function called from JS when save API call completes
  EMSCRIPTEN_KEEPALIVE
  void OnSaveComplete(int success, const char* error_message, const char* flowsheet_id, const char* flowsheet_name) {
    if (success) {
      // Update global flowsheet info
      if (flowsheet_id) {
        js_flowsheet_id = std::string(flowsheet_id);
      }
      if (flowsheet_name) {
        js_flowsheet_name = std::string(flowsheet_name);
      }
      // Show success message
      ShowMessagePopup("Save Successful", "Flowsheet saved successfully!", false);
    } else {
      // Save failed - show error message
      std::string error = error_message ? std::string(error_message) : "Unknown error";
      ShowMessagePopup("Save Failed", "Failed to save flowsheet: " + error, true);
    }
  }
  
  // C++ callback function called from JS when lock operation completes
  EMSCRIPTEN_KEEPALIVE
  void OnLockResult(const char* message, int is_error) {
    std::string msg = message ? std::string(message) : "";
    bool error = is_error != 0;
    std::string title = error ? "Lock Error" : "Lock Status";
    ShowMessagePopup(title, msg, error);
  }
}

#endif // EMSCRIPTEN

void ShowMenus(HelloImGui::RunnerParams& params) {
  // File menu
  if (ImGui::BeginMenu("File")) {
    
    #ifdef EMSCRIPTEN
    // Save menu item
    if (ImGui::MenuItem((std::string(ICON_FA_FLOPPY_DISK) + " Save").c_str(), "Ctrl+S")) {
      TriggerSaveDialog();
    }
    
    // Lock/Unlock menu item
    std::string lock_text = GetLockStatusText();
    std::string lock_icon = (js_lock_status == 1) ? ICON_FA_UNLOCK : ICON_FA_LOCK;
    if (ImGui::MenuItem((lock_icon + " " + lock_text).c_str())) {
      ToggleLock();
    }
    
    ImGui::Separator();
    
    // Exit menu item
    if (ImGui::MenuItem((std::string(ICON_FA_RIGHT_FROM_BRACKET) + " Exit").c_str(), "Ctrl+Q")) {
      ShowExitConfirmation();
    }
    #else
    if (ImGui::MenuItem((std::string(ICON_FA_FILE) + " New").c_str(), "Ctrl+N")) {
      // TODO: Implement new file functionality
    }
    if (ImGui::MenuItem((std::string(ICON_FA_FOLDER_OPEN) + " Open").c_str(), "Ctrl+O")) {
      // TODO: Implement open file functionality
    }
    if (ImGui::MenuItem((std::string(ICON_FA_FLOPPY_DISK) + " Save").c_str(), "Ctrl+S")) {
      // TODO: Implement save file functionality for native builds
    }
    if (ImGui::MenuItem((std::string(ICON_FA_FLOPPY_DISK) + " Save As").c_str(), "Ctrl+Shift+S")) {
      // TODO: Implement save as file functionality for native builds
    }
    ImGui::Separator();
    if (ImGui::MenuItem((std::string(ICON_FA_RIGHT_FROM_BRACKET) + " Exit").c_str(), "Ctrl+Q")) {
      params.appShallExit = true;
    }
    #endif
    
    ImGui::EndMenu();
  }

  // View menu
  if (ImGui::BeginMenu((std::string(ICON_FA_EYE) + " View").c_str())) {
    // Themes submenu
    if (ImGui::BeginMenu("Themes")) {
      if (ImGui::MenuItem((std::string(ICON_FA_MOON) + " Dark Theme").c_str())) {
        Themes::SetFluentUIColors();
      }
      if (ImGui::MenuItem((std::string(ICON_FA_SUN) + " Light Theme").c_str())) {
        Themes::SetFluentLight();
      }
      ImGui::EndMenu();
    }
    ImGui::EndMenu();
  }

  #ifdef EMSCRIPTEN
  // Show flowsheet info centered in menu bar (if a flowsheet is loaded)
  if (!js_flowsheet_name.empty()) {
    // Calculate the text we want to display
    std::string status_text = std::string(ICON_FA_FILE_PEN) + " Currently Editing: " + js_flowsheet_name;

    // Calculate text width
    float text_width = ImGui::CalcTextSize(status_text.c_str()).x;

    // Get menu bar width
    float menu_bar_width = ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x;

    // Calculate spacing to center the text
    // We need to account for the space already used by menus on the left
    float current_x = ImGui::GetCursorPosX();
    float spacing = (menu_bar_width - current_x - text_width) * 0.5f;

    if (spacing > 0) {
      ImGui::SetCursorPosX(current_x + spacing);
    }

    // Display the status text with a subtle style
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.85f, 1.0f, 1.0f)); // Light blue tint
    ImGui::TextUnformatted(status_text.c_str());
    ImGui::PopStyleColor();
  }
  #endif

  #ifdef EMSCRIPTEN
  // Show save dialog if needed
  ShowSaveDialog();
  
  // Show message popup if needed
  if (show_message_popup && !ImGui::IsPopupOpen("Message")) {
    ImGui::OpenPopup("Message");
  }
  
  // Center the message popup
  ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
  ImGui::SetNextWindowSize(ImVec2(400, 150), ImGuiCond_Appearing);
  
  if (ImGui::BeginPopupModal("Message", &show_message_popup, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove)) {
    // Set text color based on error status
    std::string icon = is_error_message ? ICON_FA_TRIANGLE_EXCLAMATION : ICON_FA_CHECK;
    if (is_error_message) {
      ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f)); // Red for errors
    } else {
      ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 1.0f, 0.3f, 1.0f)); // Green for success
    }
    ImGui::Text("%s %s", icon.c_str(), message_title.c_str());
    ImGui::PopStyleColor();
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    // Show message text (wrap if needed)
    ImGui::TextWrapped("%s", message_text.c_str());
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    if (ImGui::Button((std::string(ICON_FA_CHECK) + " OK").c_str(), ImVec2(100, 0))) {
      show_message_popup = false;
      ImGui::CloseCurrentPopup();
    }
    
    ImGui::EndPopup();
  }
  
  // Show exit confirmation dialog if needed
  if (show_exit_confirmation && !ImGui::IsPopupOpen("Exit Confirmation")) {
    ImGui::OpenPopup("Exit Confirmation");
  }
  
  // Center the exit confirmation popup
  ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
  ImGui::SetNextWindowSize(ImVec2(350, 120), ImGuiCond_Appearing);
  
  if (ImGui::BeginPopupModal("Exit Confirmation", &show_exit_confirmation, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove)) {
    ImGui::Text(ICON_FA_RIGHT_FROM_BRACKET " Are you sure you want to exit?");
    ImGui::Spacing();
    ImGui::Text("You will be redirected to the dashboard.");
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    if (ImGui::Button((std::string(ICON_FA_CHECK) + " Yes").c_str(), ImVec2(100, 0))) {
      show_exit_confirmation = false;
      ImGui::CloseCurrentPopup();
      ExitToDashboard();
    }
    
    ImGui::SameLine();
    if (ImGui::Button((std::string(ICON_FA_XMARK) + " No").c_str(), ImVec2(100, 0))) {
      show_exit_confirmation = false;
      ImGui::CloseCurrentPopup();
    }
    
    ImGui::EndPopup();
  }
  #endif
}
