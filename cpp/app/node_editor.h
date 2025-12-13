#pragma once

#include <math.h> // fmodf
#include <imgui.h>

#include "processX/flowsheet.h"
#include "gui_window_titles.h"

static inline ImVec2 operator+(const ImVec2& lhs, const ImVec2& rhs) { return ImVec2(lhs.x + rhs.x, lhs.y + rhs.y); }
static inline ImVec2 operator-(const ImVec2& lhs, const ImVec2& rhs) { return ImVec2(lhs.x - rhs.x, lhs.y - rhs.y); }

ImVec2 GetInputSlotPos(int slot_no, px::IUnitOp& op) { 
  return ImVec2(op.pos.x, op.pos.y + op.size.y * ((float)slot_no + 1) / ((float)op.num_inputs() + 1)); 
}

ImVec2 GetOutputSlotPos(int slot_no, px::IUnitOp& op) { 
  return ImVec2(op.pos.x + op.size.x, op.pos.y + op.size.y * ((float)slot_no + 1) / ((float)op.num_outputs() + 1)); 
}

static void ShowFlowsheet(bool* opened, px::Flowsheet& fs) {
  if (!ImGui::Begin(WindowTitles::Flowsheet.c_str(), opened)) {
    ImGui::End();
    return;
  }

  // State
  static ImVec2 scrolling = ImVec2(0.0f, 0.0f);
  static bool show_grid = true;
  static int node_selected = -1;

  // Initialization
  ImGuiIO& io = ImGui::GetIO();

  fs.build_unit_list();

  auto& units = fs.units_;
  auto& streams = fs.streams_;

  // Draw a list of nodes on the left side
  bool open_context_menu = false;
  int node_hovered_in_list = -1;
  int node_hovered_in_scene = -1;

  // --- left list --------------------------------------------------------------
  ImGui::BeginChild("unit_list", ImVec2(120, 0));
  ImGui::Text("Add Units");
  ImGui::Separator();
  
  if (ImGui::Button("Stream")) {
    fs.add<px::Stream>();
  }

  if (ImGui::Button("Valve")) {
    fs.add<px::Valve>();
  }

  if (ImGui::Button("Mixer")) {
    fs.add<px::Mixer>();
  }

  ImGui::EndChild();

  ImGui::SameLine();
  
  ImGui::BeginChild("node_list", ImVec2(120, 0));
  ImGui::Text("Units");
  ImGui::Separator();
  for (const auto& unit : units) {
    ImGui::Selectable(unit->name.c_str());
  }

  ImGui::Text("Streams");
  ImGui::Separator();
  ImGui::EndChild();
  
  ImGui::SameLine();
  ImGui::BeginGroup();

  // --- canvas header ----------------------------------------------------------
  const float NODE_SLOT_RADIUS = 4.0f;
  const ImVec2 NODE_WINDOW_PADDING(8.0f, 8.0f);

  ImGui::Checkbox("Show grid", &show_grid);
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(1, 1));
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
  ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(60, 60, 70, 200));
  ImGui::BeginChild("scrolling_region", ImVec2(0, 0), true,
                    ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove);
  ImGui::PopStyleVar(); // WindowPadding
  ImGui::PushItemWidth(120.0f);

  const ImVec2 offset = ImGui::GetCursorScreenPos() + scrolling;
  ImDrawList* draw_list = ImGui::GetWindowDrawList();

  // --- grid -------------------------------------------------------------------
  if (show_grid) {
    const ImU32 GRID_COLOR = IM_COL32(200, 200, 200, 40);
    const float GRID_SZ = 64.0f;
    ImVec2 win_pos = ImGui::GetCursorScreenPos();
    ImVec2 canvas_sz = ImGui::GetWindowSize();
    for (float x = fmodf(scrolling.x, GRID_SZ); x < canvas_sz.x; x += GRID_SZ)
      draw_list->AddLine(ImVec2(x, 0.0f) + win_pos, ImVec2(x, canvas_sz.y) + win_pos, GRID_COLOR);
    for (float y = fmodf(scrolling.y, GRID_SZ); y < canvas_sz.y; y += GRID_SZ)
      draw_list->AddLine(ImVec2(0.0f, y) + win_pos, ImVec2(canvas_sz.x, y) + win_pos, GRID_COLOR);
  }

  // --- nodes ------------------------------------------------------------------
  draw_list->ChannelsSplit(2); // 0: background, 1: foreground

  for (const auto& unit : units) {
    // ImGui::PushID((void*)unit);

    // world->screen
    ImVec2 node_rect_min = offset + ImVec2((float)unit->pos.x, (float)unit->pos.y);

    // ----- foreground: title/contents
    draw_list->ChannelsSetCurrent(1);
    bool old_any_active = ImGui::IsAnyItemActive();
    ImGui::SetCursorScreenPos(node_rect_min + NODE_WINDOW_PADDING);
    ImGui::BeginGroup();
    ImGui::Text("%s", unit->name.c_str());       // title
    // (Optional: add tiny data here)
    ImGui::EndGroup();

    // compute size from emitted items
    bool node_widgets_active = (!old_any_active && ImGui::IsAnyItemActive());
    ImVec2 unit_size = ImGui::GetItemRectSize() + NODE_WINDOW_PADDING + NODE_WINDOW_PADDING;
    unit->size.x = unit_size.x;
    unit->size.y = unit_size.y;
    ImVec2 node_rect_max = node_rect_min + ImVec2((float)unit->size.x, (float)unit->size.y);

    // ----- background: box + hit test + ports
    draw_list->ChannelsSetCurrent(0);
    ImGui::SetCursorScreenPos(node_rect_min);
    ImGui::InvisibleButton("node", unit_size);
    bool node_moving_active = ImGui::IsItemActive();
    if (node_widgets_active || node_moving_active)
      node_selected = (int)(intptr_t)unit; // any stable int is fine

    if (node_moving_active && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
      unit->pos.x += io.MouseDelta.x;      // ✅ drag in flowsheet space
      unit->pos.y += io.MouseDelta.y;
    }

    // draw node box
    const ImU32 bg = IM_COL32(60, 60, 60, 255);
    const ImU32 border = IM_COL32(110, 110, 110, 255);
    draw_list->AddRectFilled(node_rect_min, node_rect_max, bg, 6.0f);
    draw_list->AddRect(node_rect_min, node_rect_max, border, 6.0f);

    // draw input/output ports
    for (int i = 0; i < unit->num_inputs(); ++i)
      draw_list->AddCircleFilled(offset + GetInputSlotPos(i, *unit), NODE_SLOT_RADIUS,
                                IM_COL32(150, 150, 150, 200));
    for (int i = 0; i < unit->num_outputs(); ++i)
      draw_list->AddCircleFilled(offset + GetOutputSlotPos(i, *unit), NODE_SLOT_RADIUS,
                                IM_COL32(150, 150, 150, 200));

    // ImGui::PopID();
  }
  draw_list->ChannelsMerge();

  // --- context menu open (unchanged) ------------------------------------------
  if (ImGui::IsMouseReleased(ImGuiMouseButton_Right))
    if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup) || !ImGui::IsAnyItemHovered()) {
      node_selected = node_hovered_in_list = node_hovered_in_scene = -1;
      open_context_menu = true;
    }
  if (open_context_menu) {
    ImGui::OpenPopup("context_menu");
    // (add items if you want)
  }

  // --- panning ----------------------------------------------------------------
  if (ImGui::IsWindowHovered() && !ImGui::IsAnyItemActive()
      && ImGui::IsMouseDragging(ImGuiMouseButton_Middle, 0.0f))
    scrolling = scrolling + io.MouseDelta;

  ImGui::PopItemWidth();
  ImGui::EndChild();
  ImGui::PopStyleColor();
  ImGui::PopStyleVar();
  ImGui::EndGroup();

  ImGui::End();   // ← missing
}
