#pragma once

#include "processX/flowsheet.h"
#include <vector>
#include <string>
#include <functional>

#include <imgui.h>

// Global flowsheet instance
extern px::Flowsheet flowsheet;

// Selection state - tracks which unit is currently selected
enum class SelectionType { None, Valve, Mixer, Splitter, Stream, HeatExchanger, SimpleHeatExchanger, Pump, ComponentSplitter };

struct Selection {
  SelectionType type = SelectionType::None;
  uint32_t index = UINT32_MAX;
  
  bool valid() const { return type != SelectionType::None && index != UINT32_MAX; }
  void clear() { type = SelectionType::None; index = UINT32_MAX; }
};

extern Selection selected_unit;

// Helper function to build stream list for dropdown
struct StreamItem {
  px::Handle<px::Stream> handle;
  std::string display_name;
};

std::vector<StreamItem> GetStreamList();

// Helper to show stream dropdown and update handle
bool StreamCombo(const char* label, px::Handle<px::Stream>& current_handle, const std::vector<StreamItem>& streams);

// Helper to show stream dropdown with optional delete button
bool StreamComboWithDelete(const char* label, px::Handle<px::Stream>& current_handle, const std::vector<StreamItem>& streams, const std::function<void()>& on_delete = nullptr);

// Function to serialize flowsheet to JSON string
std::string GetFlowsheetJSONString();

// Function to load flowsheet from JSON string
bool LoadFlowsheetFromJSONString(const std::string& json_string);

// Function to check if data was just loaded (for modal popup)
bool ShouldShowDataLoadedModal();
void ClearDataLoadedModal();

#ifdef EMSCRIPTEN
// Expose function to JavaScript using Emscripten
extern "C" {
  const char* GetFlowsheetJSONPtr();
  int LoadFlowsheetFromJSON(const char* json_string);
}
void SetupGetFlowsheetJSON();
void SetupLoadFlowsheetJSON();
#endif


