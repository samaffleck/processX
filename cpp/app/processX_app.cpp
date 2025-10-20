// apps/processx_app.cpp

#include <hello_imgui/hello_imgui.h>
#include <imgui.h>
#include <string>
#include <filesystem>

#include "processX/plant.h"


// DemoPanelPlant.hpp
#pragma once
#include <imgui.h>
#include <string>
#include <vector>
#include <algorithm>

// your headers:
#include "processX/unitop.h"
#include "processX/stream.h"

// If Plant is in a cpp, expose it via a header, or paste the class here.
#include "processX/plant.h" // assumed

static void ShowPlantDock() {
  ImGui::Begin("Plant");

  ImGui::End();
}


static void ShowMainDock() {
  static bool on_first_load = true;
  
  if (on_first_load) {
    on_first_load = false;
    ImGui::StyleColorsLight();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  }

  ShowPlantDock();
}

void InitializeImGuiFonts() {
  HelloImGui::ImGuiDefaultSettings::LoadDefaultFont_WithFontAwesomeIcons();

  ImFontConfig config;
  config.MergeMode = true;
  config.PixelSnapH = true;

  // Unicode Math Symbols block
  static const ImWchar ranges[] = { 0x2200, 0x22FF, 0 };

  ImGuiIO& io = ImGui::GetIO();
  std::string fontPath = HelloImGui::AssetFileFullPath("fonts/DejaVuSans.ttf");
  io.Fonts->AddFontFromFileTTF(fontPath.c_str(), 20.0f, &config, ranges);
}

int main(int, char**) {
  HelloImGui::RunnerParams params;
  HelloImGui::SetAssetsFolder("assets");  
  params.appWindowParams.windowTitle = "processX";
  params.callbacks.ShowGui = ShowMainDock;
  params.callbacks.LoadAdditionalFonts = InitializeImGuiFonts;
  params.appWindowParams.windowGeometry.windowSizeState = HelloImGui::WindowSizeState::Maximized;
  params.imGuiWindowParams.defaultImGuiWindowType = HelloImGui::DefaultImGuiWindowType::ProvideFullScreenDockSpace;
  params.imGuiWindowParams.enableViewports = true;
  params.imGuiWindowParams.showMenuBar = false;
  params.imGuiWindowParams.showMenu_App = false;
  params.imGuiWindowParams.showMenu_View = false;
  // params.dockingParams = CreateDefaultLayout();
  // #ifdef EMSCRIPTEN
  // notify_ready(); 
  // #endif

  try {
    HelloImGui::Run(params);
  } catch(const std::exception& e) {
    std::cerr << e.what() << '\n';
  }
  
  return 0;
}
