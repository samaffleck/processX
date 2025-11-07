// apps/processx_app.cpp
#include <hello_imgui/hello_imgui.h>
#include <iostream>

#include "gui_main.h"
#include "gui_docking.h"
#include "gui_common.h"

#ifdef EMSCRIPTEN
#include <emscripten.h>

EM_JS(void, notify_ready, (), {
  try { parent.postMessage({ type: 'wasmReady' }, window.location.origin); } catch (e) {}
});
#endif


int main(int, char**) {
  // Set assets folder first, before configuring callbacks that might need assets
  HelloImGui::SetAssetsFolder("assets");
  
  HelloImGui::RunnerParams params;
  params.callbacks.ShowGui = ShowGui;
  params.appWindowParams.windowTitle = "ProcessX";
  params.callbacks.LoadAdditionalFonts = InitializeImGuiFonts;
  params.appWindowParams.windowGeometry.windowSizeState = HelloImGui::WindowSizeState::Maximized;
  params.imGuiWindowParams.defaultImGuiWindowType = HelloImGui::DefaultImGuiWindowType::ProvideFullScreenDockSpace;
  params.imGuiWindowParams.enableViewports = true;
  params.imGuiWindowParams.showMenuBar = false;
  params.imGuiWindowParams.showMenu_App = false;
  params.imGuiWindowParams.showMenu_View = false;
  
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
