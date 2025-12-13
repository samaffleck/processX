#include "gui_docking.h"
#include "gui_window_titles.h"

std::vector<HelloImGui::DockingSplit> CreateDefaultDockingSplits()
{
  // Layout structure:
  //    ┌──────────┬──────────────────┬──────────────┐
  //    │          │   Flowsheet      │  Properties  │
  //    │   Log    │   (top right)     │  (top right) │
  //    │  (25%)   │                  │              │
  //    │          ├──────────────────┴──────────────┤
  //    │          │                                  │
  //    │          │      Palette (bottom right)      │
  //    │          │                                  │
  //    └──────────┴──────────────────────────────────┘
  
  // Step 1: Split MainDockSpace left to create Log area (25% width)
  // This leaves MainDockSpace as the right side (75% width)
  HelloImGui::DockingSplit splitLeft;
  splitLeft.initialDock = "MainDockSpace";
  splitLeft.newDock = "LogSpace";
  splitLeft.direction = ImGuiDir_Left;
  splitLeft.ratio = 0.25f;
  
  // Step 2: Split MainDockSpace down to create bottom area for Palette (25% height)
  // This leaves MainDockSpace as the top right area (75% height)
  HelloImGui::DockingSplit splitBottom;
  splitBottom.initialDock = "MainDockSpace";
  splitBottom.newDock = "BottomRightSpace";
  splitBottom.direction = ImGuiDir_Down;
  splitBottom.ratio = 0.15f;
  
  // Step 3: Split MainDockSpace (top right) right to create Properties area (50% width of top right)
  // This leaves MainDockSpace as Flowsheet (50% width of top right)
  HelloImGui::DockingSplit splitRight;
  splitRight.initialDock = "MainDockSpace";
  splitRight.newDock = "RightSpace";
  splitRight.direction = ImGuiDir_Right;
  splitRight.ratio = 0.50f;
  
  std::vector<HelloImGui::DockingSplit> splits {splitLeft, splitBottom, splitRight};
  return splits;
}

std::vector<HelloImGui::DockableWindow> CreateDockableWindows()
{
  std::vector<HelloImGui::DockableWindow> windows;
  
  // All windows have empty GuiFunction since we render them manually in ShowGui()
  // The DockableWindow entries are just for docking layout - they tell hello_imgui where to dock windows
  
  // Chat replaces Log - left side (25% width)
  // Note: Use title with icon for docking to match ImGui::Begin calls
  windows.emplace_back(WindowTitles::Chat.c_str(), "LogSpace", []() {}, true, true);
  windows.back().callBeginEnd = false;
  
  // Flowsheet - top right, shares space with Properties (50% of top right)
  windows.emplace_back(WindowTitles::Flowsheet.c_str(), "MainDockSpace", []() {}, true, true);
  windows.back().callBeginEnd = false;
  
  // Properties - top right, shares space with Flowsheet (50% of top right)
  windows.emplace_back(WindowTitles::Properties.c_str(), "RightSpace", []() {}, true, true);
  windows.back().callBeginEnd = false;
  
  // Palette (Unit Operations) - bottom right
  windows.emplace_back(WindowTitles::UnitOperations.c_str(), "BottomRightSpace", []() {}, true, true);
  windows.back().callBeginEnd = false;
  
  // Fluid Packages - can dock anywhere
  windows.emplace_back(WindowTitles::FluidPackages.c_str(), "RightSpace", []() {}, true, true);
  windows.back().callBeginEnd = false;
  
  return windows;
}

HelloImGui::DockingParams CreateDefaultLayout()
{
  HelloImGui::DockingParams dockingParams;
  // Set up docking splits to create the layout structure
  dockingParams.dockingSplits = CreateDefaultDockingSplits();
  // Register dockable windows with their names and dock spaces - this tells hello_imgui where to dock them
  dockingParams.dockableWindows = CreateDockableWindows();
  return dockingParams;
}

