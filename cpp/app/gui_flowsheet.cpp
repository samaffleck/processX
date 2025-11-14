#include "gui_flowsheet.h"
#include "gui_common.h"
#include <imgui.h>

void ShowUnitOperations() {
  ImGui::Text("Flowsheet Units");
  ImGui::Separator();

  flowsheet.build_unit_list();

  // Group by type using collapsible headers
  // Valves
  if (flowsheet.valves_.size_alive() > 0) {
    if (ImGui::CollapsingHeader(("Valves (" + std::to_string(flowsheet.valves_.size_alive()) + ")").c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
      uint32_t idx = 0;
      flowsheet.valves_.for_each_with_handle([&](px::Valve& valve, px::Handle<px::Valve> handle) {
        bool is_selected = selected_unit.type == SelectionType::Valve && selected_unit.index == idx;
        std::string name = valve.get_name().empty() ? "(Unnamed Valve)" : valve.get_name();
        
        // Calculate available width
        float available_width = ImGui::GetContentRegionAvail().x;
        float button_width = 20.0f;
        float selectable_width = available_width - button_width - ImGui::GetStyle().ItemSpacing.x;
        
        // Selectable takes most of the width
        ImGui::PushItemWidth(selectable_width);
        if (ImGui::Selectable(name.c_str(), is_selected)) {
          selected_unit.type = SelectionType::Valve;
          selected_unit.index = idx;
        }
        ImGui::PopItemWidth();
        idx++;
      });
    }
  }

  // Mixers
  if (flowsheet.mixers_.size_alive() > 0) {
    if (ImGui::CollapsingHeader(("Mixers (" + std::to_string(flowsheet.mixers_.size_alive()) + ")").c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
      uint32_t idx = 0;
      flowsheet.mixers_.for_each_with_handle([&](px::Mixer& mixer, px::Handle<px::Mixer> handle) {
        bool is_selected = selected_unit.type == SelectionType::Mixer && selected_unit.index == idx;
        std::string name = mixer.get_name().empty() ? "(Unnamed Mixer)" : mixer.get_name();
        
        // Calculate available width
        float available_width = ImGui::GetContentRegionAvail().x;
        float button_width = 60.0f;
        float selectable_width = available_width - button_width - ImGui::GetStyle().ItemSpacing.x;
        
        // Selectable takes most of the width
        ImGui::PushItemWidth(selectable_width);
        if (ImGui::Selectable(name.c_str(), is_selected)) {
          selected_unit.type = SelectionType::Mixer;
          selected_unit.index = idx;
        }
        ImGui::PopItemWidth();
        idx++;
      });
    }
  }

  // Splitters
  if (flowsheet.splitters_.size_alive() > 0) {
    if (ImGui::CollapsingHeader(("Splitters (" + std::to_string(flowsheet.splitters_.size_alive()) + ")").c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
      uint32_t idx = 0;
      flowsheet.splitters_.for_each_with_handle([&](px::Splitter& splitter, px::Handle<px::Splitter> handle) {
        bool is_selected = selected_unit.type == SelectionType::Splitter && selected_unit.index == idx;
        std::string name = splitter.get_name().empty() ? "(Unnamed Splitter)" : splitter.get_name();
        
        // Calculate available width
        float available_width = ImGui::GetContentRegionAvail().x;
        float button_width = 60.0f;
        float selectable_width = available_width - button_width - ImGui::GetStyle().ItemSpacing.x;
        
        // Selectable takes most of the width
        ImGui::PushItemWidth(selectable_width);
        if (ImGui::Selectable(name.c_str(), is_selected)) {
          selected_unit.type = SelectionType::Splitter;
          selected_unit.index = idx;
        }
        ImGui::PopItemWidth();
        idx++;
      });
    }
  }

  // Streams
  if (flowsheet.streams_.size_alive() > 0) {
    if (ImGui::CollapsingHeader(("Streams (" + std::to_string(flowsheet.streams_.size_alive()) + ")").c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
      uint32_t idx = 0;
      flowsheet.streams_.for_each_with_handle([&](px::Stream& stream, px::Handle<px::Stream> handle) {
        bool is_selected = selected_unit.type == SelectionType::Stream && selected_unit.index == idx;
        std::string name = stream.name.empty() ? "(Unnamed Stream)" : stream.name;
        
        // Calculate available width
        float available_width = ImGui::GetContentRegionAvail().x;
        float button_width = 60.0f;
        float selectable_width = available_width - button_width - ImGui::GetStyle().ItemSpacing.x;
        
        // Selectable takes most of the width
        ImGui::PushItemWidth(selectable_width);
        if (ImGui::Selectable(name.c_str(), is_selected)) {
          selected_unit.type = SelectionType::Stream;
          selected_unit.index = idx;
        }
        ImGui::PopItemWidth();
        idx++;
      });
    }
  }

  // SimpleHeatExchangers
  if (flowsheet.simple_heat_exchangers_.size_alive() > 0) {
    if (ImGui::CollapsingHeader(("SimpleHeatExchangers (" + std::to_string(flowsheet.simple_heat_exchangers_.size_alive()) + ")").c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
      uint32_t idx = 0;
      flowsheet.simple_heat_exchangers_.for_each_with_handle([&](px::SimpleHeatExchanger& hex, px::Handle<px::SimpleHeatExchanger> handle) {
        bool is_selected = selected_unit.type == SelectionType::SimpleHeatExchanger && selected_unit.index == idx;
        std::string name = hex.get_name().empty() ? "(Unnamed SimpleHeatExchanger)" : hex.get_name();
        
        // Calculate available width
        float available_width = ImGui::GetContentRegionAvail().x;
        float button_width = 60.0f;
        float selectable_width = available_width - button_width - ImGui::GetStyle().ItemSpacing.x;
        
        // Selectable takes most of the width
        ImGui::PushItemWidth(selectable_width);
        if (ImGui::Selectable(name.c_str(), is_selected)) {
          selected_unit.type = SelectionType::SimpleHeatExchanger;
          selected_unit.index = idx;
        }
        ImGui::PopItemWidth();
        idx++;
      });
    }
  }

  // HeatExchangers
  if (flowsheet.heat_exchangers_.size_alive() > 0) {
    if (ImGui::CollapsingHeader(("HeatExchangers (" + std::to_string(flowsheet.heat_exchangers_.size_alive()) + ")").c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
      uint32_t idx = 0;
      flowsheet.heat_exchangers_.for_each_with_handle([&](px::HeatExchanger& hex, px::Handle<px::HeatExchanger> handle) {
        bool is_selected = selected_unit.type == SelectionType::HeatExchanger && selected_unit.index == idx;
        std::string name = hex.get_name().empty() ? "(Unnamed HeatExchanger)" : hex.get_name();
        
        // Calculate available width
        float available_width = ImGui::GetContentRegionAvail().x;
        float button_width = 60.0f;
        float selectable_width = available_width - button_width - ImGui::GetStyle().ItemSpacing.x;
        
        // Selectable takes most of the width
        ImGui::PushItemWidth(selectable_width);
        if (ImGui::Selectable(name.c_str(), is_selected)) {
          selected_unit.type = SelectionType::HeatExchanger;
          selected_unit.index = idx;
        }
        ImGui::PopItemWidth();
        idx++;
      });
    }
  }

  // Pumps
  if (flowsheet.pumps_.size_alive() > 0) {
    if (ImGui::CollapsingHeader(("Pumps (" + std::to_string(flowsheet.pumps_.size_alive()) + ")").c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
      uint32_t idx = 0;
      flowsheet.pumps_.for_each_with_handle([&](px::Pump& pump, px::Handle<px::Pump> handle) {
        bool is_selected = selected_unit.type == SelectionType::Pump && selected_unit.index == idx;
        std::string name = pump.get_name().empty() ? "(Unnamed Pump)" : pump.get_name();
        
        // Calculate available width
        float available_width = ImGui::GetContentRegionAvail().x;
        float button_width = 60.0f;
        float selectable_width = available_width - button_width - ImGui::GetStyle().ItemSpacing.x;
        
        // Selectable takes most of the width
        ImGui::PushItemWidth(selectable_width);
        if (ImGui::Selectable(name.c_str(), is_selected)) {
          selected_unit.type = SelectionType::Pump;
          selected_unit.index = idx;
        }
        ImGui::PopItemWidth();
        idx++;
      });
    }
  }

  if (flowsheet.valves_.size_alive() == 0 && 
      flowsheet.mixers_.size_alive() == 0 && 
      flowsheet.splitters_.size_alive() == 0 &&
      flowsheet.streams_.size_alive() == 0 &&
      flowsheet.simple_heat_exchangers_.size_alive() == 0 &&
      flowsheet.heat_exchangers_.size_alive() == 0 &&
      flowsheet.pumps_.size_alive() == 0) {
    ImGui::Text("No units in flowsheet");
    ImGui::Text("Add units using the Unit Operations palette.");
  }
}
