#include "gui_fluids.h"
#include "gui_common.h"
#include "gui_log.h"
#include "processX/flowsheet.h"
#include "processX/fluids.h"
#include <Exceptions.h>
#include <imgui.h>
#include <vector>
#include <string>
#include <algorithm>
#include <cstring>
#include <cstdio>
#include <set>
#include <map>
#include <cfloat>
#include <exception>

using namespace px;

static void ShowFluidPackage(size_t pkg_id) {
  ImGui::PushID(static_cast<int>(pkg_id));
  auto& flowsheet = user_data.fs;

  std::string package_name = flowsheet.fluids.GetPackageName(pkg_id);
  
  // Package name editing - use static buffer per package to maintain focus
  static std::map<size_t, std::string> name_buffers;
  static std::map<size_t, bool> is_editing;
  
  // Initialize buffer if needed (only when not actively editing)
  if (name_buffers.find(pkg_id) == name_buffers.end() || 
      (!is_editing[pkg_id] && name_buffers[pkg_id] != package_name)) {
    name_buffers[pkg_id] = package_name;
  }
  
  // Convert to char buffer for ImGui::InputText
  char name_buffer[256];
  strncpy(name_buffer, name_buffers[pkg_id].c_str(), sizeof(name_buffer));
  name_buffer[sizeof(name_buffer) - 1] = '\0';
  
  ImGui::Text("Package Name:");
  ImGui::SameLine(150);
  ImGui::PushItemWidth(300);
  
  // Use EnterReturnsTrue flag so we only update on Enter or when focus is lost
  ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue;
  if (ImGui::InputText("##PackageName", name_buffer, sizeof(name_buffer), flags)) {
    // User pressed Enter - commit the change
    std::string new_name(name_buffer);
    if (!new_name.empty() && new_name != package_name) {
      flowsheet.fluids.SetPackageName(pkg_id, new_name);
    }
    name_buffers[pkg_id] = new_name;
    is_editing[pkg_id] = false;
  } else {
    // Update buffer as user types (but don't commit yet)
    name_buffers[pkg_id] = name_buffer;
    is_editing[pkg_id] = ImGui::IsItemActive();
  }
  
  // Also update when the input loses focus (user clicks away)
  if (ImGui::IsItemDeactivatedAfterEdit()) {
    std::string new_name(name_buffers[pkg_id]);
    if (!new_name.empty() && new_name != package_name) {
      flowsheet.fluids.SetPackageName(pkg_id, new_name);
    }
    is_editing[pkg_id] = false;
  }
  
  ImGui::PopItemWidth();
  
  ImGui::Spacing();
  
  // Get package info
  auto components = flowsheet.fluids.GetComponents(pkg_id);
  
  // Components list using dual list box
  ImGui::Text("Components:");
  
  // Static list box per package
  static std::map<size_t, FluidSelectorListBox> list_boxes;
  auto& list_box = list_boxes[pkg_id];
  
  // Update lists based on current components
  components = flowsheet.fluids.GetComponents(pkg_id); // Refresh
  list_box.UpdateLists(components, px::component_names, px::kNumberOfComponents);
  
  // Show the dual list box
  list_box.Show(pkg_id);
  
  ImGui::Spacing();
  
  ImGui::PopID();
}

void ShowFluidPackagesWindow() {  
  auto& flowsheet = user_data.fs;

  std::vector<size_t> package_ids = flowsheet.fluids.GetAllPackageIds();
  
  if (ImGui::Button("Create New Package")) {
    flowsheet.fluids.AddFluidPackage();
    AddLogEntry(LogEntry::Info, "Created new fluid package");
  }
  
  ImGui::Spacing();
  ImGui::Separator();
  ImGui::Spacing();
  
  // List of packages using tabs
  if (package_ids.empty()) {
    ImGui::Text("No fluid packages created yet.");
  } else {
    if (ImGui::BeginTabBar("FluidPackages")) {
      for (size_t pkg_id : package_ids) {
        std::string package_name = flowsheet.fluids.GetPackageName(pkg_id);
        std::string tab_label = package_name + "##" + std::to_string(pkg_id);
        
        if (ImGui::BeginTabItem(tab_label.c_str())) {
          ShowFluidPackage(pkg_id);
          ImGui::EndTabItem();
        }
      }
      ImGui::EndTabBar();
    }
  }
}

void FluidSelectorListBox::UpdateLists(
  const std::vector<std::string>& current_components, 
  const char* component_names[], 
  int num_components
) {
  AvailableItems.clear();
  SelectedItems.clear();
  AvailableSelection.clear();
  SelectedSelection.clear();
  
  for (int i = 0; i < num_components; ++i) {
    std::string comp_name(component_names[i]);
    bool is_selected = std::find(current_components.begin(), current_components.end(), comp_name) 
                       != current_components.end();
    
    if (is_selected) {
      SelectedItems.push_back(i);
    } else {
      AvailableItems.push_back(i);
    }
  }
  
  if (OptKeepSorted) {
    SortItems(0);
    SortItems(1);
  }
}

void FluidSelectorListBox::MoveSelected(int src, int dst, size_t pkg_id) {
  std::vector<int>* src_list = (src == 0) ? &AvailableItems : &SelectedItems;
  std::vector<int>* dst_list = (dst == 0) ? &AvailableItems : &SelectedItems;
  std::set<int>* src_sel = (src == 0) ? &AvailableSelection : &SelectedSelection;
  
  // Store original state for potential rollback
  std::vector<int> original_src_list = *src_list;
  std::vector<int> original_dst_list = *dst_list;
  
  // Collect items to move (in reverse order to maintain indices)
  std::vector<int> to_move;
  for (int i = static_cast<int>(src_list->size()) - 1; i >= 0; --i) {
    if (src_sel->count((*src_list)[i])) {
      to_move.push_back((*src_list)[i]);
      src_list->erase(src_list->begin() + i);
    }
  }
  
  // Add to destination
  for (int idx : to_move) {
    dst_list->push_back(idx);
  }
  
  // Update the fluid package with new component list
  // Build the new components list from the updated SelectedItems
  std::vector<std::string> new_components;
  for (int idx : SelectedItems) {
    new_components.push_back(px::component_names[idx]);
  }

  auto& flowsheet = user_data.fs;

  std::string pkg_name = flowsheet.fluids.GetPackageName(pkg_id);
  
  if (new_components.empty()) {
    // No components left - remove the package
    flowsheet.fluids.RemoveFluidPackage(pkg_id);
    AddLogEntry(LogEntry::Info, "Removed " + pkg_name + " (last component removed)");
    // Clear selections and return early since package is removed
    src_sel->clear();
    return;
  } else {
    // Try to update components in place with error handling
    try {
      flowsheet.fluids.UpdateComponents(pkg_id, new_components);
      if (dst == 1 && !to_move.empty()) {
        // Components were added
        std::string comp_name = px::component_names[to_move.back()];
        AddLogEntry(LogEntry::Info, "Added component '" + comp_name + "' to " + pkg_name);
      } else if (dst == 0 && !to_move.empty()) {
        // Components were removed
        std::string comp_name = px::component_names[to_move.back()];
        AddLogEntry(LogEntry::Info, "Removed component '" + comp_name + "' from " + pkg_name);
      }
    } catch (const CoolProp::CoolPropBaseError& e) {
      // CoolProp-specific error (e.g., missing binary pair)
      // Build a user-friendly error message with component names instead of CAS numbers
      std::string error_detail = std::string(e.what());
      std::string component_names_str;
      
      if (dst == 1 && !to_move.empty()) {
        // Components were being added - list the ones that failed
        for (size_t i = 0; i < to_move.size(); ++i) {
          if (i > 0) component_names_str += ", ";
          component_names_str += "'" + std::string(px::component_names[to_move[i]]) + "'";
        }
        std::string error_msg = "Failed to add component(s) " + component_names_str + 
                               " to " + pkg_name + ". " + error_detail;
        AddLogEntry(LogEntry::Error, error_msg);
      } else {
        // Generic error
        std::string error_msg = "Failed to update components in " + pkg_name + ": " + error_detail;
        AddLogEntry(LogEntry::Error, error_msg);
      }
      
      // Revert UI state - move components back
      *src_list = original_src_list;
      *dst_list = original_dst_list;
      
      // Don't clear selections so user can see what failed
      return;
    } catch (const std::exception& e) {
      // General exception
      std::string error_detail = std::string(e.what());
      std::string component_names_str;
      
      if (dst == 1 && !to_move.empty()) {
        // Components were being added - list the ones that failed
        for (size_t i = 0; i < to_move.size(); ++i) {
          if (i > 0) component_names_str += ", ";
          component_names_str += "'" + std::string(px::component_names[to_move[i]]) + "'";
        }
        std::string error_msg = "Failed to add component(s) " + component_names_str + 
                               " to " + pkg_name + ". " + error_detail;
        AddLogEntry(LogEntry::Error, error_msg);
      } else {
        std::string error_msg = "Failed to update components in " + pkg_name + ": " + error_detail;
        AddLogEntry(LogEntry::Error, error_msg);
      }
      
      // Revert UI state - move components back
      *src_list = original_src_list;
      *dst_list = original_dst_list;
      
      // Don't clear selections so user can see what failed
      return;
    } catch (...) {
      // Unknown exception
      std::string error_msg = "Unknown error occurred while updating components in " + pkg_name;
      AddLogEntry(LogEntry::Error, error_msg);
      
      // Revert UI state - move components back
      *src_list = original_src_list;
      *dst_list = original_dst_list;
      
      // Don't clear selections so user can see what failed
      return;
    }
  }
}

void FluidSelectorListBox::SortItems(int side) {
  std::vector<int>* list = (side == 0) ? &AvailableItems : &SelectedItems;
  // Sort by component name (alphabetically)
  std::sort(list->begin(), list->end(), [](int a, int b) {
    return std::string(px::component_names[a]) < std::string(px::component_names[b]);
  });
}

void FluidSelectorListBox::Show(size_t pkg_id) {
  // component_names is accessible as it's a static variable in the same file
  if (ImGui::BeginTable("split", 3, ImGuiTableFlags_None)) {
    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);    // Left side
    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);      // Buttons
    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);    // Right side
    ImGui::TableNextRow();

    int request_move_selected = -1;
    float child_height_0 = 0.0f;
    
    for (int side = 0; side < 2; side++) {
      std::vector<int>& items = (side == 0) ? AvailableItems : SelectedItems;
      std::set<int>& selection = (side == 0) ? AvailableSelection : SelectedSelection;

      ImGui::TableSetColumnIndex((side == 0) ? 0 : 2);
      ImGui::Text("%s (%zu)", (side == 0) ? "Available" : "Selected", items.size());

      // Submit scrolling range to avoid glitches on moving/deletion
      const float items_height = ImGui::GetTextLineHeightWithSpacing();
      ImGui::SetNextWindowContentSize(ImVec2(0.0f, items.size() * items_height));

      bool child_visible;
      if (side == 0) {
        ImGui::SetNextWindowSizeConstraints(ImVec2(0.0f, ImGui::GetFrameHeightWithSpacing() * 4), ImVec2(FLT_MAX, FLT_MAX));
        child_visible = ImGui::BeginChild(("0_" + std::to_string(pkg_id)).c_str(), 
                                          ImVec2(-FLT_MIN, ImGui::GetFontSize() * 20), 
                                          ImGuiChildFlags_FrameStyle | ImGuiChildFlags_ResizeY);
        child_height_0 = ImGui::GetWindowSize().y;
      } else {
        child_visible = ImGui::BeginChild(("1_" + std::to_string(pkg_id)).c_str(), 
                                          ImVec2(-FLT_MIN, child_height_0), 
                                          ImGuiChildFlags_FrameStyle);
      }

      if (child_visible) {
        for (size_t item_n = 0; item_n < items.size(); item_n++) {
          int item_idx = items[item_n];
          bool item_is_selected = selection.count(item_idx) > 0;
          
          ImGui::PushID(static_cast<int>(item_idx));
          
          // Use Selectable with proper flags for selection
          ImGuiSelectableFlags selectable_flags = ImGuiSelectableFlags_AllowDoubleClick | ImGuiSelectableFlags_DontClosePopups;
          if (ImGui::Selectable(px::component_names[item_idx], item_is_selected, selectable_flags)) {
            // Handle single click selection
            if (ImGui::GetIO().KeyCtrl) {
              // Multi-select with Ctrl
              if (item_is_selected) {
                selection.erase(item_idx);
              } else {
                selection.insert(item_idx);
              }
            } else {
              // Single select - clear others and select this one
              selection.clear();
              selection.insert(item_idx);
            }
          }
          
          // Handle double-click and Enter key to move items
          if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
            request_move_selected = side;
          }
          if (ImGui::IsItemFocused() && (ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter))) {
            request_move_selected = side;
          }
          
          ImGui::PopID();
        }
      }
      ImGui::EndChild();
    }

    ImGui::TableSetColumnIndex(1);
    ImGui::NewLine();
    ImVec2 button_sz = { ImGui::GetFrameHeight(), ImGui::GetFrameHeight() };

    // Check if there are selected items in the available list (left side)
    bool has_available_selection = !AvailableSelection.empty();
    // Check if there are selected items in the selected list (right side)
    bool has_selected_selection = !SelectedSelection.empty();

    if (ImGui::Button(">", button_sz)) {
      if (has_available_selection) {
        request_move_selected = 0;
      }
    }
    if (ImGui::Button("<", button_sz)) {
      if (has_selected_selection) {
        request_move_selected = 1;
      }
    }

    // Process requests
    if (request_move_selected != -1) {
      MoveSelected(request_move_selected, request_move_selected ^ 1, pkg_id);
    }

    ImGui::EndTable();
  }
}
