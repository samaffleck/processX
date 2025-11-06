#pragma once

#include <hello_imgui/hello_imgui.h>
#include <vector>

std::vector<HelloImGui::DockingSplit> CreateDefaultDockingSplits();
std::vector<HelloImGui::DockableWindow> CreateDockableWindows();
HelloImGui::DockingParams CreateDefaultLayout();

