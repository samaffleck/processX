#include "gui_palette.h"
#include "gui_common.h"
#include "gui_log.h"
#include "hello_imgui/icons_font_awesome_6.h"
#include <imgui.h>

void ShowUnitOperationsPalette() {
  // Fixed square button size
  ImVec2 button_dim(60.0f, 60.0f);

  if (ImGui::Button((std::string(ICON_FA_CUBES) + "\nMixer").c_str(), button_dim)) {
    flowsheet.add<px::Mixer>();
    selected_unit.clear(); // Clear selection when adding new unit
    AddLogEntry(LogEntry::Info, "Added new Mixer");
  }
  ImGui::SameLine();
  if (ImGui::Button((std::string(ICON_FA_PLUG) + "\nValve").c_str(), button_dim)) {
    flowsheet.add<px::Valve>();
    selected_unit.clear();
    AddLogEntry(LogEntry::Info, "Added new Valve");
  }
  ImGui::SameLine();
  if (ImGui::Button((std::string(ICON_FA_ARROWS_UP_DOWN_LEFT_RIGHT) + "\nSplitter").c_str(), button_dim)) {
    flowsheet.add<px::Splitter>();
    selected_unit.clear();
    AddLogEntry(LogEntry::Info, "Added new Splitter");
  }
  ImGui::SameLine();
  if (ImGui::Button((std::string(ICON_FA_WAVE_SQUARE) + "\nStream").c_str(), button_dim)) {
    flowsheet.add<px::Stream>();
    selected_unit.clear();
    AddLogEntry(LogEntry::Info, "Added new Stream");
  }
  ImGui::SameLine();
  if (ImGui::Button((std::string(ICON_FA_FIRE) + "\nHX(1)").c_str(), button_dim)) {
    flowsheet.add<px::SimpleHeatExchanger>();
    selected_unit.clear();
    AddLogEntry(LogEntry::Info, "Added new SimpleHeatExchanger");
  }
  ImGui::SameLine();
  if (ImGui::Button((std::string(ICON_FA_FIRE) + "\nHX(2)").c_str(), button_dim)) {
    flowsheet.add<px::HeatExchanger>();
    selected_unit.clear();
    AddLogEntry(LogEntry::Info, "Added new HeatExchanger");
  }
  ImGui::SameLine();
  if (ImGui::Button((std::string(ICON_FA_BOLT) + "\nPump").c_str(), button_dim)) {
    flowsheet.add<px::Pump>();
    selected_unit.clear();
    AddLogEntry(LogEntry::Info, "Added new Pump");
  }
  ImGui::SameLine();
  if (ImGui::Button((std::string(ICON_FA_WRENCH) + "\nComponentSplitter").c_str(), button_dim)) {
    flowsheet.add<px::ComponentSplitter>();
    selected_unit.clear();
    AddLogEntry(LogEntry::Info, "Added new ComponentSplitter");
  }
}

