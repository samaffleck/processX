// apps/processx_app.cpp
#include <hello_imgui/hello_imgui.h>
#include <imgui.h>

#include <vector>
#include <algorithm>
#include <string>
#include <filesystem>
#include <variant>
#include <optional>
#include <cstdint>
#include <cstring>
#include <cstdio>

#include "processX/flowsheet.h" 

#include "node_editor.h"

static px::Flowsheet flowsheet{};

// Selection state - tracks which unit is currently selected
enum class SelectionType { None, Valve, Mixer, Stream };
struct Selection {
  SelectionType type = SelectionType::None;
  uint32_t index = UINT32_MAX;
  
  bool valid() const { return type != SelectionType::None && index != UINT32_MAX; }
  void clear() { type = SelectionType::None; index = UINT32_MAX; }
};
static Selection selected_unit{};

static void ShowUnitOperationsPalette() {
  ImGui::Text("Unit Operations");
  ImGui::Separator();

  if (ImGui::Button("Mixer")) {
    flowsheet.add<px::Mixer>();
    selected_unit.clear(); // Clear selection when adding new unit
  }
  ImGui::SameLine();
  if (ImGui::Button("Valve")) {
    flowsheet.add<px::Valve>();
    selected_unit.clear();
  }
  ImGui::SameLine();
  if (ImGui::Button("Stream")) {
    flowsheet.add<px::Stream>();
    selected_unit.clear();
  }
}

// Helper function to build stream list for dropdown
struct StreamItem {
  px::Handle<px::Stream> handle;
  std::string display_name;
};

static std::vector<StreamItem> GetStreamList() {
  std::vector<StreamItem> streams;
  streams.push_back({px::Handle<px::Stream>{}, "(None)"}); // Option to disconnect
  
  flowsheet.streams_.for_each_with_handle([&](px::Stream& stream, px::Handle<px::Stream> handle) {
    std::string display_name = stream.name.empty() ? "(Unnamed Stream)" : stream.name;
    streams.push_back({handle, display_name});
  });
  
  return streams;
}

// Helper to show stream dropdown and update handle
static bool StreamCombo(const char* label, px::Handle<px::Stream>& current_handle, const std::vector<StreamItem>& streams) {
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
  
  if (ImGui::BeginCombo(label, combo_preview.c_str())) {
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
        ImGui::EndCombo();
        return true;
      }
      if (is_selected) {
        ImGui::SetItemDefaultFocus();
      }
    }
    ImGui::EndCombo();
  }
  return false;
}

static void ShowValveProperties(px::Valve& valve) {
  ImGui::Text("Valve Properties");
  ImGui::Separator();

  // Name editing
  char name_buffer[256];
  strncpy(name_buffer, valve.get_name().c_str(), sizeof(name_buffer));
  name_buffer[sizeof(name_buffer) - 1] = '\0';
  if (ImGui::InputText("Name", name_buffer, sizeof(name_buffer))) {
    valve.name = name_buffer;
  }

  ImGui::Spacing();

  // Cv editing
  ImGui::Text("Flow Coefficient (Cv):");
  ImGui::PushItemWidth(200);
  if (ImGui::InputDouble("##Cv", &valve.Cv.value)) {
    // Value updated
  }
  ImGui::SameLine();
  ImGui::Checkbox("Fixed##CvFixed", &valve.Cv.fixed);
  ImGui::PopItemWidth();

  ImGui::Spacing();

  // Connections with dropdowns
  ImGui::Text("Connections:");
  
  // Build stream list
  std::vector<StreamItem> stream_list = GetStreamList();
  
  // Inlet dropdown
  ImGui::Text("Inlet:");
  ImGui::SameLine(100);
  StreamCombo("##ValveInlet", valve.in, stream_list);
  
  // Outlet dropdown
  ImGui::Text("Outlet:");
  ImGui::SameLine(100);
  StreamCombo("##ValveOutlet", valve.out, stream_list);
}

static void ShowMixerProperties(px::Mixer& mixer) {
  ImGui::Text("Mixer Properties");
  ImGui::Separator();

  // Name editing
  char name_buffer[256];
  strncpy(name_buffer, mixer.get_name().c_str(), sizeof(name_buffer));
  name_buffer[sizeof(name_buffer) - 1] = '\0';
  if (ImGui::InputText("Name", name_buffer, sizeof(name_buffer))) {
    mixer.name = name_buffer;
  }

  ImGui::Spacing();

  // Connections with dropdowns
  ImGui::Text("Connections:");
  
  // Build stream list
  std::vector<StreamItem> stream_list = GetStreamList();
  
  // Inlets - list with dropdowns
  ImGui::Text("Inlets:");
  for (size_t i = 0; i < mixer.in.size(); ++i) {
    ImGui::PushID(static_cast<int>(i));
    ImGui::Text("  [%zu]", i);
    ImGui::SameLine(50);
    char label[32];
    snprintf(label, sizeof(label), "##MixerInlet%zu", i);
    
    px::Handle<px::Stream>& inlet_handle = mixer.in[i];
    if (StreamCombo(label, inlet_handle, stream_list)) {
      // Connection updated
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Remove##RemoveInlet")) {
      mixer.in.erase(mixer.in.begin() + i);
      ImGui::PopID();
      break; // Exit loop since vector changed
    }
    ImGui::PopID();
  }
  
  if (ImGui::Button("Add Inlet")) {
    mixer.in.push_back(px::Handle<px::Stream>{});
  }
  
  ImGui::Spacing();
  
  // Outlet dropdown
  ImGui::Text("Outlet:");
  ImGui::SameLine(100);
  StreamCombo("##MixerOutlet", mixer.out, stream_list);
}

static void ShowStreamProperties(px::Stream& stream) {
  ImGui::Text("Stream Properties");
  ImGui::Separator();

  // Name editing
  char name_buffer[256];
  strncpy(name_buffer, stream.name.c_str(), sizeof(name_buffer));
  name_buffer[sizeof(name_buffer) - 1] = '\0';
  if (ImGui::InputText("Name", name_buffer, sizeof(name_buffer))) {
    stream.name = name_buffer;
  }

  ImGui::Spacing();

  // Properties editing
  ImGui::Text("Properties:");
  
  ImGui::PushItemWidth(200);
  
  ImGui::Text("Molar Flowrate (mol/s):");
  if (ImGui::InputDouble("##MolarFlow", &stream.molar_flow.value)) {
    // Value updated
  }
  ImGui::SameLine();
  ImGui::Checkbox("Fixed##MolarFlowFixed", &stream.molar_flow.fixed);

  ImGui::Text("Pressure (Pa):");
  if (ImGui::InputDouble("##Pressure", &stream.pressure.value)) {
    // Value updated
  }
  ImGui::SameLine();
  ImGui::Checkbox("Fixed##PressureFixed", &stream.pressure.fixed);

  ImGui::Text("Temperature (K):");
  if (ImGui::InputDouble("##Temperature", &stream.temperature.value)) {
    // Value updated
  }
  ImGui::SameLine();
  ImGui::Checkbox("Fixed##TemperatureFixed", &stream.temperature.fixed);

  ImGui::PopItemWidth();
}

static void ShowSelectedUnitProperties() {
  if (!selected_unit.valid()) {
    ImGui::Text("No unit selected");
    ImGui::Text("Select a unit from the flowsheet list to view/edit its properties.");
    return;
  }

  // Get the selected unit based on type and index
  bool found = false;
  if (selected_unit.type == SelectionType::Valve) {
    uint32_t idx = 0;
    flowsheet.valves_.for_each([&](px::Valve& valve) {
      if (!found && idx == selected_unit.index) {
        ShowValveProperties(valve);
        found = true;
      }
      idx++;
    });
  } else if (selected_unit.type == SelectionType::Mixer) {
    uint32_t idx = 0;
    flowsheet.mixers_.for_each([&](px::Mixer& mixer) {
      if (!found && idx == selected_unit.index) {
        ShowMixerProperties(mixer);
        found = true;
      }
      idx++;
    });
  } else if (selected_unit.type == SelectionType::Stream) {
    uint32_t idx = 0;
    flowsheet.streams_.for_each([&](px::Stream& stream) {
      if (!found && idx == selected_unit.index) {
        ShowStreamProperties(stream);
        found = true;
      }
      idx++;
    });
  }
  
  if (!found) {
    ImGui::Text("Selected unit not found. It may have been deleted.");
    selected_unit.clear();
  }
}

static void ShowUnitOperations() {
  ImGui::Text("Flowsheet Units");
  ImGui::Separator();

  flowsheet.build_unit_list();

  // Group by type using collapsible headers
  // Valves
  if (flowsheet.valves_.size_alive() > 0) {
    if (ImGui::CollapsingHeader(("Valves (" + std::to_string(flowsheet.valves_.size_alive()) + ")").c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
      uint32_t idx = 0;
      flowsheet.valves_.for_each([&](px::Valve& valve) {
        bool is_selected = selected_unit.type == SelectionType::Valve && selected_unit.index == idx;
        std::string name = valve.get_name().empty() ? "(Unnamed Valve)" : valve.get_name();
        if (ImGui::Selectable(name.c_str(), is_selected)) {
          selected_unit.type = SelectionType::Valve;
          selected_unit.index = idx;
        }
        idx++;
      });
    }
  }

  // Mixers
  if (flowsheet.mixers_.size_alive() > 0) {
    if (ImGui::CollapsingHeader(("Mixers (" + std::to_string(flowsheet.mixers_.size_alive()) + ")").c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
      uint32_t idx = 0;
      flowsheet.mixers_.for_each([&](px::Mixer& mixer) {
        bool is_selected = selected_unit.type == SelectionType::Mixer && selected_unit.index == idx;
        std::string name = mixer.get_name().empty() ? "(Unnamed Mixer)" : mixer.get_name();
        if (ImGui::Selectable(name.c_str(), is_selected)) {
          selected_unit.type = SelectionType::Mixer;
          selected_unit.index = idx;
        }
        idx++;
      });
    }
  }

  // Streams
  if (flowsheet.streams_.size_alive() > 0) {
    if (ImGui::CollapsingHeader(("Streams (" + std::to_string(flowsheet.streams_.size_alive()) + ")").c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
      uint32_t idx = 0;
      flowsheet.streams_.for_each([&](px::Stream& stream) {
        bool is_selected = selected_unit.type == SelectionType::Stream && selected_unit.index == idx;
        std::string name = stream.name.empty() ? "(Unnamed Stream)" : stream.name;
        if (ImGui::Selectable(name.c_str(), is_selected)) {
          selected_unit.type = SelectionType::Stream;
          selected_unit.index = idx;
        }
        idx++;
      });
    }
  }

  if (flowsheet.valves_.size_alive() == 0 && 
      flowsheet.mixers_.size_alive() == 0 && 
      flowsheet.streams_.size_alive() == 0) {
    ImGui::Text("No units in flowsheet");
    ImGui::Text("Add units using the Unit Operations palette.");
  }
}

static void ShowMainDock() {
  static bool on_first_load = true;
  if (on_first_load) {
    on_first_load = false;
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  }

  ImGui::Begin("Unit Operations");
  ShowUnitOperationsPalette();
  ImGui::End();

  ImGui::Begin("Flowsheet");
  ShowUnitOperations();
  ImGui::End();

  ImGui::Begin("Properties");
  ShowSelectedUnitProperties();
  ImGui::End();
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
