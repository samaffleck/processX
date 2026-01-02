#include "gui_common.h"
#include "gui_log.h"
#include "gui_util.h"

#include "processX/user_data.h"

#include <cereal/archives/json.hpp>
#include <sstream>
#include <imgui.h>
#include <iostream>

#ifdef EMSCRIPTEN
#include <emscripten.h>

EM_JS(void, SetupGetFlowsheetJSON_Impl, (), {
  Module.getFlowsheetJSON = function() {
    var ptr = Module._GetFlowsheetJSONPtr();
    if (!ptr) return null;
    
    // Find the length of the null-terminated string
    var length = 0;
    var start = ptr;
    while (HEAP8[ptr]) {
      length++;
      ptr++;
    }
    
    // Read UTF-8 bytes from memory
    var bytes = new Uint8Array(length);
    for (var i = 0; i < length; i++) {
      bytes[i] = HEAP8[start + i];
    }
    
    // Decode UTF-8 to JavaScript string
    // Use TextDecoder if available, otherwise fall back to simple decoding
    if (typeof TextDecoder !== 'undefined') {
      var decoder = new TextDecoder('utf-8');
      return decoder.decode(bytes);
    } else {
      // Fallback: simple ASCII/UTF-8 decoding (works for JSON)
      var result = '';
      for (var i = 0; i < length; i++) {
        result += String.fromCharCode(bytes[i]);
      }
      return result;
    }
  };
});

void SetupGetFlowsheetJSON() {
  SetupGetFlowsheetJSON_Impl();
}

EM_JS(void, SetupLoadFlowsheetJSON_Impl, (), {
  Module.loadFlowsheetJSON = function(jsonString) {
    console.log('[loadFlowsheetJSON] Called with string length:', jsonString ? jsonString.length : 'null');
    
    if (!jsonString || typeof jsonString !== 'string') {
      console.error('[loadFlowsheetJSON] ❌ Invalid JSON string:', typeof jsonString, jsonString);
      return false;
    }
    
    try {
      // Use ccall if available (recommended Emscripten way - now exported)
      if (typeof Module.ccall === 'function') {
        console.log('[loadFlowsheetJSON] Using ccall method');
        var result = Module.ccall('LoadFlowsheetFromJSON', 'number', ['string'], [jsonString]);
        console.log('[loadFlowsheetJSON] ccall returned:', result);
        return result === 1;
      }
      
      // Fallback: use stringToUTF8 with manual allocation (now exported)
      if (typeof Module.stringToUTF8 === 'function' && typeof Module.lengthBytesUTF8 === 'function') {
        console.log('[loadFlowsheetJSON] Using stringToUTF8 method');
        var length = Module.lengthBytesUTF8(jsonString) + 1;
        console.log('[loadFlowsheetJSON] Required buffer length:', length);
        var mallocFunc = Module._malloc || Module.malloc;
        if (typeof mallocFunc !== 'function') {
          console.error('[loadFlowsheetJSON] ❌ malloc function not found');
          return false;
        }
        
        var ptr = mallocFunc(length);
        if (!ptr) {
          console.error('[loadFlowsheetJSON] ❌ Failed to allocate memory');
          return false;
        }
        console.log('[loadFlowsheetJSON] Allocated pointer:', ptr);
        
        Module.stringToUTF8(jsonString, ptr, length);
        var result = Module._LoadFlowsheetFromJSON(ptr);
        console.log('[loadFlowsheetJSON] _LoadFlowsheetFromJSON returned:', result);
        var freeFunc = Module._free || Module.free;
        if (typeof freeFunc === 'function') {
          freeFunc(ptr);
        } else {
          console.warn('[loadFlowsheetJSON] Warning: No free function found, memory leak possible');
        }
        return result === 1;
      }
      
      // Last resort: manual encoding with TextEncoder
      console.log('[loadFlowsheetJSON] Using manual TextEncoder method');
      var encoder = new TextEncoder();
      var utf8Bytes = encoder.encode(jsonString);
      var length = utf8Bytes.length;
      console.log('[loadFlowsheetJSON] UTF-8 encoded length:', length);
      
      var mallocFunc = Module._malloc || Module.malloc;
      if (typeof mallocFunc !== 'function') {
        console.error('[loadFlowsheetJSON] ❌ malloc function not found');
        console.log('[loadFlowsheetJSON] Available methods:', {
          allocateUTF8: typeof Module.allocateUTF8,
          stringToUTF8: typeof Module.stringToUTF8,
          lengthBytesUTF8: typeof Module.lengthBytesUTF8,
          _malloc: typeof Module._malloc,
          malloc: typeof Module.malloc,
          _LoadFlowsheetFromJSON: typeof Module._LoadFlowsheetFromJSON
        });
        return false;
      }
      
      var ptr = mallocFunc(length + 1);
      if (!ptr) {
        console.error('[loadFlowsheetJSON] ❌ Failed to allocate memory');
        return false;
      }
      console.log('[loadFlowsheetJSON] Allocated pointer:', ptr);
      
      // Access HEAP8 through Module
      var HEAP8 = Module.HEAP8;
      if (!HEAP8) {
        console.error('[loadFlowsheetJSON] ❌ HEAP8 not found');
        var freeFunc = Module._free || Module.free;
        if (typeof freeFunc === 'function') {
          freeFunc(ptr);
        }
        return false;
      }
      
      for (var i = 0; i < length; i++) {
        HEAP8[ptr + i] = utf8Bytes[i];
      }
      HEAP8[ptr + length] = 0; // Null terminator
      console.log('[loadFlowsheetJSON] Copied bytes to WASM memory');
      
      var result = Module._LoadFlowsheetFromJSON(ptr);
      console.log('[loadFlowsheetJSON] _LoadFlowsheetFromJSON returned:', result);
      var freeFunc = Module._free || Module.free;
      if (typeof freeFunc === 'function') {
        freeFunc(ptr);
      } else {
        console.warn('[loadFlowsheetJSON] Warning: No free function found, memory leak possible');
      }
      return result === 1;
    } catch (error) {
      console.error('[loadFlowsheetJSON] ❌ Exception caught:', error);
      console.error('[loadFlowsheetJSON] Error stack:', error.stack);
      return false;
    }
  };
});

void SetupLoadFlowsheetJSON() {
  SetupLoadFlowsheetJSON_Impl();
}
#endif

// Global user data instance (contains flowsheet and solver settings)
px::UserData user_data{};

// Global selection state
Selection selected_unit{};

// Flag to show data loaded modal
static bool show_data_loaded_modal = false;

// Function to serialize user data to JSON string
std::string GetFlowsheetJSONString() {
  std::ostringstream oss;
  {
    cereal::JSONOutputArchive archive(oss);
    archive(cereal::make_nvp("UserData", user_data));
  }
  return oss.str();
}

bool LoadFlowsheetFromJSONString(const std::string& json_string) {
  std::cerr << "[LoadFlowsheetFromJSONString] Starting load process..." << std::endl;
  std::cerr << "[LoadFlowsheetFromJSONString] JSON string length: " << json_string.length() << std::endl;
  
  if (json_string.empty()) {
    std::cerr << "[LoadFlowsheetFromJSONString] ERROR: JSON string is empty!" << std::endl;
    // Don't show error in UI for empty JSON - just log to stderr
    return false;
  }
  
  // Log first 500 characters for debugging
  std::string preview = json_string.substr(0, std::min(500UL, json_string.length()));
  std::cerr << "[LoadFlowsheetFromJSONString] JSON preview (first 500 chars): " << preview << std::endl;

  // For debugging: log full JSON if it's reasonably sized
  if (json_string.length() < 5000) {
    std::cerr << "[LoadFlowsheetFromJSONString] FULL JSON:\n" << json_string << std::endl;
  }
  
  try {
    std::cerr << "[LoadFlowsheetFromJSONString] Creating input stream..." << std::endl;
    std::istringstream iss(json_string);

    std::cerr << "[LoadFlowsheetFromJSONString] Creating JSON archive..." << std::endl;
    cereal::JSONInputArchive archive(iss);

    // Try to detect the JSON format by checking if it has "UserData" or "Flowsheet_Data" at the top level
    // Old format: { "Flowsheet_Data": {...} }
    // New format: { "UserData": { "Flowsheet_Data": {...}, "KINSOL_Solver_Settings": {...} } }
    bool is_old_format = json_string.find("\"UserData\"") == std::string::npos &&
                         json_string.find("\"Flowsheet_Data\"") != std::string::npos;

    if (is_old_format) {
      std::cerr << "[LoadFlowsheetFromJSONString] Detected OLD format (Flowsheet_Data at top level), deserializing flowsheet only..." << std::endl;
      archive(cereal::make_nvp("Flowsheet_Data", user_data.fs));
      // Initialize newton_options to default values
      user_data.newton_options = px::NewtonOptions{};
    } else {
      std::cerr << "[LoadFlowsheetFromJSONString] Detected NEW format (UserData wrapper), deserializing user data..." << std::endl;
      archive(cereal::make_nvp("UserData", user_data));
    }

    std::cerr << "[LoadFlowsheetFromJSONString] Deserialization successful!" << std::endl;

    // Rebuild name counters from loaded data to prevent duplicate names
    std::cerr << "[LoadFlowsheetFromJSONString] Rebuilding name counters..." << std::endl;
    user_data.fs.rebuild_name_counters();
    std::cerr << "[LoadFlowsheetFromJSONString] Name counters rebuilt" << std::endl;

    // Clear selection when loading new data
    selected_unit.clear();
    std::cerr << "[LoadFlowsheetFromJSONString] Selection cleared" << std::endl;
    
    // Set flag to show modal
    show_data_loaded_modal = true;
    std::cerr << "[LoadFlowsheetFromJSONString] Modal flag set" << std::endl;
    
    // Add log entry
    AddLogEntry(LogEntry::Success, "Flowsheet data loaded successfully");
    std::cerr << "[LoadFlowsheetFromJSONString] ✅ Load completed successfully!" << std::endl;
    
    return true;
  } catch (const cereal::Exception& e) {
    std::cerr << "[LoadFlowsheetFromJSONString] ❌ Cereal exception: " << e.what() << std::endl;
    // Don't show error in UI - just log to stderr for debugging
    // This prevents confusing error messages for empty/new flowsheets
    return false;
  } catch (const std::exception& e) {
    std::cerr << "[LoadFlowsheetFromJSONString] ❌ Standard exception: " << e.what() << std::endl;
    std::cerr << "[LoadFlowsheetFromJSONString] Exception type: " << typeid(e).name() << std::endl;
    // Don't show error in UI - just log to stderr for debugging
    return false;
  } catch (...) {
    std::cerr << "[LoadFlowsheetFromJSONString] ❌ Unknown exception caught!" << std::endl;
    // Don't show error in UI - just log to stderr for debugging
    return false;
  }
}

bool ShouldShowDataLoadedModal() {
  return show_data_loaded_modal;
}

void ClearDataLoadedModal() {
  show_data_loaded_modal = false;
}

#ifdef EMSCRIPTEN
// Internal C function that returns JSON string pointer
extern "C" {
  EMSCRIPTEN_KEEPALIVE
  const char* GetFlowsheetJSONPtr() {
    static std::string json_str;
    json_str = GetFlowsheetJSONString();
    return json_str.c_str();
  }
  
  EMSCRIPTEN_KEEPALIVE
  int LoadFlowsheetFromJSON(const char* json_string) {
    if (!json_string) {
      return 0; // Failure
    }
    bool success = LoadFlowsheetFromJSONString(std::string(json_string));
    return success ? 1 : 0;
  }
}
#endif

// Helper function to build stream list for dropdown
std::vector<StreamItem> GetStreamList() {
  std::vector<StreamItem> streams;
  streams.push_back({px::Handle<px::Stream>{}, "(None)"}); // Option to disconnect
  
  user_data.fs.streams_.for_each_with_handle([&](px::Stream& stream, px::Handle<px::Stream> handle) {
    std::string display_name = stream.name.empty() ? "(Unnamed Stream)" : stream.name;
    streams.push_back({handle, display_name});
  });
  
  return streams;
}

// Helper to show stream dropdown and update handle
bool StreamCombo(const char* label, px::Handle<px::Stream>& current_handle, const std::vector<StreamItem>& streams) {
  ImGui::PushID(&current_handle); // unique + stable per field
  
  // Find current selection index
  int current_selection = 0;
  if (current_handle.valid()) {
    for (size_t i = 1; i < streams.size(); ++i) {
      if (streams[i].handle.index == current_handle.index && 
          streams[i].handle.generation == current_handle.generation) {
        current_selection = static_cast<int>(i);
        break;
      }
    }
  }
  
  // Build combo preview string
  std::string combo_preview = current_selection == 0 ? "(None)" : streams[current_selection].display_name;
  
  bool changed = false;
  
  ImGuiTableFlags flags = ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_NoBordersInBody;
  
  if (ImGui::BeginTable("##stream_combo_table", 2, flags, ImVec2(-FLT_MIN, 0.0f))) {
    ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthStretch, col1_width);
    ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch, 1 - col1_width);
    
    ImGui::TableNextColumn();
    ImGui::TextUnformatted(label);
    
    ImGui::TableNextColumn();
    ImGui::SetNextItemWidth(-FLT_MIN); // Fill entire column
    
    if (ImGui::BeginCombo("##stream_combo", combo_preview.c_str())) {
      for (size_t i = 0; i < streams.size(); ++i) {
        bool is_selected = (current_selection == static_cast<int>(i));
        if (ImGui::Selectable(streams[i].display_name.c_str(), is_selected)) {
          if (i == 0) {
            // Selected "(None)" - disconnect
            current_handle = px::Handle<px::Stream>{};
          } else {
            // Selected a stream - connect it
            current_handle = streams[i].handle;
          }
          changed = true;
          ImGui::EndCombo();
          ImGui::EndTable();
          ImGui::PopID();
          return true;
        }
        if (is_selected) {
          ImGui::SetItemDefaultFocus();
        }
      }
      ImGui::EndCombo();
    }
    
    ImGui::EndTable();
  }
  
  ImGui::PopID();
  return changed;
}

// Helper to show stream dropdown with optional delete button
bool StreamComboWithDelete(const char* label, px::Handle<px::Stream>& current_handle, const std::vector<StreamItem>& streams, const std::function<void()>& on_delete) {
  ImGui::PushID(&current_handle); // unique + stable per field
  
  // Find current selection index
  int current_selection = 0;
  if (current_handle.valid()) {
    for (size_t i = 1; i < streams.size(); ++i) {
      if (streams[i].handle.index == current_handle.index && 
          streams[i].handle.generation == current_handle.generation) {
        current_selection = static_cast<int>(i);
        break;
      }
    }
  }
  
  // Build combo preview string
  std::string combo_preview = current_selection == 0 ? "(None)" : streams[current_selection].display_name;
  
  bool changed = false;
  
  // Determine number of columns based on whether delete button is provided
  int num_columns = on_delete ? 3 : 2;
  ImGuiTableFlags flags = ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_NoBordersInBody;
  
  if (ImGui::BeginTable("##stream_combo_delete_table", num_columns, flags, ImVec2(-FLT_MIN, 0.0f))) {
    ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthStretch, col1_width);
    if (on_delete) {
      ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch, 1.0f - col1_width - 0.05f);
      ImGui::TableSetupColumn("Button", ImGuiTableColumnFlags_WidthFixed, ImGui::GetFrameHeight());
    } else {
      ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch, 1 - col1_width);
    }
    
    ImGui::TableNextColumn();
    ImGui::TextUnformatted(label);
    
    ImGui::TableNextColumn();
    ImGui::SetNextItemWidth(-FLT_MIN); // Fill entire column
    
    if (ImGui::BeginCombo("##stream_combo", combo_preview.c_str())) {
      for (size_t i = 0; i < streams.size(); ++i) {
        bool is_selected = (current_selection == static_cast<int>(i));
        if (ImGui::Selectable(streams[i].display_name.c_str(), is_selected)) {
          if (i == 0) {
            // Selected "(None)" - disconnect
            current_handle = px::Handle<px::Stream>{};
          } else {
            // Selected a stream - connect it
            current_handle = streams[i].handle;
          }
          changed = true;
          ImGui::EndCombo();
          ImGui::EndTable();
          ImGui::PopID();
          return true;
        }
        if (is_selected) {
          ImGui::SetItemDefaultFocus();
        }
      }
      ImGui::EndCombo();
    }
    
    // Add delete button if callback provided
    if (on_delete) {
      ImGui::TableNextColumn();
      float button_size = ImGui::GetFrameHeight();
      if (ImGui::Button("X##Delete", ImVec2(button_size, button_size))) {
        on_delete();
        ImGui::EndTable();
        ImGui::PopID();
        return true; // Indicate that something changed (item was deleted)
      }
    }
    
    ImGui::EndTable();
  }
  
  ImGui::PopID();
  return changed;
}

