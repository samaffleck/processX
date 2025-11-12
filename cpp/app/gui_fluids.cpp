#include "gui_fluids.h"
#include "gui_common.h"
#include "gui_log.h"
#include "processX/flowsheet.h"
#include "processX/fluids.h"
#include <imgui.h>
#include <vector>
#include <string>
#include <algorithm>
#include <cstring>
#include <cstdio>
#include <set>
#include <map>
#include <cfloat>

using namespace px;

void ShowFluidPackagesWindow() {  
  std::vector<size_t> package_ids = flowsheet.fluids.GetAllPackageIds();
  
  if (ImGui::Button("Create New Package")) {
    size_t new_id = flowsheet.fluids.AddFluidPackage({}, "HEOS", "");
    AddLogEntry(LogEntry::Info, "Created new fluid package");
  }
  
  ImGui::Spacing();
  ImGui::Separator();
  ImGui::Spacing();
  
  // List of packages
  if (package_ids.empty()) {
    ImGui::Text("No fluid packages created yet.");
  } else {
    ImGui::Text("Fluid Packages");
    ImGui::Spacing();
    
    // Track which package to delete (after iteration)
    static size_t package_to_delete = 0;
    static bool show_delete_confirm = false;
    
    for (size_t pkg_id : package_ids) {
      ImGui::PushID(static_cast<int>(pkg_id));
      
      // Get package name
      std::string package_name = flowsheet.fluids.GetPackageName(pkg_id);
      
      // Package header with name (editable)
      bool is_open = ImGui::CollapsingHeader(package_name.c_str(), 
                                             ImGuiTreeNodeFlags_DefaultOpen);
      
      if (is_open) {
        // Package name editing
        ImGui::Text("Package Name:");
        ImGui::SameLine(150);
        char name_buffer[256];
        strncpy(name_buffer, package_name.c_str(), sizeof(name_buffer));
        name_buffer[sizeof(name_buffer) - 1] = '\0';
        ImGui::PushItemWidth(300);
        if (ImGui::InputText("##PackageName", name_buffer, sizeof(name_buffer))) {
          std::string new_name(name_buffer);
          if (!new_name.empty()) {
            flowsheet.fluids.SetPackageName(pkg_id, new_name);
          }
        }
        ImGui::PopItemWidth();
        
        ImGui::Spacing();
        
        // Delete button
        if (ImGui::Button("Delete Package")) {
          package_to_delete = pkg_id;
          show_delete_confirm = true;
          ImGui::OpenPopup("Delete Package?");
        }
        ImGui::Spacing();
        
        // Get package info
        auto components = flowsheet.fluids.GetComponents(pkg_id);
        std::string thermo_pkg = flowsheet.fluids.GetThermoPackage(pkg_id);
        
        // Backend selection
        ImGui::Text("Thermodynamic Backend:");
        ImGui::SameLine(200);
        
        // Find current backend index
        int current_backend_idx = 0;
        for (int i = 0; i < px::kNumberOfBackends; ++i) {
          if (thermo_pkg == px::backend_names[i]) {
            current_backend_idx = i;
            break;
          }
        }
        
        // Backend combo
        if (ImGui::Combo("##Backend", &current_backend_idx, px::backend_names, px::kNumberOfBackends)) {
          // Backend changed - need to recreate package
          std::string new_backend = px::backend_names[current_backend_idx];
          // Remove old and create new with same components
          std::string old_name = flowsheet.fluids.GetPackageName(pkg_id);
          flowsheet.fluids.RemoveFluidPackage(pkg_id);
          [[maybe_unused]] size_t new_id = flowsheet.fluids.AddFluidPackage(components, new_backend, old_name);
          AddLogEntry(LogEntry::Info, "Changed backend for package #" + std::to_string(pkg_id) 
                     + " to " + new_backend + " (new ID: " + std::to_string(new_id) + ")");
          // Note: This changes the ID, which might break references
          // In a production system, you'd want to preserve IDs or update references
          ImGui::PopID();
          ImGui::Unindent();
          continue; // Skip rest of this package since it was recreated
        }
        
        ImGui::Spacing();
        
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
      }
      
      ImGui::PopID();
      ImGui::Spacing();
    }
    
    // Delete confirmation popup
    if (show_delete_confirm) {
      ImVec2 center = ImGui::GetMainViewport()->GetCenter();
      ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
      
      if (ImGui::BeginPopupModal("Delete Package?", nullptr, 
                                 ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove)) {
        std::string pkg_name = flowsheet.fluids.GetPackageName(package_to_delete);
        ImGui::Text("Are you sure you want to delete '%s'?", pkg_name.c_str());
        ImGui::Text("This will remove the fluid package from the registry.");
        ImGui::Text("Streams using this package will need to be reassigned.");
        ImGui::Spacing();
        
        if (ImGui::Button("Cancel", ImVec2(120, 0)) || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
          show_delete_confirm = false;
          ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Delete", ImVec2(120, 0)) || ImGui::IsKeyPressed(ImGuiKey_Enter)) {
          std::string deleted_name = flowsheet.fluids.GetPackageName(package_to_delete);
          flowsheet.fluids.RemoveFluidPackage(package_to_delete);
          AddLogEntry(LogEntry::Info, "Deleted fluid package: " + deleted_name);
          show_delete_confirm = false;
          ImGui::CloseCurrentPopup();
        }
        
        ImGui::EndPopup();
      }
    }
  }
}


void FluidSelectorListBox::UpdateLists(const std::vector<std::string>& current_components, 
                                        const char* component_names[], int num_components) {
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
      
      // Update the fluid package
      std::string comp_name(px::component_names[idx]);
    auto components = flowsheet.fluids.GetComponents(pkg_id);
    std::string backend = flowsheet.fluids.GetThermoPackage(pkg_id);
    
    if (dst == 1) {
      // Moved to "Selected" - add component
      if (std::find(components.begin(), components.end(), comp_name) == components.end()) {
        components.push_back(comp_name);
        std::string pkg_name = flowsheet.fluids.GetPackageName(pkg_id);
        flowsheet.fluids.RemoveFluidPackage(pkg_id);
        flowsheet.fluids.AddFluidPackage(components, backend, pkg_name);
        AddLogEntry(LogEntry::Info, "Added component '" + comp_name + "' to " + pkg_name);
      }
    } else {
      // Moved to "Available" - remove component
      components.erase(std::remove(components.begin(), components.end(), comp_name), components.end());
      std::string pkg_name = flowsheet.fluids.GetPackageName(pkg_id);
      if (components.empty()) {
        flowsheet.fluids.RemoveFluidPackage(pkg_id);
        AddLogEntry(LogEntry::Info, "Removed " + pkg_name + " (last component removed)");
      } else {
        flowsheet.fluids.RemoveFluidPackage(pkg_id);
        flowsheet.fluids.AddFluidPackage(components, backend, pkg_name);
        AddLogEntry(LogEntry::Info, "Removed component '" + comp_name + "' from " + pkg_name);
      }
    }
  }
  
  // Clear selections
  src_sel->clear();
  if (OptKeepSorted) {
    SortItems(dst);
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
          if (ImGui::Selectable(px::component_names[item_idx], item_is_selected, 
                               ImGuiSelectableFlags_AllowDoubleClick)) {
            // Toggle selection
            if (ImGui::GetIO().KeyCtrl) {
              // Multi-select with Ctrl
              if (item_is_selected) {
                selection.erase(item_idx);
              } else {
                selection.insert(item_idx);
              }
            } else {
              // Single select
              selection.clear();
              selection.insert(item_idx);
            }
          }
          
          if (ImGui::IsItemFocused()) {
            if (ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter)) {
              request_move_selected = side;
            }
            if (ImGui::IsMouseDoubleClicked(0)) {
              request_move_selected = side;
            }
          }
          ImGui::PopID();
        }
      }
      ImGui::EndChild();
    }

    ImGui::TableSetColumnIndex(1);
    ImGui::NewLine();
    ImVec2 button_sz = { ImGui::GetFrameHeight(), ImGui::GetFrameHeight() };

    if (ImGui::Button(">", button_sz)) {
      request_move_selected = 0;
    }
    if (ImGui::Button("<", button_sz)) {
      request_move_selected = 1;
    }

    // Process requests
    if (request_move_selected != -1) {
      MoveSelected(request_move_selected, request_move_selected ^ 1, pkg_id);
    }

    ImGui::EndTable();
  }
}
