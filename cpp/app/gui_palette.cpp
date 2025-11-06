#include "gui_palette.h"
#include "gui_common.h"
#include "gui_log.h"
#include <imgui.h>

void ShowUnitOperationsPalette() {
  ImGui::Text("Unit Operations");
  ImGui::Separator();

  // Fixed square button size
  ImVec2 button_dim(60.0f, 60.0f);

  if (ImGui::Button("Mixer", button_dim)) {
    flowsheet.add<px::Mixer>();
    selected_unit.clear(); // Clear selection when adding new unit
    AddLogEntry(LogEntry::Info, "Added new Mixer");
  }
  ImGui::SameLine();
  if (ImGui::Button("Valve", button_dim)) {
    flowsheet.add<px::Valve>();
    selected_unit.clear();
    AddLogEntry(LogEntry::Info, "Added new Valve");
  }
  ImGui::SameLine();
  if (ImGui::Button("Splitter", button_dim)) {
    flowsheet.add<px::Splitter>();
    selected_unit.clear();
    AddLogEntry(LogEntry::Info, "Added new Splitter");
  }
  ImGui::SameLine();
  if (ImGui::Button("Stream", button_dim)) {
    flowsheet.add<px::Stream>();
    selected_unit.clear();
    AddLogEntry(LogEntry::Info, "Added new Stream");
  }
}

