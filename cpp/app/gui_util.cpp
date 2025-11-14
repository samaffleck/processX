#include "gui_util.h"
#include "gui_common.h"
#include "processX/core.h"

// ImGui includes
#include "imgui_stdlib.h"
#include "hello_imgui/hello_imgui_font.h"

// Standard library
#include <string>
#include <vector>


const ImGuiInputTextFlags_ inputDoubleFlags = ImGuiInputTextFlags_::ImGuiInputTextFlags_None; //ImGuiInputTextFlags_EnterReturnsTrue
const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen;


inline const char* ChooseAutoFormat(double v_abs, QuantityEditState::Fmt& sticky, bool item_active) {
  QuantityEditState::Fmt target =
      (!std::isfinite(v_abs) || v_abs == 0.0) ? QuantityEditState::Fmt::Fixed :
      (v_abs >= 1e-3 && v_abs < 1e4)          ? QuantityEditState::Fmt::Fixed :
                                                QuantityEditState::Fmt::Sci;

  // Hysteresis: if the user is editing, freeze the current choice
  if (item_active && sticky != QuantityEditState::Fmt::Auto) {
    target = sticky;
  } else {
    sticky = target;
  }

  return (target == QuantityEditState::Fmt::Fixed) ? "%.6f" : "%.6e";
}

bool ShowDoubleInput(
  px::Var& var,
  QuantityEditState& ui_state,
  const char* label,
  UnitSet units,
  const char* /*value_format_unused*/)
{
  ImGui::PushID(&var);               // unique + stable per field

  if (!units.data || units.size == 0) {
    const double a = std::fabs(var.value);
    const char* fmt = ChooseAutoFormat(a, ui_state.sticky_fmt, ImGui::IsItemActive());
    ImGuiTableFlags flags = ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_NoBordersInBody;
    
    if (ImGui::BeginTable("##table", 2, flags, ImVec2(-FLT_MIN, 0.0f))) {
      ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthStretch, col1_width);
      ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch, 1 - col1_width);
      
      ImGui::TableNextColumn();
      ImGui::TextUnformatted(label);
      
      ImGui::TableNextColumn();
      ImGui::Checkbox("##fixed", &var.fixed);
      ImGui::SameLine();
      ImGui::SetNextItemWidth(-FLT_MIN);
      ImGui::InputDouble("##val", &var.value, 0, 0, fmt);
      
      ImGui::EndTable();
    }
    ImGui::PopID();
    return false;
  }

  if (ui_state.unit_index < 0 || static_cast<size_t>(ui_state.unit_index) >= units.size)
    ui_state.unit_index = 0;

  const UnitDef& u = units.data[ui_state.unit_index];
  double ui_value = u.xf.FromBase(var.value);
  bool changed = false;

  ImGuiTableFlags flags = ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_NoBordersInBody;

  if (ImGui::BeginTable("##table", 3, flags, ImVec2(-FLT_MIN, 0.0f))) {
    ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthStretch, col1_width);
    ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch, col2_width);
    ImGui::TableSetupColumn("Unit",  ImGuiTableColumnFlags_WidthStretch, col3_width);

    ImGui::TableNextColumn();
    ImGui::TextUnformatted(label);

    ImGui::TableNextColumn();
    ImGui::Checkbox("##fixed", &var.fixed);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-FLT_MIN);

    // Decide format for the current displayed (UI) value
    const char* fmt = ChooseAutoFormat(std::fabs(ui_value), ui_state.sticky_fmt, ImGui::IsItemActive());
    if (ImGui::InputDouble("##val", &ui_value, 0.0, 0.0, fmt)) {
      // live edit
    }
    if (ImGui::IsItemDeactivatedAfterEdit()) {
      double new_base = u.xf.ToBase(ui_value);
      if (new_base != var.value) { var.value = new_base; changed = true; }
      // allow auto to switch next frame
      ui_state.sticky_fmt = QuantityEditState::Fmt::Auto;
    }

    ImGui::TableNextColumn();
    ImGui::SetNextItemWidth(-FLT_MIN);
    if (ImGui::BeginCombo("##unit", u.symbol)) {
      for (size_t i = 0; i < units.size; ++i) {
        bool sel = (ui_state.unit_index == static_cast<int>(i));
        if (ImGui::Selectable(units.data[i].symbol, sel)) ui_state.unit_index = static_cast<int>(i);
        if (sel) ImGui::SetItemDefaultFocus();
      }
      ImGui::EndCombo();
    }
    
    ImGui::EndTable();
  }
  
  ImGui::PopID();
  return changed;
}

void ShowDoubleInput(double& val, const std::string& label, 
  const std::string& unit, const std::string& format)
{
  ImGui::PushID(&val); // unique + stable per field
  
  if (ImGui::BeginTable("##table", 3, ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_NoBordersInBody)) {
    ImGui::TableNextColumn();
    ImGui::Text("%s", label.c_str());
    ImGui::TableNextColumn();
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    ImGui::InputDouble("##input", &val, 0.0, 0.0, format.c_str());
    ImGui::TableNextColumn();
    ImGui::Text("%s", unit.c_str());

    ImGui::EndTable();
  }

  ImGui::PopID();
}

void ShowDoubleInputReadOnly(double& val, const std::string& label, 
  const std::string& unit, const std::string& format) {
  ImGui::BeginDisabled();
  ShowDoubleInput(val, label, unit, format);
  ImGui::EndDisabled();
}

void ShowIntInput(int& val, const std::string& label, const std::string& unit) {
  ImGui::PushID(&val);               // unique + stable per field

  if (ImGui::BeginTable("##table", 3, ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_NoBordersInBody)) {
    ImGui::TableNextColumn();
    ImGui::Text("%s", label.c_str());
    ImGui::TableNextColumn();
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    ImGui::InputInt("##input", &val);
    ImGui::TableNextColumn();
    ImGui::Text("%s", unit.c_str());

    ImGui::EndTable();
  }

  ImGui::PopID();
}

bool ShowIntInput(
  int& base_value,
  QuantityEditState& ui_state,
  const char* label,
  UnitSet units
) {
  ImGui::PushID(&base_value); // unique + stable per field

  if (!units.data || units.size == 0) {
    ImGui::Text("%s", label);
    ImGui::SameLine();
    ImGui::InputInt("##val", &base_value);
    ImGui::PopID();
    return false;
  }

  if (ui_state.unit_index < 0 || static_cast<size_t>(ui_state.unit_index) >= units.size)
    ui_state.unit_index = 0;

  const UnitDef& u = units.data[ui_state.unit_index];
  int ui_value = u.xf.FromBase(base_value);
  bool changed = false;

  if (ImGui::BeginTable("##table", 3, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_NoBordersInBody)) {
    ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthStretch, col1_width);
    ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch, col2_width);
    ImGui::TableSetupColumn("Unit",  ImGuiTableColumnFlags_WidthStretch, col3_width);

    ImGui::TableNextColumn();
    ImGui::TextUnformatted(label);

    ImGui::TableNextColumn();
    ImGui::SetNextItemWidth(-FLT_MIN); // Take full column width
    if (ImGui::InputInt("##val", &ui_value)) {
      // Live edit
    }
    if (ImGui::IsItemDeactivatedAfterEdit()) {
      int new_base = u.xf.ToBase(ui_value);
      if (new_base != base_value) {
        base_value = new_base;
        changed = true;
      }
    }

    ImGui::TableNextColumn();
    ImGui::SetNextItemWidth(-FLT_MIN); // Take full column width
    if (ImGui::BeginCombo("##unit", u.symbol)) {
      for (size_t i = 0; i < units.size; ++i) {
        bool is_selected = (ui_state.unit_index == static_cast<int>(i));
        if (ImGui::Selectable(units.data[i].symbol, is_selected)) {
          ui_state.unit_index = static_cast<int>(i);
        }
        if (is_selected) ImGui::SetItemDefaultFocus();
        // if (ImGui::IsItemHovered() && units.data[i].name && *units.data[i].name)
        //   ImGui::SetTooltip("%s", units.data[i].name);
      }
      ImGui::EndCombo();
    }

    ImGui::EndTable();
  }
  
  ImGui::PopID();
  return changed;
}

void ShowWarning(const std::string& warning_msg) {
  ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f)); // Red text
  ImGui::Text("[Warning]: %s", warning_msg.c_str());
  ImGui::PopStyleColor();  
}

void ShowTextInput(std::string& text, const std::string& label, std::function<void()> rightSideWidget) {
  ImGui::PushID(&text);

  if (ImGui::BeginTable("##table", 3, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_NoBordersInBody)) {
    ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthStretch,  col1_width);
    ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch,  col2_width);
    ImGui::TableSetupColumn("Widget", ImGuiTableColumnFlags_WidthStretch, col3_width);

    ImGui::TableNextColumn();
    ImGui::Text("%s", label.c_str());

    ImGui::TableNextColumn();
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    ImGui::InputText("##Input", &text);

    ImGui::TableNextColumn();
    if (rightSideWidget) rightSideWidget();

    ImGui::EndTable();
  }

  ImGui::PopID();
}

void ShowTextInput(std::string& text, const std::string& label) {
  ImGui::PushID(&text);

  if (ImGui::BeginTable("##table", 2, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_NoBordersInBody)) {
    ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthStretch,  col1_width);
    ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch,  1 - col1_width);

    ImGui::TableNextColumn();
    ImGui::Text("%s", label.c_str());

    ImGui::TableNextColumn();
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    ImGui::InputText("##Input", &text);

    ImGui::EndTable();
  }

  ImGui::PopID();
}

void ShowTextWidget(const char* label, std::function<void()> rightSideWidget) {
  std::string table_id = std::string("##table_") + label;

  if (ImGui::BeginTable(table_id.c_str(), 2, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_NoBordersInBody)) {
    ImGui::PushID(label);

    ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthStretch,  col1_width);
    ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch,  1 - col1_width);

    ImGui::TableNextColumn();
    ImGui::Text("%s", label);

    ImGui::SetNextItemWidth(-FLT_MIN); 
    ImGui::TableNextColumn();
    if (rightSideWidget) rightSideWidget();

    ImGui::PopID();
    ImGui::EndTable();
  }
}

bool EnumCombo(const char* id, int& idx, const std::vector<std::string>& labels) {
  bool changed = false;
  if (ImGui::BeginCombo(id, labels[idx].c_str())) {
    for (int i = 0; i < (int)labels.size(); ++i) {
      const bool sel = (i == idx);
      if (ImGui::Selectable(labels[i].c_str(), sel)) { idx = i; changed = true; }
      if (sel) ImGui::SetItemDefaultFocus();
    }
    ImGui::EndCombo();
  }
  return changed;
}

bool EnumComboImpl(
  const char* id, 
  int& idx,
  const std::string_view* labels, 
  int count
) {
  if (count <= 0) return false;
  idx = std::clamp(idx, 0, count - 1);
  bool changed = false;

  // OK for string literals (null-terminated). If not guaranteed, copy to std::string.
  const char* preview = labels[idx].data();

  if (ImGui::BeginCombo(id, preview)) {
    for (int i = 0; i < count; ++i) {
      const bool sel = (i == idx);
      if (ImGui::Selectable(labels[i].data(), sel)) { idx = i; changed = true; }
      if (sel) ImGui::SetItemDefaultFocus();
    }
    ImGui::EndCombo();
  }
  return changed;
}

bool ComboFromSet(const char* id, const std::set<int>& values, int& selected) {
  bool changed = false;
  if (ImGui::BeginCombo(id, std::to_string(selected).c_str())) {
    for (int v : values) {
      const bool sel = (v == selected);
      if (ImGui::Selectable(std::to_string(v).c_str(), sel)) { selected = v; changed = true; }
      if (sel) ImGui::SetItemDefaultFocus();
    }
    ImGui::EndCombo();
  }
  return changed;
}

void ShowComponentMoleFraction(std::unordered_map<std::string, double>& mf_map, const std::vector<std::string>& components) {
  double totalMoleFraction = 0.0;
  static QuantityEditState mole_frac_idx = {0};
  for (const std::string& component : components) {
    // ShowDoubleInput(mf_map[component], mole_frac_idx, component.c_str(), MakeUnitSet(kMoleFractionUnits));
    mf_map[component] = std::clamp(mf_map[component], 0.0, 1.0);
    totalMoleFraction += mf_map[component];
  }

  // ShowDoubleInput(totalMoleFraction, mole_frac_idx, "Total", MakeUnitSet(kMoleFractionUnits));

  if (std::abs(totalMoleFraction - 1) > 1e-8)
    ShowWarning("Total mole fraction != 1");
}

void HelpMarker(const char* desc) {
  ImGui::TextDisabled("(?)");
  if (ImGui::BeginItemTooltip())
  {
    ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
    ImGui::TextUnformatted(desc);
    ImGui::PopTextWrapPos();
    ImGui::EndTooltip();
  }
}

void ShowErrorMessage(const std::string& errorMessage) {
  ImGui::Begin("Error Message");
  ImGui::TextUnformatted(errorMessage.c_str());
  ImGui::End();
}

void ShowModalPopUp(const std::string& message, bool& open) {
  ImGui::OpenPopup("Message");

  if (ImGui::BeginPopupModal("Message", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::TextUnformatted(message.c_str());

    if (ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter)) {
      ImGui::CloseCurrentPopup();
      open = false;
    }

    if (ImGui::Button("OK")) {
      ImGui::CloseCurrentPopup();
      open = false;
    }
    
    ImGui::EndPopup();
  }
}

void ShowConfirmationPopUp(const std::string& message, bool& open, bool& confirm) {
  ImGui::OpenPopup("Confirmation");

  if (ImGui::BeginPopupModal("Confirmation", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::TextUnformatted(message.c_str());

    if (ImGui::BeginTable("##StepSelectionTable", 2, ImGuiTableFlags_SizingStretchSame)) {
      ImGui::TableNextColumn();
      if (ImGui::Button("Yes", ImVec2(-1, 0))) {
        ImGui::CloseCurrentPopup();
        confirm = true;
        open = false;
      }

      ImGui::TableNextColumn();
      ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
      if (ImGui::Button("No", ImVec2(-1, 0))) {
        ImGui::CloseCurrentPopup();
        confirm = false;
        open = false;
      }

      ImGui::EndTable();
    }
    ImGui::EndPopup();
  }
}

bool ShowStreamCombo(const char* label, px::Handle<px::Stream>& current_handle, const std::vector<StreamItem>& streams) {
  ImGui::PushID(&current_handle); // unique + stable per field
  
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
  
  bool changed = false;
  
  ImGuiTableFlags flags = ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_NoBordersInBody;
  
  if (ImGui::BeginTable("##stream_combo_table", 2, flags, ImVec2(-FLT_MIN, 0.0f))) {
    ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthStretch, col1_width);
    ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch, 1 - col1_width);
    
    ImGui::TableNextColumn();
    ImGui::TextUnformatted(label);
    
    ImGui::TableNextColumn();
    ImGui::SetNextItemWidth(-FLT_MIN); // Fill entire column
    
    if (ImGui::BeginCombo("##stream_combo", combo_preview.c_str())) {
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
          changed = true;
          ImGui::EndCombo();
          ImGui::EndTable();
          ImGui::PopID();
          return true;
        }
        if (is_selected) {
          ImGui::SetItemDefaultFocus();
        }
      }
      ImGui::EndCombo();
    }
    
    ImGui::EndTable();
  }
  
  ImGui::PopID();
  return changed;
}
