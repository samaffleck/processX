#pragma once 

#include "imgui.h"

namespace Themes {

  inline void SetDarkTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // Primary background
    colors[ImGuiCol_WindowBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);  // #131318
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.12f, 0.12f, 0.15f, 1.00f); // #131318
    colors[ImGuiCol_ChildBg] = ImVec4(0.10f, 0.10f, 0.12f, 1.00f); 
    colors[ImGuiCol_PopupBg] = ImVec4(0.18f, 0.18f, 0.22f, 1.00f);
    
    // Progress bar
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.55f, 0.70f, 0.95f, 1.00f);

    // Headers
    colors[ImGuiCol_Header] = ImVec4(0.18f, 0.18f, 0.22f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.30f, 0.30f, 0.40f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.25f, 0.25f, 0.35f, 1.00f);

    // Buttons
    colors[ImGuiCol_Button] = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.30f, 0.32f, 0.40f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.35f, 0.38f, 0.50f, 1.00f);

    // Frame BG
    colors[ImGuiCol_FrameBg] = ImVec4(0.15f, 0.15f, 0.18f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.22f, 0.22f, 0.27f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.25f, 0.25f, 0.30f, 1.00f);

    // Tabs
    colors[ImGuiCol_Tab] = ImVec4(0.18f, 0.18f, 0.22f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.35f, 0.35f, 0.50f, 1.00f);
    colors[ImGuiCol_TabActive] = ImVec4(0.25f, 0.25f, 0.38f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.13f, 0.13f, 0.17f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.20f, 0.20f, 0.25f, 1.00f);

    // Title
    colors[ImGuiCol_TitleBg] = ImVec4(0.12f, 0.12f, 0.15f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.15f, 0.15f, 0.20f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);

    // Borders
    colors[ImGuiCol_Border] = ImVec4(0.20f, 0.20f, 0.25f, 0.50f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

    // Text
    colors[ImGuiCol_Text] = ImVec4(0.90f, 0.90f, 0.95f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.55f, 1.00f);

    // Highlights
    colors[ImGuiCol_CheckMark] = ImVec4(0.50f, 0.70f, 1.00f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.50f, 0.70f, 1.00f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.60f, 0.80f, 1.00f, 1.00f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.50f, 0.70f, 1.00f, 0.50f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.60f, 0.80f, 1.00f, 0.75f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.70f, 0.90f, 1.00f, 1.00f);

    // Scrollbar
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.30f, 0.30f, 0.35f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.40f, 0.50f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.45f, 0.45f, 0.55f, 1.00f);

    // Table colors for dark theme
    colors[ImGuiCol_TableHeaderBg]     = ImVec4(0.18f, 0.18f, 0.22f, 1.00f);  // Slightly lighter than WindowBg for contrast
    colors[ImGuiCol_TableBorderStrong] = ImVec4(0.30f, 0.30f, 0.35f, 1.00f);  // Outer borders (thicker)
    colors[ImGuiCol_TableBorderLight]  = ImVec4(0.22f, 0.22f, 0.27f, 1.00f);  // Inner borders (thinner)


    // Style tweaks
    style.WindowRounding = 5.0f;
    style.FrameRounding = 5.0f;
    style.GrabRounding = 5.0f;
    style.TabRounding = 5.0f;
    style.PopupRounding = 5.0f;
    style.ScrollbarRounding = 5.0f;
    style.WindowPadding = ImVec2(10, 10);
    style.FramePadding = ImVec2(6, 4);
    style.ItemSpacing = ImVec2(8, 6);
    style.PopupBorderSize = 0.f;
  }

  inline void SetLightTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    style.WindowRounding = 5.0f;
    style.FrameRounding = 5.0f;
    style.GrabRounding = 5.0f;
    style.TabRounding = 5.0f;
    style.PopupRounding = 5.0f;
    style.ScrollbarRounding = 5.0f;
    style.WindowPadding = ImVec2(10, 10);
    style.FramePadding = ImVec2(6, 4);
    style.ItemSpacing = ImVec2(8, 6);
    style.PopupBorderSize = 0.f;

    ImVec4 bg_dark = ImVec4(0.92f, 0.92f, 0.92f, 1.00f); // light gray
    ImVec4 bg = ImVec4(0.96f, 0.96f, 0.96f, 1.00f); // very light gray
    ImVec4 bg_light = ImVec4(1.00f, 1.00f, 1.00f, 1.00f); // white
    ImVec4 text = ImVec4(0.15f, 0.15f, 0.15f, 1.00f); // almost black
    ImVec4 text_muted = ImVec4(0.4f, 0.4f, 0.4f, 1.00f);    // mid-gray
    ImVec4 border = ImVec4(0.6f, 0.6f, 0.6f, 1.00f);    // light gray
    ImVec4 border_muted = ImVec4(0.7f, 0.7f, 0.7f, 1.00f);    // lighter gray
    ImVec4 primary = ImVec4(0.55f, 0.70f, 0.95f, 1.00f); // lighter office blue
    ImVec4 secondary = ImVec4(0.54f, 0.41f, 0.41f, 1.00f); // oklch(0.4 0.1 62)
    ImVec4 danger = ImVec4(0.50f, 0.35f, 0.30f, 1.00f); // oklch(0.5 0.05 30)
    ImVec4 warning = ImVec4(0.50f, 0.50f, 0.30f, 1.00f); // oklch(0.5 0.05 100)
    ImVec4 success = ImVec4(0.30f, 0.50f, 0.30f, 1.00f); // oklch(0.5 0.05 160)
    ImVec4 info = ImVec4(0.40f, 0.40f, 0.50f, 1.00f); // oklch(0.5 0.05 260)

    // Apply to ImGui color slots
    colors[ImGuiCol_Text] = text;
    colors[ImGuiCol_TextDisabled] = text_muted;
    colors[ImGuiCol_WindowBg] = bg;
    colors[ImGuiCol_ChildBg] = bg;
    colors[ImGuiCol_PopupBg] = bg_light;
    colors[ImGuiCol_Border] = border;
    colors[ImGuiCol_BorderShadow] = ImVec4(0, 0, 0, 0);
    colors[ImGuiCol_FrameBg] = bg_dark;
    colors[ImGuiCol_FrameBgHovered] = border_muted;
    colors[ImGuiCol_FrameBgActive] = border;
    colors[ImGuiCol_TitleBg] = bg_dark;
    colors[ImGuiCol_TitleBgActive] = bg_dark;
    colors[ImGuiCol_TitleBgCollapsed] = bg_dark;
    colors[ImGuiCol_MenuBarBg] = bg;
    colors[ImGuiCol_ScrollbarBg] = bg_dark;
    colors[ImGuiCol_ScrollbarGrab] = border;
    colors[ImGuiCol_ScrollbarGrabHovered] = border_muted;
    colors[ImGuiCol_ScrollbarGrabActive] = text_muted;
    colors[ImGuiCol_CheckMark] = primary;
    colors[ImGuiCol_SliderGrab] = primary;
    colors[ImGuiCol_PlotHistogram] = primary;
    colors[ImGuiCol_SliderGrabActive] = primary;
    colors[ImGuiCol_Button] = bg_dark;
    colors[ImGuiCol_ButtonHovered] = border_muted;
    colors[ImGuiCol_ButtonActive] = border;
    colors[ImGuiCol_Header] = primary;
    colors[ImGuiCol_HeaderHovered] = primary;
    colors[ImGuiCol_HeaderActive] = primary;
    colors[ImGuiCol_Separator] = border;
    colors[ImGuiCol_SeparatorHovered] = border_muted;
    colors[ImGuiCol_SeparatorActive] = border;
    colors[ImGuiCol_ResizeGrip] = border;
    colors[ImGuiCol_ResizeGripHovered] = border_muted;
    colors[ImGuiCol_ResizeGripActive] = text;
    colors[ImGuiCol_Tab] = bg_dark;
    colors[ImGuiCol_TabHovered] = primary;
    colors[ImGuiCol_TabActive] = primary;
    colors[ImGuiCol_TabUnfocused] = bg_dark;
    colors[ImGuiCol_TabUnfocusedActive] = border;
    colors[ImGuiCol_DockingPreview] = primary;
    colors[ImGuiCol_DockingEmptyBg] = bg;
    colors[ImGuiCol_PlotLines] = secondary;
    colors[ImGuiCol_PlotLinesHovered] = danger;
    colors[ImGuiCol_TextSelectedBg] = primary;
    colors[ImGuiCol_DragDropTarget] = info;
    colors[ImGuiCol_NavHighlight] = primary;
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1, 1, 1, 0.7f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.8f, 0.8f, 0.8f, 0.2f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.8f, 0.8f, 0.8f, 0.35f);
    colors[ImGuiCol_TableHeaderBg]     = bg_dark;
    colors[ImGuiCol_TableBorderStrong] = border; 
    colors[ImGuiCol_TableBorderLight]  = border_muted;
  }

  inline void SetBessDarkColors() {
    ImGuiStyle &style = ImGui::GetStyle();
    ImVec4 *colors = style.Colors;

    // Primary background
    colors[ImGuiCol_WindowBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);  // #131318
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.12f, 0.12f, 0.15f, 1.00f); // #131318

    colors[ImGuiCol_PopupBg] = ImVec4(0.18f, 0.18f, 0.22f, 1.00f);

    // Headers
    colors[ImGuiCol_Header] = ImVec4(0.18f, 0.18f, 0.22f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.30f, 0.30f, 0.40f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.25f, 0.25f, 0.35f, 1.00f);

    // Buttons
    colors[ImGuiCol_Button] = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.30f, 0.32f, 0.40f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.35f, 0.38f, 0.50f, 1.00f);

    // Frame BG
    colors[ImGuiCol_FrameBg] = ImVec4(0.15f, 0.15f, 0.18f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.22f, 0.22f, 0.27f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.25f, 0.25f, 0.30f, 1.00f);

    // Tabs
    colors[ImGuiCol_Tab] = ImVec4(0.18f, 0.18f, 0.22f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.35f, 0.35f, 0.50f, 1.00f);
    colors[ImGuiCol_TabActive] = ImVec4(0.25f, 0.25f, 0.38f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.13f, 0.13f, 0.17f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.20f, 0.20f, 0.25f, 1.00f);

    // Title
    colors[ImGuiCol_TitleBg] = ImVec4(0.12f, 0.12f, 0.15f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.15f, 0.15f, 0.20f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);

    // Borders
    colors[ImGuiCol_Border] = ImVec4(0.20f, 0.20f, 0.25f, 0.50f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

    // Text
    colors[ImGuiCol_Text] = ImVec4(0.90f, 0.90f, 0.95f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.55f, 1.00f);

    // Highlights
    colors[ImGuiCol_CheckMark] = ImVec4(0.50f, 0.70f, 1.00f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.50f, 0.70f, 1.00f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.60f, 0.80f, 1.00f, 1.00f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.50f, 0.70f, 1.00f, 0.50f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.60f, 0.80f, 1.00f, 0.75f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.70f, 0.90f, 1.00f, 1.00f);

    // Scrollbar
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.30f, 0.30f, 0.35f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.40f, 0.50f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.45f, 0.45f, 0.55f, 1.00f);

    // Style tweaks
    style.WindowRounding = 5.0f;
    style.FrameRounding = 5.0f;
    style.GrabRounding = 5.0f;
    style.TabRounding = 5.0f;
    style.PopupRounding = 5.0f;
    style.ScrollbarRounding = 5.0f;
    style.WindowPadding = ImVec2(10, 10);
    style.FramePadding = ImVec2(6, 4);
    style.ItemSpacing = ImVec2(8, 6);
    style.PopupBorderSize = 0.f;
  }

  inline void SetFluentUIColors() {
    ImGuiStyle &style = ImGui::GetStyle();
    ImVec4 *colors = style.Colors;

    // General window settings
    style.WindowRounding = 5.0f;
    style.FrameRounding = 5.0f;
    style.ScrollbarRounding = 5.0f;
    style.GrabRounding = 5.0f;
    style.TabRounding = 5.0f;
    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;
    style.PopupBorderSize = 1.0f;
    style.PopupRounding = 5.0f;

    // Setting the colors
    colors[ImGuiCol_Text] = ImVec4(0.95f, 0.95f, 0.95f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.18f, 0.18f, 0.18f, 1.f);
    colors[ImGuiCol_Border] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);

    // Accent colors changed to darker olive-green/grey shades
    colors[ImGuiCol_CheckMark] = ImVec4(0.45f, 0.45f, 0.45f, 1.00f);        // Dark gray for check marks
    colors[ImGuiCol_SliderGrab] = ImVec4(0.45f, 0.45f, 0.45f, 1.00f);       // Dark gray for sliders
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f); // Slightly lighter gray when active
    colors[ImGuiCol_Button] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);           // Button background (dark gray)
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);    // Button hover state
    colors[ImGuiCol_ButtonActive] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);     // Button active state
    colors[ImGuiCol_Header] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);           // Dark gray for menu headers
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.45f, 0.45f, 0.45f, 1.00f);    // Slightly lighter on hover
    colors[ImGuiCol_HeaderActive] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);     // Lighter gray when active
    colors[ImGuiCol_Separator] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);        // Separators in dark gray
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.45f, 0.45f, 0.45f, 1.00f); // Resize grips in dark gray
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.55f, 0.55f, 0.55f, 1.00f);
    colors[ImGuiCol_Tab] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);        // Tabs background
    colors[ImGuiCol_TabHovered] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f); // Darker gray on hover
    colors[ImGuiCol_TabActive] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
    colors[ImGuiCol_DockingPreview] = ImVec4(0.45f, 0.45f, 0.45f, 1.00f); // Docking preview in gray
    colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f); // Empty dock background
    // Additional styles
    style.FramePadding = ImVec2(8.0f, 4.0f);
    style.ItemSpacing = ImVec2(8.0f, 4.0f);
    style.IndentSpacing = 20.0f;
    style.ScrollbarSize = 16.0f;
  }


  inline void SetFluentLight() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // Accent (Fluent blue #0078D4 and companions)
    const ImVec4 ACCENT        = ImVec4(0.00f, 0.47f, 0.83f, 1.00f); // #0078D4
    const ImVec4 ACCENT_HOVER  = ImVec4(0.06f, 0.43f, 0.73f, 1.00f); // #106EBE
    const ImVec4 ACCENT_ACTIVE = ImVec4(0.00f, 0.35f, 0.62f, 1.00f); // #005A9E

    // Shape & layout
    style.WindowRounding    = 5.0f;
    style.FrameRounding     = 5.0f;
    style.ScrollbarRounding = 5.0f;
    style.GrabRounding      = 5.0f;
    style.TabRounding       = 5.0f;

    style.WindowBorderSize  = 1.0f;
    style.FrameBorderSize   = 1.0f;
    style.PopupBorderSize   = 1.0f;
    style.PopupRounding     = 5.0f;

    style.FramePadding      = ImVec2(8.0f, 4.0f);
    style.ItemSpacing       = ImVec2(8.0f, 4.0f);
    style.IndentSpacing     = 20.0f;
    style.ScrollbarSize     = 16.0f;

    // Text
    colors[ImGuiCol_Text]           = ImVec4(0.10f, 0.10f, 0.10f, 1.00f); // Stronger contrast
    colors[ImGuiCol_TextDisabled]   = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);

    // Surfaces (slightly darker than pure white to avoid "blankness")
    colors[ImGuiCol_WindowBg]       = ImVec4(0.95f, 0.95f, 0.95f, 1.00f); // Page
    colors[ImGuiCol_ChildBg]        = ImVec4(0.99f, 0.99f, 0.99f, 1.00f); // Cards/panels
    colors[ImGuiCol_PopupBg]        = ImVec4(1.00f, 1.00f, 1.00f, 1.00f); // Popups

    // Structure & depth
    colors[ImGuiCol_Border]         = ImVec4(0.72f, 0.72f, 0.72f, 1.00f); // Crisper outlines
    colors[ImGuiCol_BorderShadow]   = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

    colors[ImGuiCol_FrameBg]        = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
    colors[ImGuiCol_FrameBgActive]  = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);

    colors[ImGuiCol_TitleBg]        = ImVec4(0.93f, 0.93f, 0.93f, 1.00f);
    colors[ImGuiCol_TitleBgActive]  = ImVec4(0.88f, 0.88f, 0.88f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]= ImVec4(0.95f, 0.95f, 0.95f, 1.00f);

    colors[ImGuiCol_MenuBarBg]      = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);

    // Scrollbars
    colors[ImGuiCol_ScrollbarBg]          = ImVec4(0.95f, 0.95f, 0.95f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab]        = ImVec4(0.78f, 0.78f, 0.78f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.70f, 0.70f, 0.70f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive]  = ImVec4(0.62f, 0.62f, 0.62f, 1.00f);

    // Primary accents (more pronounced)
    colors[ImGuiCol_CheckMark]      = ACCENT;
    colors[ImGuiCol_SliderGrab]     = ACCENT_HOVER;
    colors[ImGuiCol_SliderGrabActive]= ACCENT_ACTIVE;

    // Buttons: neutral base, accented hover/active with alpha (keeps text readable)
    colors[ImGuiCol_Button]         = ImVec4(0.93f, 0.93f, 0.93f, 1.00f);
    colors[ImGuiCol_ButtonHovered]  = ImVec4(ACCENT.x, ACCENT.y, ACCENT.z, 0.15f);
    colors[ImGuiCol_ButtonActive]   = ImVec4(ACCENT_ACTIVE.x, ACCENT_ACTIVE.y, ACCENT_ACTIVE.z, 0.25f);

    // Headers (Tree/Collapsing/Selectable) with clear accent presence
    colors[ImGuiCol_Header]         = ImVec4(ACCENT.x, ACCENT.y, ACCENT.z, 0.13f);
    colors[ImGuiCol_HeaderHovered]  = ImVec4(ACCENT.x, ACCENT.y, ACCENT.z, 0.20f);
    colors[ImGuiCol_HeaderActive]   = ImVec4(ACCENT_ACTIVE.x, ACCENT_ACTIVE.y, ACCENT_ACTIVE.z, 0.30f);

    // Separators: stronger for structure
    colors[ImGuiCol_Separator]         = ImVec4(0.55f, 0.55f, 0.55f, 1.00f);
    colors[ImGuiCol_SeparatorHovered]  = ImVec4(ACCENT.x, ACCENT.y, ACCENT.z, 0.55f);
    colors[ImGuiCol_SeparatorActive]   = ImVec4(ACCENT_ACTIVE.x, ACCENT_ACTIVE.y, ACCENT_ACTIVE.z, 0.70f);

    // Resize grips: neutral → accent on interaction
    colors[ImGuiCol_ResizeGrip]        = ImVec4(0.75f, 0.75f, 0.75f, 1.00f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(ACCENT.x, ACCENT.y, ACCENT.z, 0.40f);
    colors[ImGuiCol_ResizeGripActive]  = ImVec4(ACCENT_ACTIVE.x, ACCENT_ACTIVE.y, ACCENT_ACTIVE.z, 0.60f);

    // Tabs: clear active state with accent tint
    colors[ImGuiCol_Tab]                = ImVec4(0.92f, 0.92f, 0.92f, 1.00f);
    colors[ImGuiCol_TabHovered]         = ImVec4(ACCENT.x, ACCENT.y, ACCENT.z, 0.22f);
    colors[ImGuiCol_TabActive]          = ImVec4(ACCENT.x, ACCENT.y, ACCENT.z, 0.30f);
    colors[ImGuiCol_TabUnfocused]       = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(ACCENT.x, ACCENT.y, ACCENT.z, 0.22f);

    // Docking & focus
    colors[ImGuiCol_DockingPreview]     = ImVec4(ACCENT.x, ACCENT.y, ACCENT.z, 0.45f);
    colors[ImGuiCol_DockingEmptyBg]     = ImVec4(0.96f, 0.96f, 0.96f, 1.00f);
    colors[ImGuiCol_NavHighlight]       = ImVec4(ACCENT.x, ACCENT.y, ACCENT.z, 0.40f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(ACCENT.x, ACCENT.y, ACCENT.z, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg]  = ImVec4(0.20f, 0.20f, 0.20f, 0.12f);
    colors[ImGuiCol_DragDropTarget]     = ImVec4(ACCENT.x, ACCENT.y, ACCENT.z, 0.80f);

    // Tables (if you use them)
    colors[ImGuiCol_TableHeaderBg]      = ImVec4(0.92f, 0.92f, 0.92f, 1.00f);
    colors[ImGuiCol_TableBorderStrong]  = ImVec4(0.66f, 0.66f, 0.66f, 1.00f);
    colors[ImGuiCol_TableBorderLight]   = ImVec4(0.83f, 0.83f, 0.83f, 1.00f);
    colors[ImGuiCol_TableRowBg]         = ImVec4(1.00f, 1.00f, 1.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt]      = ImVec4(0.00f, 0.00f, 0.00f, 0.03f);

    // Plots (subtle accent)
    colors[ImGuiCol_PlotLines]          = ImVec4(ACCENT.x, ACCENT.y, ACCENT.z, 0.90f);
    colors[ImGuiCol_PlotLinesHovered]   = ImVec4(ACCENT_ACTIVE.x, ACCENT_ACTIVE.y, ACCENT_ACTIVE.z, 1.00f);
    colors[ImGuiCol_PlotHistogram]      = ImVec4(ACCENT.x, ACCENT.y, ACCENT.z, 0.90f);
    colors[ImGuiCol_PlotHistogramHovered]= ImVec4(ACCENT_ACTIVE.x, ACCENT_ACTIVE.y, ACCENT_ACTIVE.z, 1.00f);
  }


  inline void SetCatpuccinMochaColors() {
    ImGuiStyle &style = ImGui::GetStyle();
    ImVec4 *colors = style.Colors;

    // Base colors inspired by Catppuccin Mocha
    colors[ImGuiCol_Text] = ImVec4(0.90f, 0.89f, 0.88f, 1.00f);         // Latte
    colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.56f, 0.52f, 1.00f); // Surface2
    colors[ImGuiCol_WindowBg] = ImVec4(0.17f, 0.14f, 0.20f, 1.00f);     // Base
    colors[ImGuiCol_ChildBg] = ImVec4(0.18f, 0.16f, 0.22f, 1.00f);      // Mantle
    colors[ImGuiCol_PopupBg] = ImVec4(0.17f, 0.14f, 0.20f, 1.00f);      // Base
    colors[ImGuiCol_Border] = ImVec4(0.27f, 0.23f, 0.29f, 1.00f);       // Overlay0
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.21f, 0.18f, 0.25f, 1.00f);              // Crust
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.24f, 0.20f, 0.29f, 1.00f);       // Overlay1
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.26f, 0.22f, 0.31f, 1.00f);        // Overlay2
    colors[ImGuiCol_TitleBg] = ImVec4(0.14f, 0.12f, 0.18f, 1.00f);              // Mantle
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.17f, 0.15f, 0.21f, 1.00f);        // Mantle
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.14f, 0.12f, 0.18f, 1.00f);     // Mantle
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.17f, 0.15f, 0.22f, 1.00f);            // Base
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.17f, 0.14f, 0.20f, 1.00f);          // Base
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.21f, 0.18f, 0.25f, 1.00f);        // Crust
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.24f, 0.20f, 0.29f, 1.00f); // Overlay1
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.26f, 0.22f, 0.31f, 1.00f);  // Overlay2
    colors[ImGuiCol_CheckMark] = ImVec4(0.95f, 0.66f, 0.47f, 1.00f);            // Peach
    colors[ImGuiCol_SliderGrab] = ImVec4(0.82f, 0.61f, 0.85f, 1.00f);           // Lavender
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.89f, 0.54f, 0.79f, 1.00f);     // Pink
    colors[ImGuiCol_Button] = ImVec4(0.65f, 0.34f, 0.46f, 1.00f);               // Maroon
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.71f, 0.40f, 0.52f, 1.00f);        // Red
    colors[ImGuiCol_ButtonActive] = ImVec4(0.76f, 0.46f, 0.58f, 1.00f);         // Pink
    colors[ImGuiCol_Header] = ImVec4(0.65f, 0.34f, 0.46f, 1.00f);               // Maroon
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.71f, 0.40f, 0.52f, 1.00f);        // Red
    colors[ImGuiCol_HeaderActive] = ImVec4(0.76f, 0.46f, 0.58f, 1.00f);         // Pink
    colors[ImGuiCol_Separator] = ImVec4(0.27f, 0.23f, 0.29f, 1.00f);            // Overlay0
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.95f, 0.66f, 0.47f, 1.00f);     // Peach
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.95f, 0.66f, 0.47f, 1.00f);      // Peach
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.82f, 0.61f, 0.85f, 1.00f);           // Lavender
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.89f, 0.54f, 0.79f, 1.00f);    // Pink
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.92f, 0.61f, 0.85f, 1.00f);     // Mauve
    colors[ImGuiCol_Tab] = ImVec4(0.21f, 0.18f, 0.25f, 1.00f);                  // Crust
    colors[ImGuiCol_TabHovered] = ImVec4(0.82f, 0.61f, 0.85f, 1.00f);           // Lavender
    colors[ImGuiCol_TabActive] = ImVec4(0.76f, 0.46f, 0.58f, 1.00f);            // Pink
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.18f, 0.16f, 0.22f, 1.00f);         // Mantle
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.21f, 0.18f, 0.25f, 1.00f);   // Crust
    colors[ImGuiCol_DockingPreview] = ImVec4(0.95f, 0.66f, 0.47f, 0.70f);       // Peach
    colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);       // Base
    colors[ImGuiCol_PlotLines] = ImVec4(0.82f, 0.61f, 0.85f, 1.00f);            // Lavender
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.89f, 0.54f, 0.79f, 1.00f);     // Pink
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.82f, 0.61f, 0.85f, 1.00f);        // Lavender
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.89f, 0.54f, 0.79f, 1.00f); // Pink
    colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);        // Mantle
    colors[ImGuiCol_TableBorderStrong] = ImVec4(0.27f, 0.23f, 0.29f, 1.00f);    // Overlay0
    colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);     // Surface2
    colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);  // Surface0
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.82f, 0.61f, 0.85f, 0.35f); // Lavender
    colors[ImGuiCol_DragDropTarget] = ImVec4(0.95f, 0.66f, 0.47f, 0.90f); // Peach
    colors[ImGuiCol_NavHighlight] = ImVec4(0.82f, 0.61f, 0.85f, 1.00f);   // Lavender
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

    // Style adjustments
    style.WindowRounding = 6.0f;
    style.FrameRounding = 4.0f;
    style.ScrollbarRounding = 4.0f;
    style.GrabRounding = 3.0f;
    style.ChildRounding = 4.0f;

    style.WindowTitleAlign = ImVec2(0.50f, 0.50f);
    style.WindowPadding = ImVec2(8.0f, 8.0f);
    style.FramePadding = ImVec2(5.0f, 4.0f);
    style.ItemSpacing = ImVec2(6.0f, 6.0f);
    style.ItemInnerSpacing = ImVec2(6.0f, 6.0f);
    style.IndentSpacing = 22.0f;

    style.ScrollbarSize = 14.0f;
    style.GrabMinSize = 10.0f;

    style.AntiAliasedLines = true;
    style.AntiAliasedFill = true;
  }


} // end Themes namespace
