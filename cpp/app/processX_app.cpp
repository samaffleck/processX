// apps/processx_app.cpp

#include <hello_imgui/hello_imgui.h>
#include <imgui.h>
#include <string>
#include <filesystem>

#include "processX/plant.h"


// DemoPanelPlant.hpp
#pragma once
#include <imgui.h>
#include <string>
#include <vector>
#include <algorithm>

// your headers:
#include "processX/unitop.h"
#include "processX/stream.h"

// If Plant is in a cpp, expose it via a header, or paste the class here.
#include "processX/plant.h" // assumed

namespace demo {

static std::string EndpointLabel(const px::Plant& P, const px::Endpoint& e) {
  if (e.unit == px::kInvalid) return "(unconnected)";
  const auto& u = P.units.at(P.unit_idx.at(e.unit));
  const bool isIn = e.kind == px::PortKind::Inlet;
  const auto& names = isIn ? u.inlet_names : u.outlet_names;
  std::string slot = (e.slot < names.size()) ? names[e.slot] : ("#" + std::to_string(e.slot));
  return u.name + (isIn ? " :: " : " -> ") + slot;
}

static bool ComboUnits(const char* label, px::Plant& P, px::Handle& sel, bool outletsOnly, int* outMaxSlots = nullptr) {
  int current = -1;
  std::vector<px::Handle> handles;
  handles.reserve(P.units.size());
  for (auto& u : P.units) handles.push_back(u.id);

  if (handles.empty()) {
    ImGui::TextDisabled("%s (none)", label);
    sel = px::kInvalid;
    if (outMaxSlots) *outMaxSlots = 0;
    return false;
  }

  // find current
  if (sel != px::kInvalid) {
    auto it = std::find(handles.begin(), handles.end(), sel);
    if (it != handles.end()) current = int(it - handles.begin());
  }

  bool changed = false;
  if (ImGui::BeginCombo(label, (sel == px::kInvalid) ? "(choose unit)" : P.GetUnit(sel).name.c_str())) {
    for (int i = 0; i < (int)handles.size(); ++i) {
      const bool isSelected = (i == current);
      const auto& u = P.GetUnit(handles[i]);
      if (ImGui::Selectable(u.name.c_str(), isSelected)) {
        sel = u.id;
        current = i;
        changed = true;
      }
      if (isSelected) ImGui::SetItemDefaultFocus();
    }
    ImGui::EndCombo();
  }

  if (outMaxSlots) {
    if (sel != px::kInvalid) {
      const auto& u = P.GetUnit(sel);
      *outMaxSlots = outletsOnly ? (int)u.outlet_names.size() : (int)u.inlet_names.size();
    } else {
      *outMaxSlots = 0;
    }
  }
  return changed;
}

static bool ComboSlot(const char* label, const std::vector<std::string>& names, int& slot) {
  const char* preview = "(slot)";
  std::string buf;
  if (slot >= 0 && slot < (int)names.size()) preview = names[slot].c_str();

  bool changed = false;
  if (ImGui::BeginCombo(label, preview)) {
    for (int i = 0; i < (int)names.size(); ++i) {
      bool sel = (i == slot);
      if (ImGui::Selectable(names[i].c_str(), sel)) { slot = i; changed = true; }
      if (sel) ImGui::SetItemDefaultFocus();
    }
    ImGui::EndCombo();
  }
  return changed;
}

} // namespace demo

// ========================= Panel =========================

static void ShowPlantDock() {
  static px::Plant plant;

  // state for connection panel
  static px::Handle fromUnit = px::kInvalid;
  static int fromSlot = -1;
  static px::Handle toUnit = px::kInvalid;
  static int toSlot = -1;

  ImGui::Begin("Plant");

  if (ImGui::CollapsingHeader("Add Units", ImGuiTreeNodeFlags_DefaultOpen)) {

    ImGui::PushID("add_units");
    if (ImGui::Button("Add Valve")) {
      auto u = plant.AddUnit(px::UnitType::Valve, {"In"}, {"Out"});
      plant.GetUnit(u).name = "Valve " + std::to_string(u);
    }

    if (ImGui::Button("Add Mixer")) {
      std::vector<std::string> ins{};
      auto u = plant.AddUnit(px::UnitType::Mixer, std::move(ins), {"Out"});
      plant.GetUnit(u).name = "Mixer " + std::to_string(u);
    }

    if (ImGui::Button("Add Splitter")) {
      std::vector<std::string> outs{};
      auto u = plant.AddUnit(px::UnitType::Splitter, {"In"}, std::move(outs));
      plant.GetUnit(u).name = "Splitter " + std::to_string(u);
    }
    ImGui::PopID();
  }

  ImGui::Separator();

  if (ImGui::CollapsingHeader("Add Stream", ImGuiTreeNodeFlags_DefaultOpen)) {
    if (ImGui::Button("New Material Stream")) {
      auto sid = plant.AddStream(px::StreamKind::Material, "S" + std::to_string((int)plant.streams.size()));
      (void)sid;
    }
  }

  ImGui::Separator();

  if (ImGui::CollapsingHeader("Connect Stream", ImGuiTreeNodeFlags_DefaultOpen)) {
    ImGui::TextDisabled("Pick an Outlet (from) and an Inlet (to), then choose a stream to connect.");

    int maxOutSlots = 0, maxInSlots = 0;
    demo::ComboUnits("From Unit (Outlet)", plant, fromUnit, /*outletsOnly*/true, &maxOutSlots);
    if (fromUnit != px::kInvalid) {
      auto& u = plant.GetUnit(fromUnit);
      if (fromSlot >= maxOutSlots) fromSlot = -1;
      demo::ComboSlot("Outlet Slot", u.outlet_names, fromSlot);
    }

    demo::ComboUnits("To Unit (Inlet)", plant, toUnit, /*outletsOnly*/false, &maxInSlots);
    if (toUnit != px::kInvalid) {
      auto& u = plant.GetUnit(toUnit);
      if (toSlot >= maxInSlots) toSlot = -1;
      demo::ComboSlot("Inlet Slot", u.inlet_names, toSlot);
    }

    // stream chooser
    static px::Handle selStream = px::kInvalid;
    int selIndex = -1;
    if (!plant.streams.empty()) {
      if (selStream != px::kInvalid) {
        auto it = plant.stream_idx.find(selStream);
        if (it != plant.stream_idx.end()) selIndex = (int)it->second;
      }
      const char* prev = (selIndex >= 0) ? plant.streams[selIndex].props.name.c_str() : "(choose stream)";
      if (ImGui::BeginCombo("Stream", prev)) {
        for (int i = 0; i < (int)plant.streams.size(); ++i) {
          bool isSel = (i == selIndex);
          if (ImGui::Selectable(plant.streams[i].props.name.c_str(), isSel)) {
            selIndex = i;
            selStream = plant.streams[i].id;
          }
          if (isSel) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
      }
    } else {
      ImGui::TextDisabled("No streams yet.");
    }

    bool canConnect = fromUnit != px::kInvalid && toUnit != px::kInvalid &&
                      fromSlot >= 0 && toSlot >= 0 &&
                      selStream != px::kInvalid;

    if (!canConnect) ImGui::BeginDisabled();
    if (ImGui::Button("Connect")) {
      try {
        plant.Connect(selStream, fromUnit, (uint16_t)fromSlot, toUnit, (uint16_t)toSlot);
      } catch (const std::exception& e) {
        ImGui::OpenPopup("connect_error");
      }
    }
    if (!canConnect) ImGui::EndDisabled();

    if (ImGui::BeginPopup("connect_error")) {
      ImGui::TextWrapped("Connection failed (check slot indices and directions).");
      ImGui::EndPopup();
    }
  }

  ImGui::Separator();

  if (ImGui::CollapsingHeader("Units", ImGuiTreeNodeFlags_DefaultOpen)) {
    if (ImGui::BeginTable("units_table", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
      ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, 70);
      ImGui::TableSetupColumn("Name");
      ImGui::TableSetupColumn("Inlets");
      ImGui::TableSetupColumn("Outlets");
      ImGui::TableHeadersRow();

      for (auto& u : plant.units) {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0); ImGui::Text("%u", (unsigned)u.id);
        ImGui::TableSetColumnIndex(1); ImGui::TextUnformatted(u.name.c_str());
        ImGui::TableSetColumnIndex(2);
        {
          std::string j; for (size_t i=0;i<u.inlet_names.size();++i){ if(i) j += ", "; j += u.inlet_names[i]; }
          ImGui::TextUnformatted(j.c_str());
        }
        ImGui::TableSetColumnIndex(3);
        {
          std::string j; for (size_t i=0;i<u.outlet_names.size();++i){ if(i) j += ", "; j += u.outlet_names[i]; }
          ImGui::TextUnformatted(j.c_str());
        }
      }
      ImGui::EndTable();
    }
  }

  if (ImGui::CollapsingHeader("Streams", ImGuiTreeNodeFlags_DefaultOpen)) {
    if (ImGui::BeginTable("streams_table", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
      ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, 70);
      ImGui::TableSetupColumn("Name");
      ImGui::TableSetupColumn("From");
      ImGui::TableSetupColumn("To");
      ImGui::TableHeadersRow();

      for (auto& s : plant.streams) {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0); ImGui::Text("%u", (unsigned)s.id);
        ImGui::TableSetColumnIndex(1); ImGui::TextUnformatted(s.props.name.c_str());
        ImGui::TableSetColumnIndex(2); ImGui::TextUnformatted(demo::EndpointLabel(plant, s.from).c_str());
        ImGui::TableSetColumnIndex(3); ImGui::TextUnformatted(demo::EndpointLabel(plant, s.to).c_str());
      }
      ImGui::EndTable();
    }
  }

  ImGui::End();
}


static void ShowMainDock() {
  static bool on_first_load = true;
  
  if (on_first_load) {
    on_first_load = false;
    ImGui::StyleColorsLight();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  }

  ShowPlantDock();
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
