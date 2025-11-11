// apps/processX_app_sokol.cpp
// Sokol-based entry point for ProcessX with HighDPI support

// Define SOKOL_IMPL to get implementations
#define SOKOL_IMPL

#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_log.h"
#include "sokol_glue.h"
#include "sokol_time.h"
#include "imgui.h"
#define SOKOL_IMGUI_IMPL
#include "util/sokol_imgui.h"
#include "imgui_internal.h"  // Needed for DockBuilder functions (must be after imgui.h)
#include <iostream>
#include <cstdio>
#include <fstream>
#include <vector>

// Note: gui_docking.h uses hello_imgui types, so we'll handle docking manually
// #include "gui_docking.h"
#include "gui_common.h"
#include "themes.h"
#include "gui_log.h"
#include "gui_palette.h"
#include "gui_flowsheet.h"
#include "gui_properties.h"
#include "gui_fluids.h"

#ifdef EMSCRIPTEN
#include <emscripten.h>

EM_JS(void, notify_ready, (), {
  try { parent.postMessage({ type: 'wasmReady' }, window.location.origin); } catch (e) {}
});
#endif

static sg_pass_action pass_action;
static bool show_quit_dialog = false;
static bool app_initialized = false;

void ShowGui() {
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
  
  ShowLogWindow();
}

void ShowMenus() {
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
      sapp_request_quit();
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

void init(void) {
  // Setup sokol-gfx
  sg_desc desc = { };
  desc.environment = sglue_environment();
  desc.logger.func = slog_func;
  sg_setup(&desc);

  // Setup sokol-imgui with HighDPI support
  simgui_desc_t simgui_desc = { };
  simgui_desc.logger.func = slog_func;
  simgui_setup(&simgui_desc);

  // Configure ImGui for HighDPI rendering
  ImGuiIO& io = ImGui::GetIO();
  io.ConfigDpiScaleFonts = true;
  io.ConfigDpiScaleViewports = true;
  
  // Enable docking and viewports
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

  // Load DejaVuSans as the primary font
  // Static storage to keep font data alive (required when FontDataOwnedByAtlas = false)
  static std::vector<unsigned char> font_data_storage;
  
  std::ifstream file("assets/fonts/Roboto-VariableFont_wdth,wght.ttf", std::ios::binary | std::ios::ate);
  if (file.is_open()) {
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    font_data_storage.resize(size);
    
    if (file.read(reinterpret_cast<char*>(font_data_storage.data()), size)) {
      ImFontConfig config;
      config.FontDataOwnedByAtlas = false;
      config.MergeMode = false;
      config.PixelSnapH = true;
      
      ImFont* main_font = io.Fonts->AddFontFromMemoryTTF(
        font_data_storage.data(), 
        static_cast<int>(font_data_storage.size()), 
        16.0f, 
        &config
      );
      
      if (main_font) {
        io.FontDefault = main_font;
      } else {
        io.Fonts->AddFontDefault();
      }
    } else {
      io.Fonts->AddFontDefault();
    }
    file.close();
  } else {
    io.Fonts->AddFontDefault();
  }
  
  // Build font atlas - sokol_imgui will automatically handle texture updates
  // via Dear ImGui's texture update callback (no need to call GetTexDataAsRGBA32)
  io.Fonts->Build();
  
  // Note: sokol_imgui automatically handles font atlas texture updates
  // when the atlas is rebuilt. No manual texture management needed.

  // Initial clear color
  pass_action.colors[0].load_action = SG_LOADACTION_CLEAR;
  pass_action.colors[0].clear_value = { 0.1f, 0.1f, 0.1f, 1.0f };

  // Note: Docking layout will be set up automatically by ImGui on first use
  // We don't need CreateDefaultLayout() here since it's hello_imgui-specific

  app_initialized = true;

  #ifdef EMSCRIPTEN
  // Notify parent that WASM app is ready after initialization
  notify_ready();
  #endif
}

void frame(void) {
  const int width = sapp_width();
  const int height = sapp_height();
  
  // New ImGui frame with HighDPI support
  // sapp_dpi_scale() returns the device pixel ratio for proper HighDPI rendering
  simgui_new_frame({ width, height, sapp_frame_duration(), sapp_dpi_scale() });

  // Show menu bar
  if (ImGui::BeginMainMenuBar()) {
    ShowMenus();
    ImGui::EndMainMenuBar();
  }

  // Create fullscreen dock space
  ImGuiViewport* viewport = ImGui::GetMainViewport();
  ImGui::SetNextWindowPos(viewport->WorkPos);
  ImGui::SetNextWindowSize(viewport->WorkSize);
  ImGui::SetNextWindowViewport(viewport->ID);
  
  // Remove ImGuiWindowFlags_MenuBar to avoid duplicate menu bar
  ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
  window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
  window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
  
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  ImGui::Begin("DockSpace", nullptr, window_flags);
  ImGui::PopStyleVar();

  ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
  
  // Set up default docking layout on first frame
  static bool layout_initialized = false;
  if (!layout_initialized) {
    layout_initialized = true;
    
    // Clear any existing docking setup and create new node
    ImGui::DockBuilderRemoveNode(dockspace_id);
    ImGuiID dock_main_id = ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dock_main_id, viewport->WorkSize);
    
    // Split main dockspace left to create Log area (25% width)
    ImGuiID dock_id_left;
    ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.25f, &dock_id_left, &dock_main_id);
    
    // Split main dockspace down to create bottom area for Palette (25% height)
    ImGuiID dock_id_bottom;
    ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.25f, &dock_id_bottom, &dock_main_id);
    
    // Split main dockspace right to create Properties area (50% width of top right)
    ImGuiID dock_id_properties;
    ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.50f, &dock_id_properties, &dock_main_id);
    
    // Dock windows to their respective spaces
    ImGui::DockBuilderDockWindow("Log", dock_id_left);
    ImGui::DockBuilderDockWindow("Flowsheet", dock_main_id);
    ImGui::DockBuilderDockWindow("Properties", dock_id_properties);
    ImGui::DockBuilderDockWindow("Unit Operations", dock_id_bottom);
    ImGui::DockBuilderDockWindow("Fluid Packages", dock_id_properties);
    
    // Finish building the dock space
    ImGui::DockBuilderFinish(dockspace_id);
  }
  
  ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);

  // Show main GUI
  ShowGui();

  ImGui::End();

  // Render ImGui
  sg_pass pass = {};
  pass.action = pass_action;
  pass.swapchain = sglue_swapchain();
  sg_begin_pass(&pass);
  simgui_render();
  sg_end_pass();
  sg_commit();
}

void cleanup(void) {
  simgui_shutdown();
  sg_shutdown();
}

void input(const sapp_event* event) {
  if (event->type == SAPP_EVENTTYPE_QUIT_REQUESTED) {
    // Allow quit - don't cancel it
    sapp_quit();
  }
  else {
    simgui_handle_event(event);
  }
}

sapp_desc sokol_main(int argc, char* argv[]) {
  (void)argc;
  (void)argv;
  sapp_desc desc = { };
  desc.init_cb = init;
  desc.frame_cb = frame;
  desc.cleanup_cb = cleanup;
  desc.event_cb = input;
  desc.width = 1920;
  desc.height = 1080;
  desc.fullscreen = false;  // Start windowed
  desc.high_dpi = true;  // Enable HighDPI support
  desc.html5_ask_leave_site = false;
  desc.ios_keyboard_resizes_canvas = false;
  desc.window_title = "ProcessX (Sokol)";
  desc.icon.sokol_default = true;
  desc.enable_clipboard = true;
  desc.logger.func = slog_func;
  return desc;
}

