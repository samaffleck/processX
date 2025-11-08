#include "gui_properties.h"
#include "gui_common.h"
#include "gui_log.h"
#include <imgui.h>
#include <cstring>
#include <cstdio>

void ShowValveProperties(px::Valve& valve) {
  ImGui::Text("Valve Properties");
  ImGui::Separator();

  // Name editing
  char name_buffer[256];
  strncpy(name_buffer, valve.get_name().c_str(), sizeof(name_buffer));
  name_buffer[sizeof(name_buffer) - 1] = '\0';
  if (ImGui::InputText("##Name", name_buffer, sizeof(name_buffer))) {
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

void ShowMixerProperties(px::Mixer& mixer) {
  ImGui::Text("Mixer Properties");
  ImGui::Separator();

  // Name editing
  char name_buffer[256];
  strncpy(name_buffer, mixer.get_name().c_str(), sizeof(name_buffer));
  name_buffer[sizeof(name_buffer) - 1] = '\0';
  if (ImGui::InputText("##Name", name_buffer, sizeof(name_buffer))) {
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

void ShowSplitterProperties(px::Splitter& splitter) {
  ImGui::Text("Splitter Properties");
  ImGui::Separator();

  // Name editing
  char name_buffer[256];
  strncpy(name_buffer, splitter.get_name().c_str(), sizeof(name_buffer));
  name_buffer[sizeof(name_buffer) - 1] = '\0';
  if (ImGui::InputText("##Name", name_buffer, sizeof(name_buffer))) {
    splitter.name = name_buffer;
  }

  ImGui::Spacing();

  // Connections with dropdowns
  ImGui::Text("Connections:");
  
  // Build stream list
  std::vector<StreamItem> stream_list = GetStreamList();
  
  // Inlet dropdown
  ImGui::Text("Inlet:");
  ImGui::SameLine(100);
  StreamCombo("##SplitterInlet", splitter.in, stream_list);
  
  ImGui::Spacing();
  
  // Outlets - list with dropdowns
  ImGui::Text("Outlets:");
  for (size_t i = 0; i < splitter.out.size(); ++i) {
    ImGui::PushID(static_cast<int>(i));
    ImGui::Text("  [%zu]", i);
    ImGui::SameLine(50);
    char label[32];
    snprintf(label, sizeof(label), "##SplitterOutlet%zu", i);
    
    px::Handle<px::Stream>& outlet_handle = splitter.out[i];
    if (StreamCombo(label, outlet_handle, stream_list)) {
      // Connection updated
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Remove##RemoveOutlet")) {
      splitter.out.erase(splitter.out.begin() + i);
      ImGui::PopID();
      break; // Exit loop since vector changed
    }
    ImGui::PopID();
  }
  
  if (ImGui::Button("Add Outlet")) {
    splitter.out.push_back(px::Handle<px::Stream>{});
  }
}

void ShowStreamProperties(px::Stream& stream) {
  ImGui::Text("Stream Properties");
  ImGui::Separator();

  // Name editing
  char name_buffer[256];
  strncpy(name_buffer, stream.name.c_str(), sizeof(name_buffer));
  name_buffer[sizeof(name_buffer) - 1] = '\0';
  if (ImGui::InputText("##Name", name_buffer, sizeof(name_buffer))) {
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

  ImGui::Text("Molar Enthalpy (J/mol):");
  if (ImGui::InputDouble("##MolarEnthalpy", &stream.molar_enthalpy.value)) {
    // Value updated
  }
  ImGui::SameLine();
  ImGui::Checkbox("Fixed##MolarEnthalpyFixed", &stream.molar_enthalpy.fixed);

  ImGui::PopItemWidth();
}

void ShowSimpleHeatExchangerProperties(px::SimpleHeatExchanger& hex) {
  ImGui::Text("SimpleHeatExchanger Properties");
  ImGui::Separator();

  // Name editing
  char name_buffer[256];
  strncpy(name_buffer, hex.get_name().c_str(), sizeof(name_buffer));
  name_buffer[sizeof(name_buffer) - 1] = '\0';
  if (ImGui::InputText("##Name", name_buffer, sizeof(name_buffer))) {
    hex.name = name_buffer;
  }

  ImGui::Spacing();

  // Parameters editing
  ImGui::Text("Parameters:");
  ImGui::PushItemWidth(200);
  
  ImGui::Text("Pressure Drop (Pa):");
  if (ImGui::InputDouble("##dP", &hex.dP.value)) {
    // Value updated
  }
  ImGui::SameLine();
  ImGui::Checkbox("Fixed##dPFixed", &hex.dP.fixed);

  ImGui::Text("Heat Duty (W):");
  if (ImGui::InputDouble("##Q", &hex.Q.value)) {
    // Value updated
  }
  ImGui::SameLine();
  ImGui::Checkbox("Fixed##QFixed", &hex.Q.fixed);

  ImGui::PopItemWidth();

  ImGui::Spacing();

  // Connections with dropdowns
  ImGui::Text("Connections:");
  
  // Build stream list
  std::vector<StreamItem> stream_list = GetStreamList();
  
  // Inlet dropdown
  ImGui::Text("Inlet:");
  ImGui::SameLine(100);
  StreamCombo("##SimpleHXInlet", hex.in, stream_list);
  
  // Outlet dropdown
  ImGui::Text("Outlet:");
  ImGui::SameLine(100);
  StreamCombo("##SimpleHXOutlet", hex.out, stream_list);
}

void ShowHeatExchangerProperties(px::HeatExchanger& hex) {
  ImGui::Text("HeatExchanger Properties");
  ImGui::Separator();

  // Name editing
  char name_buffer[256];
  strncpy(name_buffer, hex.get_name().c_str(), sizeof(name_buffer));
  name_buffer[sizeof(name_buffer) - 1] = '\0';
  if (ImGui::InputText("##Name", name_buffer, sizeof(name_buffer))) {
    hex.name = name_buffer;
  }

  ImGui::Spacing();

  // Parameters editing
  ImGui::Text("Parameters:");
  ImGui::PushItemWidth(200);
  
  ImGui::Text("Pressure Drop Hot (Pa):");
  if (ImGui::InputDouble("##dP_hot", &hex.dP_hot.value)) {
    // Value updated
  }
  ImGui::SameLine();
  ImGui::Checkbox("Fixed##dP_hotFixed", &hex.dP_hot.fixed);

  ImGui::Text("Pressure Drop Cold (Pa):");
  if (ImGui::InputDouble("##dP_cold", &hex.dP_cold.value)) {
    // Value updated
  }
  ImGui::SameLine();
  ImGui::Checkbox("Fixed##dP_coldFixed", &hex.dP_cold.fixed);

  ImGui::Text("Heat Duty (W):");
  if (ImGui::InputDouble("##Q", &hex.Q.value)) {
    // Value updated
  }
  ImGui::SameLine();
  ImGui::Checkbox("Fixed##QFixed", &hex.Q.fixed);

  ImGui::PopItemWidth();

  ImGui::Spacing();

  // Connections with dropdowns
  ImGui::Text("Connections:");
  
  // Build stream list
  std::vector<StreamItem> stream_list = GetStreamList();
  
  // Hot side
  ImGui::Text("Hot Side:");
  ImGui::Text("  Inlet:");
  ImGui::SameLine(100);
  StreamCombo("##HXHotInlet", hex.in_hot, stream_list);
  
  ImGui::Text("  Outlet:");
  ImGui::SameLine(100);
  StreamCombo("##HXHotOutlet", hex.out_hot, stream_list);
  
  ImGui::Spacing();
  
  // Cold side
  ImGui::Text("Cold Side:");
  ImGui::Text("  Inlet:");
  ImGui::SameLine(100);
  StreamCombo("##HXColdInlet", hex.in_cold, stream_list);
  
  ImGui::Text("  Outlet:");
  ImGui::SameLine(100);
  StreamCombo("##HXColdOutlet", hex.out_cold, stream_list);
}

void ShowSelectedUnitProperties() {
  if (!selected_unit.valid()) {
    ImGui::Text("No unit selected");
    ImGui::Text("Select a unit from the flowsheet list to view/edit its properties.");
    return;
  }

  // Get the selected unit based on type and index
  bool found = false;
  if (selected_unit.type == SelectionType::Valve) {
    uint32_t idx = 0;
    flowsheet.valves_.for_each_with_handle([&](px::Valve& valve, px::Handle<px::Valve> handle) {
      if (!found && idx == selected_unit.index) {
        ShowValveProperties(valve);
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        // Delete button
        if (ImGui::Button("Delete Valve", ImVec2(-1, 0))) {
          flowsheet.erase(handle);
          selected_unit.clear();
          found = true;
          return;
        }
        found = true;
      }
      idx++;
    });
  } else if (selected_unit.type == SelectionType::Mixer) {
    uint32_t idx = 0;
    flowsheet.mixers_.for_each_with_handle([&](px::Mixer& mixer, px::Handle<px::Mixer> handle) {
      if (!found && idx == selected_unit.index) {
        ShowMixerProperties(mixer);
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        // Delete button
        if (ImGui::Button("Delete Mixer", ImVec2(-1, 0))) {
          flowsheet.erase(handle);
          selected_unit.clear();
          found = true;
          return;
        }
        found = true;
      }
      idx++;
    });
  } else if (selected_unit.type == SelectionType::Splitter) {
    uint32_t idx = 0;
    flowsheet.splitters_.for_each_with_handle([&](px::Splitter& splitter, px::Handle<px::Splitter> handle) {
      if (!found && idx == selected_unit.index) {
        ShowSplitterProperties(splitter);
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        // Delete button
        if (ImGui::Button("Delete Splitter", ImVec2(-1, 0))) {
          flowsheet.erase(handle);
          selected_unit.clear();
          found = true;
          return;
        }
        found = true;
      }
      idx++;
    });
  } else if (selected_unit.type == SelectionType::Stream) {
    uint32_t idx = 0;
    flowsheet.streams_.for_each_with_handle([&](px::Stream& stream, px::Handle<px::Stream> handle) {
      if (!found && idx == selected_unit.index) {
        ShowStreamProperties(stream);
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        // Delete button
        if (ImGui::Button("Delete Stream", ImVec2(-1, 0))) {
          flowsheet.erase(handle);
          selected_unit.clear();
          found = true;
          return;
        }
        found = true;
      }
      idx++;
    });
  } else if (selected_unit.type == SelectionType::SimpleHeatExchanger) {
    uint32_t idx = 0;
    flowsheet.simple_heat_exchangers_.for_each_with_handle([&](px::SimpleHeatExchanger& hex, px::Handle<px::SimpleHeatExchanger> handle) {
      if (!found && idx == selected_unit.index) {
        ShowSimpleHeatExchangerProperties(hex);
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        // Delete button
        if (ImGui::Button("Delete SimpleHeatExchanger", ImVec2(-1, 0))) {
          flowsheet.erase(handle);
          selected_unit.clear();
          found = true;
          return;
        }
        found = true;
      }
      idx++;
    });
  } else if (selected_unit.type == SelectionType::HeatExchanger) {
    uint32_t idx = 0;
    flowsheet.heat_exchangers_.for_each_with_handle([&](px::HeatExchanger& hex, px::Handle<px::HeatExchanger> handle) {
      if (!found && idx == selected_unit.index) {
        ShowHeatExchangerProperties(hex);
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        // Delete button
        if (ImGui::Button("Delete HeatExchanger", ImVec2(-1, 0))) {
          flowsheet.erase(handle);
          selected_unit.clear();
          found = true;
          return;
        }
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

