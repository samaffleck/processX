#pragma once

#include "gui_common.h"
#include <vector>
#include <string>
#include <set>

// Show the fluid packages management window
void ShowFluidPackagesWindow();

// Simplified dual list box for component selection
struct FluidSelectorListBox {
    std::vector<int> AvailableItems;      // Indices into component_names array
    std::vector<int> SelectedItems;       // Indices into component_names array
    std::set<int> AvailableSelection;    // Selected items in available list
    std::set<int> SelectedSelection;      // Selected items in selected list
    bool OptKeepSorted = true;

    // Update the lists based on current package components
    void UpdateLists(const std::vector<std::string>& current_components, 
                     const char* component_names[], int num_components);
    
    // Move selected items from src (0=available, 1=selected) to dst
    void MoveSelected(int src, int dst, size_t pkg_id);
    
    // Sort items in a list
    void SortItems(int side);
    
    // Show the dual list box UI
    void Show(size_t pkg_id);
};
