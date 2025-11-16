#include "gui_properties.h"
#include "gui_common.h"
#include "gui_log.h"
#include "gui_util.h"
#include "units.h"
#include <imgui.h>
#include <cstring>
#include <cstdio>
#include <cmath>
#include <string>

void ShowValveProperties(px::Valve& valve) {
  ImGui::Text("Valve Properties");
  ImGui::Separator();

  ShowTextInput(valve.name, "Name");

  ImGui::Spacing();

  // Cv editing - Flow coefficient (mol/(s·Pa))
  static QuantityEditState cv_state;
  ShowDoubleInput(valve.Cv, cv_state, "Flow Coefficient (Cv)", MakeUnitSet(kMolarPerPressureUnits));

  ImGui::Spacing();

  // Connections
  ImGui::Text("Connections:");
  std::vector<StreamItem> stream_list = GetStreamList();
  
  StreamCombo("Inlet:", valve.in, stream_list);
  
  StreamCombo("Outlet:", valve.out, stream_list);
}

void ShowMixerProperties(px::Mixer& mixer) {
  ImGui::Text("Mixer Properties");
  ImGui::Separator();

  ShowTextInput(mixer.name, "Name");

  ImGui::Spacing();

  // Connections
  ImGui::Text("Connections:");
  std::vector<StreamItem> stream_list = GetStreamList();
  
  if (ImGui::Button("Add Inlet")) {
    mixer.in.push_back(px::Handle<px::Stream>{});
  }
  
  // Inlets - list with dropdowns
  for (size_t i = 0; i < mixer.in.size(); ++i) {
    ImGui::PushID(static_cast<int>(i));
    std::string label = "Inlet " + std::to_string(i) + ":";
    bool should_break = StreamComboWithDelete(label.c_str(), mixer.in[i], stream_list, [&mixer, i]() {
      mixer.in.erase(mixer.in.begin() + i);
    });
    ImGui::PopID();
    if (should_break) break; // Exit loop if item was deleted or connection changed
  }
  
  ImGui::Spacing();
  
  StreamCombo("Outlet:", mixer.out, stream_list);
}

void ShowSplitterProperties(px::Splitter& splitter) {
  ImGui::Text("Splitter Properties");
  ImGui::Separator();

  // Name editing
  ShowTextInput(splitter.name, "Name");

  ImGui::Spacing();

  // Connections
  ImGui::Text("Connections:");
  std::vector<StreamItem> stream_list = GetStreamList();
  
  // Inlet dropdown
  StreamCombo("Inlet:", splitter.in, stream_list);
  
  ImGui::Spacing();
  
  if (ImGui::Button("Add Outlet")) {
    splitter.out.push_back(px::Handle<px::Stream>{});
  }
  
  // Outlets - list with dropdowns
  ImGui::Text("Outlets:");
  for (size_t i = 0; i < splitter.out.size(); ++i) {
    ImGui::PushID(static_cast<int>(i));
    std::string label = "Outlet " + std::to_string(i) + ":";
    px::Handle<px::Stream>& outlet_handle = splitter.out[i];
    bool should_break = StreamComboWithDelete(label.c_str(), outlet_handle, stream_list, [&splitter, i]() {
      splitter.out.erase(splitter.out.begin() + i);
    });
    ImGui::PopID();
    if (should_break) break; // Exit loop if item was deleted or connection changed
  }
  
}

void ShowStreamProperties(px::Stream& stream) {
  ImGui::Text("Stream Properties");
  ImGui::Separator();

  ShowTextInput(stream.name, "Name");

  ImGui::Spacing();

  // Properties editing
  ImGui::Text("Properties:");
  
  static QuantityEditState molar_flow_state;
  static QuantityEditState pressure_state;
  static QuantityEditState temperature_state;
  static QuantityEditState enthalpy_state;
  
  ShowDoubleInput(stream.molar_flow, molar_flow_state, "Molar Flowrate", MakeUnitSet(kMolarFlowUnits));
  ShowDoubleInput(stream.pressure, pressure_state, "Pressure", MakeUnitSet(kPressureUnits));
  ShowDoubleInput(stream.temperature, temperature_state, "Temperature", MakeUnitSet(kTemperatureUnits));
  ShowDoubleInput(stream.molar_enthalpy, enthalpy_state, "Molar Enthalpy", MakeUnitSet(kSpecificMolarEnergyUnits));

  ImGui::Spacing();
  ImGui::Separator();
  ImGui::Spacing();
  
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
      
      std::string pkg_name = flowsheet.fluids.GetPackageName(pkg_id);
      package_labels.push_back(pkg_name);
      
      // Check if this is the current package
      if (stream.fluid_package_id == pkg_id) {
        current_selection = static_cast<int>(i + 1); // +1 because index 0 is "(None)"
      }
    }
    
    // Create C-style string array for combo box
    std::vector<const char*> combo_items;
    combo_items.reserve(package_labels.size());
    for (const auto& label : package_labels) {
      combo_items.push_back(label.c_str());
    }
    
    // Use ShowLabeledComboBox for fluid package selection
    ShowLabeledComboBox<int>("Fluid Package", current_selection, combo_items.data(), 
                             static_cast<int>(combo_items.size()), [&](int sel) {
      if (sel == 0) {
        // Selected "(None)" - set to 0
        stream.fluid_package_id = 0;
        stream.mole_fractions.clear();
      } else {
        // Selected a package
        size_t selected_pkg_id = package_ids[sel - 1];
        stream.fluid_package_id = selected_pkg_id;
        // Initialize composition variables based on fluid package components
        auto components = flowsheet.fluids.GetComponents(selected_pkg_id);
        stream.initialize_composition(components.size(), components);
      }
    });
    
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
          static QuantityEditState mole_frac_state;
          double sum = 0.0;
          for (size_t i = 0; i < components.size() && i < stream.mole_fractions.size(); ++i) {
            ShowDoubleInput(stream.mole_fractions[i], mole_frac_state, components[i].c_str(), MakeUnitSet(kMoleFractionUnits));
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
  ShowTextInput(hex.name, "Name");

  ImGui::Spacing();

  // Parameters editing
  ImGui::Text("Parameters:");
  
  static QuantityEditState dP_state;
  static QuantityEditState Q_state;
  
  ShowDoubleInput(hex.dP, dP_state, "Pressure Drop", MakeUnitSet(kPressureUnits));
  ShowDoubleInput(hex.Q, Q_state, "Heat Duty", MakeUnitSet(kPowerUnits));

  ImGui::Spacing();

  // Connections with dropdowns
  ImGui::Text("Connections:");
  
  // Build stream list
  std::vector<StreamItem> stream_list = GetStreamList();
  
  // Inlet dropdown
  StreamCombo("Inlet:", hex.in, stream_list);
  
  // Outlet dropdown
  StreamCombo("Outlet:", hex.out, stream_list);
}

void ShowHeatExchangerProperties(px::HeatExchanger& hex) {
  ImGui::Text("HeatExchanger Properties");
  ImGui::Separator();

  // Name editing
  ShowTextInput(hex.name, "Name");

  ImGui::Spacing();

  // Parameters editing
  ImGui::Text("Parameters:");
  
  static QuantityEditState dP_hot_state;
  static QuantityEditState dP_cold_state;
  static QuantityEditState Q_state;
  
  ShowDoubleInput(hex.dP_hot, dP_hot_state, "Pressure Drop Hot", MakeUnitSet(kPressureUnits));
  ShowDoubleInput(hex.dP_cold, dP_cold_state, "Pressure Drop Cold", MakeUnitSet(kPressureUnits));
  ShowDoubleInput(hex.Q, Q_state, "Heat Duty", MakeUnitSet(kPowerUnits));

  ImGui::Spacing();

  // Connections with dropdowns
  ImGui::Text("Connections:");
  
  // Build stream list
  std::vector<StreamItem> stream_list = GetStreamList();
  
  // Hot side
  ImGui::Text("Hot Side:");
  StreamCombo("  Inlet:", hex.in_hot, stream_list);
  StreamCombo("  Outlet:", hex.out_hot, stream_list);
  
  ImGui::Spacing();
  
  // Cold side
  ImGui::Text("Cold Side:");
  StreamCombo("  Inlet:", hex.in_cold, stream_list);
  StreamCombo("  Outlet:", hex.out_cold, stream_list);
}

void ShowPumpProperties(px::Pump& pump) {
  ImGui::Text("Pump Properties");
  ImGui::Separator();

  ShowTextInput(pump.name, "Name");

  ImGui::Spacing();

  // Parameters editing
  ImGui::Text("Parameters:");
  
  static QuantityEditState dP_state;
  static QuantityEditState W_state;
  static QuantityEditState eta_state;
  
  ShowDoubleInput(pump.dP, dP_state, "Pressure Rise", MakeUnitSet(kPressureUnits));
  ShowDoubleInput(pump.W, W_state, "Work", MakeUnitSet(kPowerUnits));
  ShowDoubleInput(pump.eta, eta_state, "Efficiency", MakeUnitSet(kDimensionlessUnits));

  ImGui::Spacing();

  // Connections with dropdowns
  ImGui::Text("Connections:");
  
  // Build stream list
  std::vector<StreamItem> stream_list = GetStreamList();
  
  // Inlet dropdown
  StreamCombo("Inlet:", pump.in, stream_list);
  
  // Outlet dropdown
  StreamCombo("Outlet:", pump.out, stream_list);
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
  } else if (selected_unit.type == SelectionType::Pump) {
    uint32_t idx = 0;
    flowsheet.pumps_.for_each_with_handle([&](px::Pump& pump, px::Handle<px::Pump> handle) {
      if (!found && idx == selected_unit.index) {
        ShowPumpProperties(pump);
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        // Delete button
        if (ImGui::Button("Delete Pump", ImVec2(-1, 0))) {
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

