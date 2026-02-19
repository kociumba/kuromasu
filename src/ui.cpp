#include "ui.h"
#include <extras/IconsFontAwesome6.h>
#include "input.h"
#include "kuromasu.h"
#include "rendering.h"

static const char* info_text =
    R"(The board starts with blank cells and some "Observer" white cells
"Observer" (white cell with a number) must see exactly its
number of white cells in four cardinal directions.
Black cells block an "Observer's" view.
Black cells can not touch.
All white cells including observers must stay connected)";

#if defined(__ANDROID__)
static float info_offset = 60.0f;
#else
static float info_offset = 20.0f;
#endif

bool confirm_popup(const char* title, const char* message, bool* open) {
    if (*open) {
        ImGui::OpenPopup(title);
        *open = false;
    }

    bool confirmed = false;

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

    float minWidth = std::min(300.0f, ImGui::GetMainViewport()->Size.x * 0.85f);
    ImGui::SetNextWindowSizeConstraints(ImVec2(scale_to_DPIF(minWidth), 0),
        ImVec2(ImGui::GetMainViewport()->Size.x * 0.85f, FLT_MAX));

    if (ImGui::BeginPopupModal(
            title, nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_Popup)) {
        ImGui::TextWrapped("%s", message);
        ImGui::Separator();
        ImGui::Spacing();

        float buttonWidth = scale_to_DPIF(80.0f);
        float spacing = ImGui::GetStyle().ItemSpacing.x;
        float totalWidth = buttonWidth * 2 + spacing;
        ImGui::SetCursorPosX((ImGui::GetWindowWidth() - totalWidth) * 0.5f);

        if (ImGui::Button("Confirm", ImVec2(buttonWidth, 0))) {
            confirmed = true;
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::IsKeyPressed(ImGuiKey_Enter)) {
            confirmed = true;
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel", ImVec2(buttonWidth, 0))) { ImGui::CloseCurrentPopup(); }
        if (ImGui::IsKeyPressed(ImGuiKey_Escape)) { ImGui::CloseCurrentPopup(); }

        ImGui::EndPopup();
    }

    return confirmed;
}

void render_section_header(const char* label, const char* icon = nullptr) {
    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
    if (icon) {
        ImGui::TextUnformatted(icon);
        ImGui::SameLine(0, 6.0f);
    }
    ImGui::TextUnformatted(label);
    ImGui::PopStyleColor();
    ImGui::Separator();
}

void board_controls(state_t& state) {
    ImGui::Text("Kuromasu");

    ImGui::SameLine(ImGui::GetContentRegionAvail().x - info_offset);

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));

    if (ImGui::Button(ICON_FA_CIRCLE_INFO "##Info")) { ImGui::OpenPopup("Kuromasu Info"); }

    ImGui::PopStyleColor(4);

    if (ImGui::BeginPopup("Kuromasu Info")) {
        ImGui::Text("Kuromasu puzzles");
        ImGui::Separator();
        ImGui::PushTextWrapPos(GetScreenWidth());
        ImGui::TextUnformatted(info_text);
        ImGui::PopTextWrapPos();
        // ImGui::Separator();
        ImGui::EndPopup();
    }

    ImGui::Separator();
    ImGui::Spacing();

    // ==================== EDIT CONTROLS ====================
    bool can_undo = !state.undo_stack.empty();
    bool can_redo = !state.redo_stack.empty();

    float hamburger_width = ImGui::GetFrameHeight() + ImGui::GetStyle().ItemSpacing.x;
    float button_width =
        (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x - hamburger_width) *
        0.5f;

    if (!can_undo) ImGui::BeginDisabled();
    if (ImGui::Button(ICON_FA_ROTATE_LEFT "Undo", ImVec2(button_width, 0))) {
        undo_action(state);
        solve(state);
    }
    if (!can_undo) ImGui::EndDisabled();

    ImGui::SameLine();

    if (!can_redo) ImGui::BeginDisabled();
    if (ImGui::Button(ICON_FA_ROTATE_RIGHT "Redo", ImVec2(button_width, 0))) {
        redo_action(state);
        solve(state);
    }
    if (!can_redo) ImGui::EndDisabled();

    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
    if (ImGui::Button(ICON_FA_BARS "##edit_menu", ImVec2(ImGui::GetFrameHeight(), 0))) {
        ImGui::OpenPopup("##edit_popup");
    }
    ImGui::PopStyleColor(3);

    static bool clear_popup = false;

    if (ImGui::BeginPopup("##edit_popup")) {
        if (ImGui::MenuItem(ICON_FA_TRASH "Clear Board")) { clear_popup = true; }
        // if (ImGui::MenuItem(ICON_FA_FILE_EXPORT "  Export")) {
        // }

        ImGui::EndPopup();
    }

    if (confirm_popup("Reset current board ?", "Are you sure ?", &clear_popup)) {
        state.game = state.starting_pos;
    }

    ImGui::BeginChild("##scrollable_controls", ImVec2(0, 0), false, ImGuiWindowFlags_None);

    // ==================== GENERATION CONTROLS ====================
    render_section_header("Generation");

    // Seed controls
    static uint32_t ui_seed = state.seed;
    static bool seed_modified = false;
    static bool seed_frozen = false;

    // keep ui_seed synced to state.seed unless the user modified it or it's frozen
    if (!seed_modified && !seed_frozen) { ui_seed = state.seed; }

    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted("Seed");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x -
                            (ImGui::GetFrameHeight() + ImGui::GetStyle().ItemSpacing.x) * 2);

    const char* seed_label = seed_modified ? "##seed_input*" : "##seed_input";
    if (ImGui::InputScalar(seed_label, ImGuiDataType_U32, &ui_seed)) {
        seed_modified = (ui_seed != state.seed);
    }

    if (ImGui::IsItemHovered()) {
        if (seed_modified) {
            ImGui::SetTooltip(
                "Seed modified: Generate will use the entered seed and will not randomize.");
        }
    }

    ImGui::SameLine();
    if (!seed_modified && !seed_frozen) ImGui::BeginDisabled();
    if (ImGui::Button(ICON_FA_ARROWS_ROTATE "##reset_seed", ImVec2(ImGui::GetFrameHeight(), 0))) {
        ui_seed = state.seed;
        seed_modified = false;
    }
    if (ImGui::IsItemHovered()) { ImGui::SetTooltip("Reset Seed"); }
    if (!seed_modified && !seed_frozen) ImGui::EndDisabled();

    ImGui::SameLine();
    if (ImGui::Button(seed_frozen ? ICON_FA_LOCK "##freeze" : ICON_FA_LOCK_OPEN "##freeze",
            ImVec2(ImGui::GetFrameHeight(), 0))) {
        seed_frozen = !seed_frozen;
    }
    if (ImGui::IsItemHovered()) { ImGui::SetTooltip(seed_frozen ? "Unlock Seed" : "Lock Seed"); }

    ImGui::Spacing();

    // Generation chances (percent)
    static float ui_black_chance = 50.0f;
    static float ui_observer_chance = 50.0f;

    ImGui::SetNextItemWidth(-1);
    if (ImGui::SliderFloat("##black_chance", &ui_black_chance, 0.0f, 100.0f, "Black: %.0f%%")) {
        if (ui_black_chance < 0.0f) ui_black_chance = 0.0f;
        if (ui_black_chance > 100.0f) ui_black_chance = 100.0f;
    }

    ImGui::SetNextItemWidth(-1);
    if (ImGui::SliderFloat(
            "##observer_chance", &ui_observer_chance, 0.0f, 100.0f, "Observer: %.0f%%")) {
        if (ui_observer_chance < 0.0f) ui_observer_chance = 0.0f;
        if (ui_observer_chance > 100.0f) ui_observer_chance = 100.0f;
    }

    // Board size controls (preserve user edits across frames)
    static int ui_width = (int)state.game.width;
    static int ui_height = (int)state.game.height;
    // Track modifications per-dimension so each can be reset independently
    static bool width_modified = false;
    static bool height_modified = false;

    // If the user hasn't touched the inputs, keep them in sync with the current board
    if (!width_modified && !height_modified) {
        if (ui_width != (int)state.game.width || ui_height != (int)state.game.height) {
            ui_width = (int)state.game.width;
            ui_height = (int)state.game.height;
        }
    }

    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * .5f);
    bool width_changed = ImGui::InputInt("Width##board_width", &ui_width, 1, 10);
    if (width_changed) width_modified = (ui_width != (int)state.game.width);

    ImGui::SameLine();

    if (!width_modified) ImGui::BeginDisabled();
    if (ImGui::Button(ICON_FA_ARROWS_ROTATE "##reset_width", ImVec2(ImGui::GetFrameHeight(), 0))) {
        ui_width = (int)state.game.width;
        width_modified = false;
    }
    if (ImGui::IsItemHovered()) { ImGui::SetTooltip("Reset Width"); }
    if (!width_modified) ImGui::EndDisabled();

    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * .5f);
    bool height_changed = ImGui::InputInt("Height##board_height", &ui_height, 1, 10);
    if (height_changed) height_modified = (ui_height != (int)state.game.height);

    ImGui::SameLine();

    if (!height_modified) ImGui::BeginDisabled();
    if (ImGui::Button(ICON_FA_ARROWS_ROTATE "##reset_height", ImVec2(ImGui::GetFrameHeight(), 0))) {
        ui_height = (int)state.game.height;
        height_modified = false;
    }
    if (ImGui::IsItemHovered()) { ImGui::SetTooltip("Reset Height"); }
    if (!height_modified) ImGui::EndDisabled();

    int width = ui_width;
    int height = ui_height;
    if (width < 3) width = 3;
    if (height < 3) height = 3;

    ImGui::Spacing();
    if (ImGui::Button("Generate", ImVec2(-1, 0))) {
        std::optional<uint32_t> s;
        if (seed_frozen || seed_modified) {
            s = ui_seed;
        } else {
            s = std::nullopt;
        }

        // If board size was modified (width or height), resize before generating
        if (width_modified || height_modified) {
            state.game.resize(
                (size_t)width, (size_t)height, cell{.type = cell::blank, .observer_value = -1});
            state.solved_state.resize(
                (size_t)width, (size_t)height, cell{.type = cell::blank, .observer_value = -1});
            width_modified = false;
            height_modified = false;
        }

        state.seed = generate_board(state, s, ui_black_chance, ui_observer_chance);
        state.redo_stack.clear();
        state.undo_stack.clear();
        state.solved = false;

        // After generation, if we randomized, keep the ui in sync; if user had modified, keep that state
        if (!seed_modified && !seed_frozen) { ui_seed = state.seed; }
        seed_modified = false;
    }

    ImGui::Spacing();
    ImGui::Spacing();

    // ==================== OPTIONS ====================
    render_section_header("Options");

    if (ImGui::Checkbox("Horizontal layout", &state.horizontal_layout)) {
        state.needs_dock_rebuild = true;
    }
    ImGui::Checkbox("Auto Surround", &state.auto_surround);
    ImGui::Checkbox("Custom Cursor", &state.custom_cursor);

    ImGui::Spacing();

    ImGui::Dummy(ImVec2(0.0f, ImGui::GetWindowHeight() * 0.4f));  // for mobile

    ImGui::EndChild();
}
