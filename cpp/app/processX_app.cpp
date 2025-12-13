// apps/processx_app.cpp
#include <hello_imgui/hello_imgui.h>
#include <hello_imgui/app_window_params.h>

#include <imgui.h>

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <string>

#include "gui_docking.h"
#include "gui_common.h"
#include "gui_menu.h"
#include "themes.h"
#include "gui_log.h"
#include "gui_palette.h"
#include "gui_flowsheet.h"
#include "gui_properties.h"
#include "gui_fluids.h"
#include "gui_chat.h"


void ShowStatusBar() {
  // Remove rounded borders, but keep border size for top border
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);  // Remove all automatic borders
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 2.0f));  // Reduced vertical padding
  
  float statusWindowHeight = ImGui::GetFrameHeight() * 1.0f;
  ImGuiViewport* viewport = ImGui::GetMainViewport();
  
  ImVec2 statusBarSize, statusBarPos;
  {
    // Position status bar at the bottom of the viewport
    statusBarSize = ImVec2(viewport->Size.x, statusWindowHeight);
    statusBarPos = ImVec2(viewport->Pos.x, viewport->Pos.y + viewport->Size.y - statusBarSize.y);
  }
  
  ImGui::SetNextWindowPos(statusBarPos);
  ImGui::SetNextWindowSize(statusBarSize);
  ImGui::SetNextWindowViewport(viewport->ID);
  
  // Remove NoBackground flag so it has a background
  ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking | 
                                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                                 ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings |
                                 ImGuiWindowFlags_NoNav;
  
  // Frame-based throttling: only assemble every N frames
  static int frame_counter = 0;
  static bool cached_assembled = false;
  static size_t cached_num_unknowns = 0;
  static size_t cached_num_equations = 0;
  static std::string cached_err;
  static const int ASSEMBLE_INTERVAL = 10; // Assemble every 10 frames
  
  frame_counter++;
  bool should_assemble = (frame_counter % ASSEMBLE_INTERVAL == 0);
  
  // Calculate DOF first to determine background color
  std::string err;
  bool assembled;
  size_t num_unknowns;
  size_t num_equations;
  
  if (should_assemble) {
    assembled = flowsheet.assemble(&err);
    num_unknowns = flowsheet.reg.size();
    num_equations = flowsheet.sys.size();
    // Update cache
    cached_assembled = assembled;
    cached_num_unknowns = num_unknowns;
    cached_num_equations = num_equations;
    cached_err = err;
  } else {
    // Use cached values
    assembled = cached_assembled;
    num_unknowns = cached_num_unknowns;
    num_equations = cached_num_equations;
    err = cached_err;
  }
  
  int dof = static_cast<int>(num_unknowns) - static_cast<int>(num_equations);
  
  // Set blue background if DOF == 0, otherwise use default
  if (dof == 0 && assembled) {
    // Use vibrant royal blue (#2160D0) to indicate system is well-posed
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.8f, 0.3f, 0.3f));
  }
  
  if (ImGui::Begin("##StatusBar", nullptr, windowFlags)) {
    // Draw top border line manually
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 window_pos = ImGui::GetWindowPos();
    ImVec2 window_size = ImGui::GetWindowSize();
    ImU32 border_color = ImGui::GetColorU32(ImGuiCol_Border);
    float border_thickness = 1.0f;
    
    // Draw top border line
    draw_list->AddLine(
      ImVec2(window_pos.x, window_pos.y),
      ImVec2(window_pos.x + window_size.x, window_pos.y),
      border_color,
      border_thickness
    );
    
    // Display status information
    ImGui::Text("DOF: %d", dof);
    ImGui::SameLine();
    ImGui::Text("Unknowns: %zu", num_unknowns);
    ImGui::SameLine();
    ImGui::Text("Equations: %zu", num_equations);
    
    // Show error status if assembly failed
    if (!assembled && !err.empty()) {
      ImGui::SameLine();
      // Truncate error message if too long to fit in status bar
      std::string short_err = err;
      if (short_err.length() > 100) {
        short_err = short_err.substr(0, 97) + "...";
      }
      ImGui::TextColored(ImVec4(0.9f, 0.7f, 0.3f, 1.0f), "Error: %s", short_err.c_str());
    }
  }
  ImGui::End();
  
  // Pop the green background color if we pushed it
  if (dof == 0 && assembled) {
    ImGui::PopStyleColor();
  }
  
  ImGui::PopStyleVar(3);
}


#ifdef EMSCRIPTEN
#include <emscripten.h>

EM_JS(void, notify_ready, (), {
  try { parent.postMessage({ type: 'wasmReady' }, window.location.origin); } catch (e) {}
});

// Clipboard functions for Emscripten using browser Clipboard API
EM_JS(void, set_clipboard_text_js, (const char* text), {
  const str = UTF8ToString(text);
  if (navigator.clipboard && navigator.clipboard.writeText) {
    navigator.clipboard.writeText(str).catch(function(err) {
      console.error('Failed to write to clipboard:', err);
      // Fallback to older method
      const textArea = document.createElement('textarea');
      textArea.value = str;
      textArea.style.position = 'fixed';
      textArea.style.left = '-999999px';
      document.body.appendChild(textArea);
      textArea.select();
      try {
        document.execCommand('copy');
      } catch (e) {
        console.error('Fallback copy failed:', e);
      }
      document.body.removeChild(textArea);
    });
  } else {
    // Fallback for browsers without Clipboard API
    const textArea = document.createElement('textarea');
    textArea.value = str;
    textArea.style.position = 'fixed';
    textArea.style.left = '-999999px';
    document.body.appendChild(textArea);
    textArea.select();
    try {
      document.execCommand('copy');
    } catch (e) {
      console.error('Copy failed:', e);
    }
    document.body.removeChild(textArea);
  }
});

// Initialize clipboard cache on module load
EM_JS(void, init_clipboard_cache, (), {
  if (!window._clipboardCache) {
    window._clipboardCache = '';
  }
  // Pre-fetch clipboard content if possible
  if (navigator.clipboard && navigator.clipboard.readText) {
    navigator.clipboard.readText().then(function(text) {
      window._clipboardCache = text || '';
    }).catch(function(err) {
      // Silently fail - clipboard may require user interaction
      window._clipboardCache = '';
    });
  }
});

// Get clipboard text from cache and return as allocated C string
// Caller must free the returned pointer
EM_JS(char*, get_clipboard_text_from_cache, (), {
  // Try to update cache asynchronously (for next time)
  if (navigator.clipboard && navigator.clipboard.readText) {
    navigator.clipboard.readText().then(function(text) {
      window._clipboardCache = text || '';
    }).catch(function(err) {
      // Silently fail
      window._clipboardCache = '';
    });
  }
  
  // Return cached value (may be empty on first call)
  const result = (window._clipboardCache || '');
  const len = lengthBytesUTF8(result) + 1;
  const ptr = _malloc(len);
  stringToUTF8(result, ptr, len);
  return ptr;
});

// Static clipboard buffer for Emscripten (managed by ImGui)
static std::string clipboard_buffer;
#endif


void ShowGui()  {
  // Initialize on first load (runs once)
  static bool on_first_load = true;
  if (on_first_load) {
    on_first_load = false;
    AddLogEntry(LogEntry::Info, "Application started");
    Themes::SetFluentUIColors();
    
    #ifdef EMSCRIPTEN
    SetupGetFlowsheetJSON();
    SetupLoadFlowsheetJSON();
    #endif
  }

  // Show data loaded modal if needed
  if (ShouldShowDataLoadedModal()) {
    ImGui::OpenPopup("Data Loaded");
    // Always center modal when appearing
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
  }
  
  if (ImGui::BeginPopupModal("Data Loaded", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove)) {
    ImGui::Text("Data loaded successfully!");
    ImGui::Spacing();
    if (ImGui::Button("OK", ImVec2(120, 0)) || ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
      ClearDataLoadedModal();
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }

  // Render all dockable windows
  ImGui::Begin("Unit Operations");
  ShowUnitOperationsPalette();
  ImGui::End();

  ImGui::Begin("Flowsheet");
  ShowUnitOperations();
  ImGui::End();

  ImGui::Begin("Properties");
  ShowSelectedUnitProperties();
  ImGui::End();

  ImGui::Begin("Fluid Packages");
  ShowFluidPackagesWindow();
  ImGui::End();
  
  ImGui::Begin("Chat");
  ShowChatWindow();
  ImGui::End();
  
  // Show custom status bar at the bottom
  ShowStatusBar();
}

void InitializeImGuiFonts() {
  HelloImGui::ImGuiDefaultSettings::LoadDefaultFont_WithFontAwesomeIcons();

  // Load DejaVuSans font for math symbols
  // Use hello_imgui's LoadFont helper which handles cross-platform loading automatically
  try {
    // Unicode Math Symbols block
    static const ImWchar ranges[] = { 0x2200, 0x22FF, 0 };
    
    ImFontConfig config;
    config.MergeMode = true; // Merge with existing font
    config.PixelSnapH = true;
    config.GlyphRanges = ranges;
    
    HelloImGui::FontLoadingParams fontParams;
    fontParams.fontConfig = config;
    fontParams.mergeToLastFont = true; // Merge with the last font (default font)
    fontParams.insideAssets = true; // Load from assets folder
    fontParams.adjustSizeToDpi = true; // Adjust for High DPI
    
    ImFont* font = HelloImGui::LoadFont("fonts/Roboto-VariableFont_wdth,wght.ttf", 16.0f, fontParams);
    if (!font) {
      std::cerr << "Warning: Failed to load Roboto-VariableFont_wdth,wght.ttf font" << std::endl;
    }
  } catch (const std::exception& e) {
    std::cerr << "Error loading font: " << e.what() << std::endl;
    // Continue without the custom font
  }
}

void PostInit() {
  ImGuiIO& io = ImGui::GetIO();
  io.ConfigDpiScaleFonts = true;
  io.ConfigDpiScaleViewports = true;
  
  #ifdef EMSCRIPTEN
  // Initialize clipboard cache
  init_clipboard_cache();
  
  // Set up clipboard handlers for Emscripten using browser Clipboard API
  io.SetClipboardTextFn = [](void*, const char* text) {
    set_clipboard_text_js(text);
  };
  io.GetClipboardTextFn = [](void*) -> const char* {
    // Get clipboard text from JavaScript cache
    char* ptr = get_clipboard_text_from_cache();
    if (ptr) {
      // Copy to our static buffer (ImGui will use this)
      clipboard_buffer = std::string(ptr);
      // Free the JavaScript-allocated memory
      free(ptr);
      return clipboard_buffer.c_str();
    }
    clipboard_buffer.clear();
    return clipboard_buffer.c_str();
  };
  io.ClipboardUserData = nullptr;
  #endif
}


int main(int, char**) {
  // Set assets folder first, before configuring callbacks that might need assets
  HelloImGui::SetAssetsFolder("assets");
  
  HelloImGui::RunnerParams params;
  params.callbacks.ShowGui = ShowGui;
  params.appWindowParams.windowTitle = "ProcessX";
  params.callbacks.LoadAdditionalFonts = InitializeImGuiFonts;
  params.callbacks.PostInit = PostInit;
  params.appWindowParams.windowGeometry.windowSizeState = HelloImGui::WindowSizeState::Maximized;
  params.imGuiWindowParams.defaultImGuiWindowType = HelloImGui::DefaultImGuiWindowType::ProvideFullScreenDockSpace;
  params.imGuiWindowParams.enableViewports = true;
  params.imGuiWindowParams.showMenuBar = true;
  params.imGuiWindowParams.showMenu_App = false;
  params.imGuiWindowParams.showMenu_View = false;
  params.imGuiWindowParams.showStatusBar = false;  // Use our custom status bar instead
  params.fpsIdling.enableIdling = false;
  
  // Reserve space at the bottom for the status bar
  // The margin is in em units (multiples of font size)
  // Status bar height is approximately 1.4 * frame height, which is roughly 1.4 em
  params.imGuiWindowParams.fullScreenWindow_MarginBottomRight = ImVec2(0.0f, 1.5f);
  
  params.dpiAwareParams.dpiWindowSizeFactor = 0.0f; // 0 = auto-detect (recommended)
  
  // Set up custom menus
  params.callbacks.ShowMenus = [&params]() { ShowMenus(params); };
  
  // Set up docking layout - this creates the layout structure on first load
  params.dockingParams = CreateDefaultLayout();  

  #ifdef EMSCRIPTEN
  // Notify parent that WASM app is ready after initialization
  notify_ready();
  #endif

  try {
    HelloImGui::Run(params);
  } catch(const std::exception& e) {
    std::cerr << e.what() << '\n';
  }
  
  return 0;
}
