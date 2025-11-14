#pragma once

// ImGui Includes
#include "hello_imgui/hello_imgui.h"

// Standard library
#include <vector>
#include <string>
#include <set>

// Stokesian includes
#include "units.h"

// Forward declaration for Var
namespace px { struct Var; }


// Declare flags; implemented in .cpp file
extern const ImGuiInputTextFlags_ inputDoubleFlags;
extern const ImGuiTreeNodeFlags treeNodeFlags;

static const float col1_width = 0.375;
static const float col2_width = 0.375;
static const float col3_width = 0.25;

void ShowEquationImg(const std::string& img_path, const char* label = "Equation");
// void ShowDoubleInput(double& val, const std::string& label, const std::string& unit, const std::string& format);
// bool ShowDoubleInput(double& base_value, QuantityEditState& ui_state, const char* label, UnitSet units, const char* value_format = "%.6f");
bool ShowDoubleInput(px::Var& var, QuantityEditState& ui_state, const char* label, UnitSet units, const char* value_format = "%.6f");
// void ShowDoubleInputReadOnly(double& val, const std::string& label, const std::string& unit, const std::string& format);
// void ShowDoubleInputReadOnly(double& base_value, QuantityEditState& ui_state, const char* label, UnitSet units, const char* value_format = "%.6f");
void ShowTextInput(std::string& text, const std::string& label, std::function<void()> rightSideWidget);
void ShowTextInput(std::string& text, const std::string& label);
void ShowTextWidget(const char* label, std::function<void()> rightSideWidget);
void ShowIntInput(int& val, const std::string& label, const std::string& unit);
bool ShowIntInput(int& base_value, QuantityEditState& ui_state, const char* label, UnitSet units);
void ShowComponentMoleFraction(std::unordered_map<std::string, double>& mf_map, const std::vector<std::string>& components);
bool EnumCombo(const char* id, int& idx, const std::vector<std::string>& labels);
bool EnumComboImpl(const char* id, int& idx, const std::string_view* labels, int count);
bool ComboFromSet(const char* id, const std::set<int>& values, int& selected);

// Forward declarations
namespace px { template<typename T> class Handle; class Stream; }
struct StreamItem;

// Stream combo with consistent table layout
bool ShowStreamCombo(const char* label, px::Handle<px::Stream>& current_handle, const std::vector<StreamItem>& streams);
void ShowWarning(const std::string& warning_msg);
std::vector<HelloImGui::DockingSplit> CreateThreeColumnDockingSplits();
HelloImGui::DockingParams CreateDefaultLayout();
void InitializeImGuiFonts();
void HelpMarker(const char* desc);
void ShowErrorMessage(const std::string& errorMessage);
void ShowModalPopUp(const std::string& message, bool& open);
void ShowConfirmationPopUp(const std::string& message, bool& open, bool& confirm);

template<typename T>
bool ShowLabeledComboBox(
  const char* labelText,
  int& selectedIndex,
  const char* const* itemList,
  int itemCount,
  std::function<void(T)> onChanged
  ) {
    bool valueChanged = false;

    // Make table fill all available width
    ImGuiTableFlags flags = ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_NoBordersInBody;
    std::string table_id = std::string("##combo_tbl_") + labelText;

    if (ImGui::BeginTable(table_id.c_str(), 2, flags, ImVec2(-FLT_MIN, 0.0f))) {
      ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthStretch, col1_width);
      ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch, 1 - col1_width);

      ImGui::TableNextColumn();
      ImGui::TextUnformatted(labelText);

      ImGui::TableNextColumn();
      ImGui::PushID(labelText);
      ImGui::SetNextItemWidth(-FLT_MIN); // Fill entire column
      if (ImGui::Combo("##combo", &selectedIndex, itemList, itemCount)) {
        valueChanged = true;
        if (onChanged) {
            onChanged(static_cast<T>(selectedIndex));
        }
      }
      ImGui::PopID();

      ImGui::EndTable();
    }

    return valueChanged;
}

template <size_t N>
bool EnumCombo(const char* id, int& idx, const std::array<std::string_view, N>& labels) {
  return EnumComboImpl(id, idx, labels.data(), static_cast<int>(N));
}

// raw C-array of std::string_view
template <size_t N>
bool EnumCombo(const char* id, int& idx, const std::string_view (&labels)[N]) {
  return EnumComboImpl(id, idx, labels, static_cast<int>(N));
}

struct ImWindow {
  ImWindow(const char* name) { ImGui::Begin(name); }
  ~ImWindow() { ImGui::End(); }
};
