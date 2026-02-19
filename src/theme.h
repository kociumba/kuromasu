#ifndef THEME_H
#define THEME_H

#include <imgui.h>

namespace ImGui {

inline void theme() {
    StyleColorsDark();
    auto& style = GetStyle();
    auto& io = GetIO();
    // if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    //     // style.WindowRounding = 0.0f;
    //     // style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    // }

    ImVec4* colors = style.Colors;

    ImVec4 green_primary = ImVec4(119.0f / 255.0f, 171.0f / 255.0f, 28.0f / 255.0f, 1.0f);
    ImVec4 green_hover =
        ImVec4(139.0f / 255.0f, 191.0f / 255.0f, 48.0f / 255.0f, 1.0f);  // Lighter green
    ImVec4 green_active =
        ImVec4(99.0f / 255.0f, 151.0f / 255.0f, 8.0f / 255.0f, 1.0f);  // Darker green
    ImVec4 green_tint = ImVec4(119.0f / 255.0f, 171.0f / 255.0f, 28.0f / 255.0f, 0.4f);

    // clang-format off
    colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.04f, 0.04f, 0.04f, 0.94f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
    colors[ImGuiCol_Border] = ImVec4(green_primary.x * 0.6f, green_primary.y * 0.6f, green_primary.z * 0.6f, 0.50f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.15f, 0.15f, 0.15f, 0.54f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(green_hover.x, green_hover.y, green_hover.z, 0.40f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(green_primary.x, green_primary.y, green_primary.z, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(green_primary.x * 0.8f, green_primary.y * 0.8f, green_primary.z * 0.8f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(green_primary.x * 0.6f, green_primary.y * 0.6f, green_primary.z * 0.6f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(green_hover.x * 0.7f, green_hover.y * 0.7f, green_hover.z * 0.7f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(green_active.x, green_active.y, green_active.z, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(green_primary.x, green_primary.y, green_primary.z, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(green_primary.x, green_primary.y, green_primary.z, 0.40f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(green_hover.x, green_hover.y, green_hover.z, 0.52f);
    colors[ImGuiCol_Button] = ImVec4(0.20f, 0.20f, 0.20f, 0.40f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(green_hover.x * 0.2f, green_hover.y * 0.2f, green_hover.z * 0.2f, 0.04f);
    colors[ImGuiCol_ButtonActive] = ImVec4(green_active.x, green_active.y, green_active.z, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(1.00f, 1.00f, 1.00f, 0.04f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(green_tint.x * 0.5f, green_tint.y * 0.5f, green_tint.z * 0.5f, 0.80f);
    colors[ImGuiCol_HeaderActive] = ImVec4(green_primary.x * 0.3f, green_primary.y * 0.3f, green_primary.z * 0.3f, 0.16f);
    colors[ImGuiCol_Separator] = ImVec4(green_primary.x * 0.6f, green_primary.y * 0.6f, green_primary.z * 0.6f, 0.50f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(green_hover.x, green_hover.y, green_hover.z, 0.78f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(green_primary.x, green_primary.y, green_primary.z, 1.00f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 1.00f, 1.00f, 0.04f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.00f, 1.00f, 1.00f, 0.13f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(green_active.x, green_active.y, green_active.z, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(green_hover.x, green_hover.y, green_hover.z, 0.50f);
    colors[ImGuiCol_Tab] = ImVec4(green_primary.x * 0.3f, green_primary.y * 0.3f, green_primary.z * 0.3f, 0.73f);
    colors[ImGuiCol_TabSelected] = ImVec4(green_primary.x, green_primary.y, green_primary.z, 1.00f);
    colors[ImGuiCol_TabSelectedOverline] = ImVec4(green_hover.x, green_hover.y, green_hover.z, 1.00f);
    colors[ImGuiCol_TabDimmed] = ImVec4(green_primary.x * 0.2f, green_primary.y * 0.2f, green_primary.z * 0.2f, 0.97f);
    colors[ImGuiCol_TabDimmedSelected] = ImVec4(green_primary.x * 0.7f, green_primary.y * 0.7f, green_primary.z * 0.7f, 1.00f);
    colors[ImGuiCol_TabDimmedSelectedOverline] = ImVec4(green_primary.x * 0.5f, green_primary.y * 0.5f, green_primary.z * 0.5f, 0.00f);
    colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(green_primary.x, green_primary.y, green_primary.z, 1.00f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(green_primary.x, green_primary.y, green_primary.z, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(green_hover.x, green_hover.y, green_hover.z, 1.00f);
    colors[ImGuiCol_TableHeaderBg] = ImVec4(green_primary.x * 0.2f, green_primary.y * 0.2f, green_primary.z * 0.2f, 1.00f);
    colors[ImGuiCol_TableBorderStrong] = ImVec4(green_primary.x * 0.5f, green_primary.y * 0.5f, green_primary.z * 0.5f, 1.00f);
    colors[ImGuiCol_TableBorderLight] = ImVec4(green_primary.x * 0.3f, green_primary.y * 0.3f, green_primary.z * 0.3f, 1.00f);
    colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt] = ImVec4(green_primary.x * 0.1f, green_primary.y * 0.1f, green_primary.z * 0.1f, 0.06f);
    colors[ImGuiCol_TextLink] = ImVec4(green_primary.x, green_primary.y, green_primary.z, 1.00f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(green_tint.x * 0.5f, green_tint.y * 0.5f, green_tint.z * 0.5f, 0.04f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(green_primary.x, green_primary.y, green_primary.z, 0.90f);
    colors[ImGuiCol_NavCursor] = ImVec4(green_primary.x, green_primary.y, green_primary.z, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(green_primary.x * 0.5f, green_primary.y * 0.5f, green_primary.z * 0.5f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
    // clang-format on

    style.WindowPadding = ImVec2(8.00f, 8.00f);
    style.FramePadding = ImVec2(5.00f, 2.00f);
    style.CellPadding = ImVec2(6.00f, 6.00f);
    style.ItemSpacing = ImVec2(6.00f, 6.00f);
    style.ItemInnerSpacing = ImVec2(6.00f, 6.00f);
    style.TouchExtraPadding = ImVec2(0.00f, 0.00f);
    style.IndentSpacing = 25;
    style.ScrollbarSize = 11;
    style.GrabMinSize = 10;
    style.WindowBorderSize = 1;
    style.ChildBorderSize = 1;
    style.PopupBorderSize = 1;
    style.FrameBorderSize = 1;
    style.TabBorderSize = 1;
    style.WindowRounding = 10;
    style.ChildRounding = 4;
    style.FrameRounding = 3;
    style.PopupRounding = 4;
    style.ScrollbarRounding = 9;
    style.GrabRounding = 3;
    style.LogSliderDeadzone = 4;
    style.TabRounding = 4;
}

}  // namespace ImGui

#endif /* THEME_H */
