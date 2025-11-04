// apps/processx_app.cpp
#include <hello_imgui/hello_imgui.h>
#include <imgui.h>

#include <vector>
#include <algorithm>
#include <string>
#include <filesystem>
#include <variant>
#include <optional>
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <ctime>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <cereal/archives/json.hpp>
#include "processX/flowsheet.h" 

#include "node_editor.h"
#include "themes.h"

// Emscripten Includes
#ifdef EMSCRIPTEN
#include <emscripten.h>

EM_JS(void, notify_ready, (), {
  try { parent.postMessage({ type: 'wasmReady' }, window.location.origin); } catch (e) {}
});
#endif


static px::Flowsheet flowsheet{};

// Function to serialize flowsheet to JSON string
std::string GetFlowsheetJSONString() {
  std::ostringstream oss;
  {
    cereal::JSONOutputArchive archive(oss);
    archive(cereal::make_nvp("Flowsheet_Data", flowsheet));
  }
  return oss.str();
}

// Expose function to JavaScript using Emscripten
#ifdef EMSCRIPTEN
// Internal C function that returns JSON string pointer
extern "C" {
  EMSCRIPTEN_KEEPALIVE
  const char* GetFlowsheetJSONPtr() {
    static std::string json_str;
    json_str = GetFlowsheetJSONString();
    return json_str.c_str();
  }
}

// JavaScript wrapper function that converts pointer to string and returns it
EM_JS(void, SetupGetFlowsheetJSON, (), {
  Module.getFlowsheetJSON = function() {
    var ptr = Module._GetFlowsheetJSONPtr();
    if (!ptr) return null;
    
    // Find the length of the null-terminated string
    var length = 0;
    var start = ptr;
    while (HEAP8[ptr]) {
      length++;
      ptr++;
    }
    
    // Read UTF-8 bytes from memory
    var bytes = new Uint8Array(length);
    for (var i = 0; i < length; i++) {
      bytes[i] = HEAP8[start + i];
    }
    
    // Decode UTF-8 to JavaScript string
    // Use TextDecoder if available, otherwise fall back to simple decoding
    if (typeof TextDecoder !== 'undefined') {
      var decoder = new TextDecoder('utf-8');
      return decoder.decode(bytes);
    } else {
      // Fallback: simple ASCII/UTF-8 decoding (works for JSON)
      var result = '';
      for (var i = 0; i < length; i++) {
        result += String.fromCharCode(bytes[i]);
      }
      return result;
    }
  };
});
#endif

// Log system
struct LogEntry {
  std::string message;
  std::string timestamp;
  enum Type { Info, Success, Error } type;
};

static std::vector<LogEntry> log_messages;
static constexpr size_t MAX_LOG_ENTRIES = 1000;

static void AddLogEntry(LogEntry::Type type, const std::string& message) {
  // Get current time
  auto now = std::chrono::system_clock::now();
  auto time_t = std::chrono::system_clock::to_time_t(now);
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
    now.time_since_epoch()) % 1000;
  
  std::stringstream ss;
  ss << std::put_time(std::localtime(&time_t), "%H:%M:%S");
  ss << "." << std::setfill('0') << std::setw(3) << ms.count();
  
  LogEntry entry;
  entry.message = message;
  entry.timestamp = ss.str();
  entry.type = type;
  
  log_messages.push_back(entry);
  
  // Limit log size
  if (log_messages.size() > MAX_LOG_ENTRIES) {
    log_messages.erase(log_messages.begin());
  }
}

static void ShowLogWindow() {
  ImGui::Begin("Log");
  
  // Auto-scroll to bottom if scrolled to bottom before
  static bool auto_scroll = true;
  
  // Options and Solve button
  if (ImGui::Button("Solve")) {
    AddLogEntry(LogEntry::Info, "Starting solve...");
    
    // First, assemble the system
    std::string assemble_error;
    if (!flowsheet.assemble(&assemble_error)) {
      AddLogEntry(LogEntry::Error, "Assembly failed: " + assemble_error);
    } else {
      AddLogEntry(LogEntry::Info, "System assembled successfully");
      
      // Configure solver options
      px::NewtonOptions options;
      options.max_iters = 50;
      options.tol_res = 1e-10;
      options.tol_step = 1e-12;
      options.fd_rel = 1e-6;
      options.fd_abs = 1e-8;
      options.verbose = false; // We'll log ourselves
      
      // Run solver
      px::NewtonReport report = flowsheet.solve(options);
      
      if (report.converged) {
        std::string success_msg = "Solve converged! Iterations: " + std::to_string(report.iters) 
                                  + ", Final residual: " + std::to_string(report.final_res);
        AddLogEntry(LogEntry::Success, success_msg);
        if (!report.msg.empty()) {
          AddLogEntry(LogEntry::Info, "Message: " + report.msg);
        }
      } else {
        std::string error_msg = "Solve failed: " + report.msg;
        if (report.iters > 0) {
          error_msg += " (Iterations: " + std::to_string(report.iters) 
                      + ", Final residual: " + std::to_string(report.final_res) + ")";
        }
        AddLogEntry(LogEntry::Error, error_msg);
      }
    }
  }
  ImGui::SameLine();
  if (ImGui::Button("Clear")) {
    log_messages.clear();
  }
  ImGui::SameLine();
  ImGui::Checkbox("Auto-scroll", &auto_scroll);
  
  ImGui::Separator();
  
  // Log display
  ImGui::BeginChild("LogContent", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
  
  // Enable text wrapping - wrap at window edge (0 = auto)
  ImGui::PushTextWrapPos(0.0f);
  
  for (const auto& entry : log_messages) {
    // Color code by type
    ImVec4 color;
    switch (entry.type) {
      case LogEntry::Info:
        color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // White
        break;
      case LogEntry::Success:
        color = ImVec4(0.0f, 1.0f, 0.0f, 1.0f); // Green
        break;
      case LogEntry::Error:
        color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f); // Red
        break;
    }
    
    ImGui::PushStyleColor(ImGuiCol_Text, color);
    ImGui::TextWrapped("[%s] %s", entry.timestamp.c_str(), entry.message.c_str());
    ImGui::PopStyleColor();
  }
  
  ImGui::PopTextWrapPos();
  
  // Auto-scroll to bottom
  if (auto_scroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 10) {
    ImGui::SetScrollHereY(1.0f);
  }
  
  ImGui::EndChild();
  ImGui::End();
}

// Selection state - tracks which unit is currently selected
enum class SelectionType { None, Valve, Mixer, Stream };
struct Selection {
  SelectionType type = SelectionType::None;
  uint32_t index = UINT32_MAX;
  
  bool valid() const { return type != SelectionType::None && index != UINT32_MAX; }
  void clear() { type = SelectionType::None; index = UINT32_MAX; }
};
static Selection selected_unit{};

static void ShowUnitOperationsPalette() {
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
  if (ImGui::Button("Stream", button_dim)) {
    flowsheet.add<px::Stream>();
    selected_unit.clear();
    AddLogEntry(LogEntry::Info, "Added new Stream");
  }
}

// Helper function to build stream list for dropdown
struct StreamItem {
  px::Handle<px::Stream> handle;
  std::string display_name;
};

static std::vector<StreamItem> GetStreamList() {
  std::vector<StreamItem> streams;
  streams.push_back({px::Handle<px::Stream>{}, "(None)"}); // Option to disconnect
  
  flowsheet.streams_.for_each_with_handle([&](px::Stream& stream, px::Handle<px::Stream> handle) {
    std::string display_name = stream.name.empty() ? "(Unnamed Stream)" : stream.name;
    streams.push_back({handle, display_name});
  });
  
  return streams;
}

// Helper to show stream dropdown and update handle
static bool StreamCombo(const char* label, px::Handle<px::Stream>& current_handle, const std::vector<StreamItem>& streams) {
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
  
  if (ImGui::BeginCombo(label, combo_preview.c_str())) {
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
        ImGui::EndCombo();
        return true;
      }
      if (is_selected) {
        ImGui::SetItemDefaultFocus();
      }
    }
    ImGui::EndCombo();
  }
  return false;
}

static void ShowValveProperties(px::Valve& valve) {
  ImGui::Text("Valve Properties");
  ImGui::Separator();

  // Name editing
  char name_buffer[256];
  strncpy(name_buffer, valve.get_name().c_str(), sizeof(name_buffer));
  name_buffer[sizeof(name_buffer) - 1] = '\0';
  if (ImGui::InputText("##Name", name_buffer, sizeof(name_buffer))) {
    valve.name = name_buffer;
  }

  ImGui::Spacing();

  // Cv editing
  ImGui::Text("Flow Coefficient (Cv):");
  ImGui::PushItemWidth(200);
  if (ImGui::InputDouble("##Cv", &valve.Cv.value)) {
    // Value updated
  }
  ImGui::SameLine();
  ImGui::Checkbox("Fixed##CvFixed", &valve.Cv.fixed);
  ImGui::PopItemWidth();

  ImGui::Spacing();

  // Connections with dropdowns
  ImGui::Text("Connections:");
  
  // Build stream list
  std::vector<StreamItem> stream_list = GetStreamList();
  
  // Inlet dropdown
  ImGui::Text("Inlet:");
  ImGui::SameLine(100);
  StreamCombo("##ValveInlet", valve.in, stream_list);
  
  // Outlet dropdown
  ImGui::Text("Outlet:");
  ImGui::SameLine(100);
  StreamCombo("##ValveOutlet", valve.out, stream_list);
}

static void ShowMixerProperties(px::Mixer& mixer) {
  ImGui::Text("Mixer Properties");
  ImGui::Separator();

  // Name editing
  char name_buffer[256];
  strncpy(name_buffer, mixer.get_name().c_str(), sizeof(name_buffer));
  name_buffer[sizeof(name_buffer) - 1] = '\0';
  if (ImGui::InputText("##Name", name_buffer, sizeof(name_buffer))) {
    mixer.name = name_buffer;
  }

  ImGui::Spacing();

  // Connections with dropdowns
  ImGui::Text("Connections:");
  
  // Build stream list
  std::vector<StreamItem> stream_list = GetStreamList();
  
  // Inlets - list with dropdowns
  ImGui::Text("Inlets:");
  for (size_t i = 0; i < mixer.in.size(); ++i) {
    ImGui::PushID(static_cast<int>(i));
    ImGui::Text("  [%zu]", i);
    ImGui::SameLine(50);
    char label[32];
    snprintf(label, sizeof(label), "##MixerInlet%zu", i);
    
    px::Handle<px::Stream>& inlet_handle = mixer.in[i];
    if (StreamCombo(label, inlet_handle, stream_list)) {
      // Connection updated
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Remove##RemoveInlet")) {
      mixer.in.erase(mixer.in.begin() + i);
      ImGui::PopID();
      break; // Exit loop since vector changed
    }
    ImGui::PopID();
  }
  
  if (ImGui::Button("Add Inlet")) {
    mixer.in.push_back(px::Handle<px::Stream>{});
  }
  
  ImGui::Spacing();
  
  // Outlet dropdown
  ImGui::Text("Outlet:");
  ImGui::SameLine(100);
  StreamCombo("##MixerOutlet", mixer.out, stream_list);
}

static void ShowStreamProperties(px::Stream& stream) {
  ImGui::Text("Stream Properties");
  ImGui::Separator();

  // Name editing
  char name_buffer[256];
  strncpy(name_buffer, stream.name.c_str(), sizeof(name_buffer));
  name_buffer[sizeof(name_buffer) - 1] = '\0';
  if (ImGui::InputText("##Name", name_buffer, sizeof(name_buffer))) {
    stream.name = name_buffer;
  }

  ImGui::Spacing();

  // Properties editing
  ImGui::Text("Properties:");
  
  ImGui::PushItemWidth(200);
  
  ImGui::Text("Molar Flowrate (mol/s):");
  if (ImGui::InputDouble("##MolarFlow", &stream.molar_flow.value)) {
    // Value updated
  }
  ImGui::SameLine();
  ImGui::Checkbox("Fixed##MolarFlowFixed", &stream.molar_flow.fixed);

  ImGui::Text("Pressure (Pa):");
  if (ImGui::InputDouble("##Pressure", &stream.pressure.value)) {
    // Value updated
  }
  ImGui::SameLine();
  ImGui::Checkbox("Fixed##PressureFixed", &stream.pressure.fixed);

  ImGui::Text("Temperature (K):");
  if (ImGui::InputDouble("##Temperature", &stream.temperature.value)) {
    // Value updated
  }
  ImGui::SameLine();
  ImGui::Checkbox("Fixed##TemperatureFixed", &stream.temperature.fixed);

  ImGui::PopItemWidth();
}

static void ShowSelectedUnitProperties() {
  if (!selected_unit.valid()) {
    ImGui::Text("No unit selected");
    ImGui::Text("Select a unit from the flowsheet list to view/edit its properties.");
    return;
  }

  // Get the selected unit based on type and index
  bool found = false;
  if (selected_unit.type == SelectionType::Valve) {
    uint32_t idx = 0;
    flowsheet.valves_.for_each_with_handle([&](px::Valve& valve, px::Handle<px::Valve> handle) {
      if (!found && idx == selected_unit.index) {
        ShowValveProperties(valve);
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        // Delete button
        if (ImGui::Button("Delete Valve", ImVec2(-1, 0))) {
          flowsheet.erase(handle);
          selected_unit.clear();
          found = true;
          return;
        }
        found = true;
      }
      idx++;
    });
  } else if (selected_unit.type == SelectionType::Mixer) {
    uint32_t idx = 0;
    flowsheet.mixers_.for_each_with_handle([&](px::Mixer& mixer, px::Handle<px::Mixer> handle) {
      if (!found && idx == selected_unit.index) {
        ShowMixerProperties(mixer);
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        // Delete button
        if (ImGui::Button("Delete Mixer", ImVec2(-1, 0))) {
          flowsheet.erase(handle);
          selected_unit.clear();
          found = true;
          return;
        }
        found = true;
      }
      idx++;
    });
  } else if (selected_unit.type == SelectionType::Stream) {
    uint32_t idx = 0;
    flowsheet.streams_.for_each_with_handle([&](px::Stream& stream, px::Handle<px::Stream> handle) {
      if (!found && idx == selected_unit.index) {
        ShowStreamProperties(stream);
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        // Delete button
        if (ImGui::Button("Delete Stream", ImVec2(-1, 0))) {
          flowsheet.erase(handle);
          selected_unit.clear();
          found = true;
          return;
        }
        found = true;
      }
      idx++;
    });
  }
  
  if (!found) {
    ImGui::Text("Selected unit not found. It may have been deleted.");
    selected_unit.clear();
  }
}

static void ShowUnitOperations() {
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

  if (flowsheet.valves_.size_alive() == 0 && 
      flowsheet.mixers_.size_alive() == 0 && 
      flowsheet.streams_.size_alive() == 0) {
    ImGui::Text("No units in flowsheet");
    ImGui::Text("Add units using the Unit Operations palette.");
  }
}

// Docking layout functions
std::vector<HelloImGui::DockingSplit> CreateDefaultDockingSplits()
{
  // Layout structure:
  //    ┌──────────┬──────────────────┬──────────────┐
  //    │          │   Flowsheet      │  Properties  │
  //    │   Log    │   (top right)    │  (top right) │
  //    │  (25%)   │                  │              │
  //    │          ├──────────────────┴──────────────┤
  //    │          │                                  │
  //    │          │      Palette (bottom right)      │
  //    │          │                                  │
  //    └──────────┴──────────────────────────────────┘
  
  // Step 1: Split MainDockSpace left to create Log area (25% width)
  // This leaves MainDockSpace as the right side (75% width)
  HelloImGui::DockingSplit splitLeft;
  splitLeft.initialDock = "MainDockSpace";
  splitLeft.newDock = "LogSpace";
  splitLeft.direction = ImGuiDir_Left;
  splitLeft.ratio = 0.25f;
  
  // Step 2: Split MainDockSpace down to create bottom area for Palette (25% height)
  // This leaves MainDockSpace as the top right area (75% height)
  HelloImGui::DockingSplit splitBottom;
  splitBottom.initialDock = "MainDockSpace";
  splitBottom.newDock = "BottomRightSpace";
  splitBottom.direction = ImGuiDir_Down;
  splitBottom.ratio = 0.25f;
  
  // Step 3: Split MainDockSpace (top right) right to create Properties area (50% width of top right)
  // This leaves MainDockSpace as Flowsheet (50% width of top right)
  HelloImGui::DockingSplit splitRight;
  splitRight.initialDock = "MainDockSpace";
  splitRight.newDock = "RightSpace";
  splitRight.direction = ImGuiDir_Right;
  splitRight.ratio = 0.50f;
  
  std::vector<HelloImGui::DockingSplit> splits {splitLeft, splitBottom, splitRight};
  return splits;
}

std::vector<HelloImGui::DockableWindow> CreateDockableWindows()
{
  std::vector<HelloImGui::DockableWindow> windows;
  
  // All windows have empty GuiFunction since we render them manually in ShowGui()
  // The DockableWindow entries are just for docking layout - they tell hello_imgui where to dock windows
  
  // Log - left side (25% width)
  windows.emplace_back("Log", "LogSpace", []() {}, true, true);
  windows.back().callBeginEnd = false;
  
  // Flowsheet - top right, shares space with Properties (50% of top right)
  windows.emplace_back("Flowsheet", "MainDockSpace", []() {}, true, true);
  windows.back().callBeginEnd = false;
  
  // Properties - top right, shares space with Flowsheet (50% of top right)
  windows.emplace_back("Properties", "RightSpace", []() {}, true, true);
  windows.back().callBeginEnd = false;
  
  // Palette (Unit Operations) - bottom right
  windows.emplace_back("Unit Operations", "BottomRightSpace", []() {}, true, true);
  windows.back().callBeginEnd = false;
  
  return windows;
}

HelloImGui::DockingParams CreateDefaultLayout()
{
  HelloImGui::DockingParams dockingParams;
  // Set up docking splits to create the layout structure
  dockingParams.dockingSplits = CreateDefaultDockingSplits();
  // Register dockable windows with their names and dock spaces - this tells hello_imgui where to dock them
  dockingParams.dockableWindows = CreateDockableWindows();
  return dockingParams;
}

// ShowGui callback - runs every frame, renders all dockable windows
static void ShowGui()
{
  // Initialize on first load (runs once)
  static bool on_first_load = true;
  if (on_first_load) {
    on_first_load = false;
    AddLogEntry(LogEntry::Info, "Application started");
    Themes::SetFluentUIColors();
    
    #ifdef EMSCRIPTEN
    SetupGetFlowsheetJSON();
    #endif
  }

  // Render all dockable windows
  ImGui::Begin("Unit Operations");
  ShowUnitOperationsPalette();
  ImGui::End();

  ImGui::Begin("Flowsheet");
  ShowUnitOperations();
  ImGui::End();

  ImGui::Begin("Properties");
  ShowSelectedUnitProperties();
  ImGui::End();
  
  ShowLogWindow();
}

void InitializeImGuiFonts() {
  HelloImGui::ImGuiDefaultSettings::LoadDefaultFont_WithFontAwesomeIcons();

  // Load DejaVuSans font for math symbols
  // Use hello_imgui's LoadFont helper which handles cross-platform loading automatically
  try {
    // Unicode Math Symbols block
    static const ImWchar ranges[] = { 0x2200, 0x22FF, 0 };
    
    ImFontConfig config;
    config.MergeMode = true; // Merge with existing font
    config.PixelSnapH = true;
    config.GlyphRanges = ranges;
    
    HelloImGui::FontLoadingParams fontParams;
    fontParams.fontConfig = config;
    fontParams.mergeToLastFont = true; // Merge with the last font (default font)
    fontParams.insideAssets = true; // Load from assets folder
    fontParams.adjustSizeToDpi = true; // Adjust for High DPI
    
    ImFont* font = HelloImGui::LoadFont("fonts/DejaVuSans.ttf", 20.0f, fontParams);
    if (!font) {
      std::cerr << "Warning: Failed to load DejaVuSans.ttf font" << std::endl;
    }
  } catch (const std::exception& e) {
    std::cerr << "Error loading font: " << e.what() << std::endl;
    // Continue without the custom font
  }
}

int main(int, char**) {
  // Set assets folder first, before configuring callbacks that might need assets
  HelloImGui::SetAssetsFolder("assets");
  
  HelloImGui::RunnerParams params;
  params.callbacks.ShowGui = ShowGui;
  params.appWindowParams.windowTitle = "ProcessX";
  params.callbacks.LoadAdditionalFonts = InitializeImGuiFonts;
  params.appWindowParams.windowGeometry.windowSizeState = HelloImGui::WindowSizeState::Maximized;
  params.imGuiWindowParams.defaultImGuiWindowType = HelloImGui::DefaultImGuiWindowType::ProvideFullScreenDockSpace;
  params.imGuiWindowParams.enableViewports = true;
  params.imGuiWindowParams.showMenuBar = false;
  params.imGuiWindowParams.showMenu_App = false;
  params.imGuiWindowParams.showMenu_View = false;
  
  // Set up docking layout - this creates the layout structure on first load
  params.dockingParams = CreateDefaultLayout();  

  #ifdef EMSCRIPTEN
  // Notify parent that WASM app is ready after initialization
  notify_ready();
  #endif

  try {
    HelloImGui::Run(params);
  } catch(const std::exception& e) {
    std::cerr << e.what() << '\n';
  }
  
  return 0;
}
