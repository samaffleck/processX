#include "gui_properties.h"
#include "gui_common.h"
#include "gui_log.h"
#include <imgui.h>
#include <cstring>
#include <cstdio>
#include <cmath>

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

  ImGui::Spacing();
  ImGui::Separator();
  ImGui::Spacing();

  // Fluid Package selection
  ImGui::Text("Fluid Package:");
  
  // Get all available fluid package IDs
  std::vector<size_t> package_ids = flowsheet.fluids.GetAllPackageIds();
  
  if (package_ids.empty()) {
    ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "No fluid packages available");
    ImGui::Text("Create a fluid package in the 'Fluid Packages' window first.");
  } else {
    // Build display strings for combo box
    std::vector<std::string> package_labels;
    package_labels.reserve(package_ids.size() + 1);
    package_labels.push_back("(None)");
    
    int current_selection = 0; // Default to "(None)"
    
    for (size_t i = 0; i < package_ids.size(); ++i) {
      size_t pkg_id = package_ids[i];
      auto components = flowsheet.fluids.GetComponents(pkg_id);
      std::string backend = flowsheet.fluids.GetThermoPackage(pkg_id);
      
      // Build label: "PackageName: Component1, Component2, ... (Backend)"
      std::string pkg_name = flowsheet.fluids.GetPackageName(pkg_id);
      std::string label = pkg_name + ": ";
      if (components.empty()) {
        label += "(empty)";
      } else {
        for (size_t j = 0; j < components.size(); ++j) {
          if (j > 0) label += ", ";
          label += components[j];
        }
      }
      label += " (" + backend + ")";
      package_labels.push_back(label);
      
      // Check if this is the current package
      if (stream.fluid_package_id == pkg_id) {
        current_selection = static_cast<int>(i + 1); // +1 because index 0 is "(None)"
      }
    }
    
    // Create C-style string array for ImGui::Combo
    std::vector<const char*> combo_items;
    combo_items.reserve(package_labels.size());
    for (const auto& label : package_labels) {
      combo_items.push_back(label.c_str());
    }
    
    // Combo box
    ImGui::PushItemWidth(400);
    if (ImGui::Combo("##FluidPackage", &current_selection, combo_items.data(), 
                     static_cast<int>(combo_items.size()))) {
      // Selection changed
      if (current_selection == 0) {
        // Selected "(None)" - set to 0
        stream.fluid_package_id = 0;
        stream.mole_fractions.clear();
        // stream.mole_fractions_liquid.clear();
        // stream.mole_fractions_vapor.clear();
      } else {
        // Selected a package
        size_t selected_pkg_id = package_ids[current_selection - 1];
        stream.fluid_package_id = selected_pkg_id;
        // Initialize composition variables based on fluid package components
        auto components = flowsheet.fluids.GetComponents(selected_pkg_id);
        stream.initialize_composition(components.size(), components);
      }
    }
    ImGui::PopItemWidth();
    
    // Show current package info if selected
    if (stream.fluid_package_id != 0) {
      auto components = flowsheet.fluids.GetComponents(stream.fluid_package_id);
      std::string backend = flowsheet.fluids.GetThermoPackage(stream.fluid_package_id);
      
      ImGui::Text("Components: ");
      ImGui::SameLine();
      if (components.empty()) {
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "(empty - add components in Fluid Packages window)");
      } else {
        std::string comp_list;
        for (size_t i = 0; i < components.size(); ++i) {
          if (i > 0) comp_list += ", ";
          comp_list += components[i];
        }
        ImGui::Text("%s", comp_list.c_str());
      }
      
      ImGui::Text("Backend: %s", backend.c_str());
      
      ImGui::Spacing();
      ImGui::Separator();
      ImGui::Spacing();
      
      // Composition editing
      if (!components.empty()) {
        ImGui::Text("Composition:");
        ImGui::Spacing();
        
        // Ensure composition variables are initialized
        if (stream.mole_fractions.size() != components.size()) {
          stream.initialize_composition(components.size(), components);
        }
        
        ImGui::PushItemWidth(200);
        
        // Overall mole fractions
        if (ImGui::CollapsingHeader("Overall Mole Fractions", ImGuiTreeNodeFlags_DefaultOpen)) {
          double sum = 0.0;
          for (size_t i = 0; i < components.size() && i < stream.mole_fractions.size(); ++i) {
            ImGui::Text("%s:", components[i].c_str());
            ImGui::SameLine(150);
            char label[64];
            snprintf(label, sizeof(label), "##MoleFrac_%zu", i);
            if (ImGui::InputDouble(label, &stream.mole_fractions[i].value)) {
              // Value updated
            }
            ImGui::SameLine();
            snprintf(label, sizeof(label), "Fixed##MoleFracFixed_%zu", i);
            ImGui::Checkbox(label, &stream.mole_fractions[i].fixed);
            sum += stream.mole_fractions[i].value;
          }
          ImGui::Text("Sum: %.6f", sum);
          if (std::abs(sum - 1.0) > 1e-6 && sum > 0.0) {
            ImGui::SameLine();
            if (ImGui::Button("Normalize")) {
              for (size_t i = 0; i < stream.mole_fractions.size(); ++i) {
                stream.mole_fractions[i].value /= sum;
              }
            }
          }
        }
        
        ImGui::Spacing();
        
        // // Liquid phase mole fractions
        // if (ImGui::CollapsingHeader("Liquid Phase Mole Fractions")) {
        //   double sum_liq = 0.0;
        //   for (size_t i = 0; i < components.size() && i < stream.mole_fractions_liquid.size(); ++i) {
        //     ImGui::Text("%s:", components[i].c_str());
        //     ImGui::SameLine(150);
        //     char label[64];
        //     snprintf(label, sizeof(label), "##MoleFracLiq_%zu", i);
        //     if (ImGui::InputDouble(label, &stream.mole_fractions_liquid[i].value)) {
        //       // Value updated
        //     }
        //     ImGui::SameLine();
        //     snprintf(label, sizeof(label), "Fixed##MoleFracLiqFixed_%zu", i);
        //     ImGui::Checkbox(label, &stream.mole_fractions_liquid[i].fixed);
        //     sum_liq += stream.mole_fractions_liquid[i].value;
        //   }
        //   ImGui::Text("Sum: %.6f", sum_liq);
        //   if (std::abs(sum_liq - 1.0) > 1e-6 && sum_liq > 0.0) {
        //     ImGui::SameLine();
        //     if (ImGui::Button("Normalize##Liq")) {
        //       for (size_t i = 0; i < stream.mole_fractions_liquid.size(); ++i) {
        //         stream.mole_fractions_liquid[i].value /= sum_liq;
        //       }
        //     }
        //   }
        // }
        
        // ImGui::Spacing();
        
        // // Vapor phase mole fractions
        // if (ImGui::CollapsingHeader("Vapor Phase Mole Fractions")) {
        //   double sum_vap = 0.0;
        //   for (size_t i = 0; i < components.size() && i < stream.mole_fractions_vapor.size(); ++i) {
        //     ImGui::Text("%s:", components[i].c_str());
        //     ImGui::SameLine(150);
        //     char label[64];
        //     snprintf(label, sizeof(label), "##MoleFracVap_%zu", i);
        //     if (ImGui::InputDouble(label, &stream.mole_fractions_vapor[i].value)) {
        //       // Value updated
        //     }
        //     ImGui::SameLine();
        //     snprintf(label, sizeof(label), "Fixed##MoleFracVapFixed_%zu", i);
        //     ImGui::Checkbox(label, &stream.mole_fractions_vapor[i].fixed);
        //     sum_vap += stream.mole_fractions_vapor[i].value;
        //   }
        //   ImGui::Text("Sum: %.6f", sum_vap);
        //   if (std::abs(sum_vap - 1.0) > 1e-6 && sum_vap > 0.0) {
        //     ImGui::SameLine();
        //     if (ImGui::Button("Normalize##Vap")) {
        //       for (size_t i = 0; i < stream.mole_fractions_vapor.size(); ++i) {
        //         stream.mole_fractions_vapor[i].value /= sum_vap;
        //       }
        //     }
        //   }
        // }
        
        // ImGui::Spacing();
        
        // // Quality (vapor fraction)
        // ImGui::Text("Quality (Vapor Fraction):");
        // ImGui::SameLine(200);
        // if (ImGui::InputDouble("##Quality", &stream.quality.value)) {
        //   // Clamp to [0, 1]
        //   if (stream.quality.value < 0.0) stream.quality.value = 0.0;
        //   if (stream.quality.value > 1.0) stream.quality.value = 1.0;
        // }
        // ImGui::SameLine();
        // ImGui::Checkbox("Fixed##QualityFixed", &stream.quality.fixed);
        
        ImGui::PopItemWidth();
      } else {
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "No components in fluid package.");
        ImGui::Text("Add components in the 'Fluid Packages' window.");
      }
    }
  }
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

