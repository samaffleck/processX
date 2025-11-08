#include "gui_main.h"
#include "gui_common.h"
#include "gui_log.h"
#include "gui_palette.h"
#include "gui_flowsheet.h"
#include "gui_properties.h"
#include "gui_docking.h"
#include "themes.h"
#include <imgui.h>
#include <hello_imgui/hello_imgui.h>
#include <iostream>
#include <cstdio>

void ShowGui()
{
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
  
  ShowLogWindow();
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
    
    ImFont* font = HelloImGui::LoadFont("fonts/DejaVuSans.ttf", 20.0f, fontParams);
    if (!font) {
      std::cerr << "Warning: Failed to load DejaVuSans.ttf font" << std::endl;
    }
  } catch (const std::exception& e) {
    std::cerr << "Error loading font: " << e.what() << std::endl;
    // Continue without the custom font
  }
}

