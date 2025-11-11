// apps/processx_app.cpp
#include <hello_imgui/hello_imgui.h>
#include <imgui.h>
#include <iostream>
#include <cstdio>

#include "gui_docking.h"
#include "gui_common.h"
#include "themes.h"
#include "gui_log.h"
#include "gui_palette.h"
#include "gui_flowsheet.h"
#include "gui_properties.h"
#include "gui_fluids.h"
#include "gui_chat.h"


#ifdef EMSCRIPTEN
#include <emscripten.h>

EM_JS(void, notify_ready, (), {
  try { parent.postMessage({ type: 'wasmReady' }, window.location.origin); } catch (e) {}
});
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
}

void ShowMenus(HelloImGui::RunnerParams& params) {
  // File menu
  if (ImGui::BeginMenu("File")) {
    if (ImGui::MenuItem("New", "Ctrl+N")) {
      // TODO: Implement new file functionality
    }
    if (ImGui::MenuItem("Open", "Ctrl+O")) {
      // TODO: Implement open file functionality
    }
    if (ImGui::MenuItem("Save", "Ctrl+S")) {
      // TODO: Implement save file functionality
    }
    ImGui::Separator();
    if (ImGui::MenuItem("Exit", "Ctrl+Q")) {
      params.appShallExit = true;
    }
    ImGui::EndMenu();
  }

  // Edit menu
  if (ImGui::BeginMenu("Edit")) {
    if (ImGui::MenuItem("Undo", "Ctrl+Z")) {
      // TODO: Implement undo functionality
    }
    if (ImGui::MenuItem("Redo", "Ctrl+Y")) {
      // TODO: Implement redo functionality
    }
    ImGui::Separator();
    if (ImGui::MenuItem("Cut", "Ctrl+X")) {
      // TODO: Implement cut functionality
    }
    if (ImGui::MenuItem("Copy", "Ctrl+C")) {
      // TODO: Implement copy functionality
    }
    if (ImGui::MenuItem("Paste", "Ctrl+V")) {
      // TODO: Implement paste functionality
    }
    ImGui::EndMenu();
  }

  // View menu
  if (ImGui::BeginMenu("View")) {
    // Themes submenu
    if (ImGui::BeginMenu("Themes")) {
      if (ImGui::MenuItem("Dark Theme")) {
        Themes::SetDarkTheme();
      }
      if (ImGui::MenuItem("Light Theme")) {
        Themes::SetLightTheme();
      }
      if (ImGui::MenuItem("Bess Dark")) {
        Themes::SetBessDarkColors();
      }
      if (ImGui::MenuItem("Fluent UI")) {
        Themes::SetFluentUIColors();
      }
      if (ImGui::MenuItem("Fluent Light")) {
        Themes::SetFluentLight();
      }
      if (ImGui::MenuItem("Catppuccin Mocha")) {
        Themes::SetCatpuccinMochaColors();
      }
      ImGui::EndMenu();
    }
    ImGui::EndMenu();
  }
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
