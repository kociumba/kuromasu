#include "input.h"
#include "kuromasu.h"
#include "rendering.h"

static ImVec2 get_mouse_position() {
    float mx, my;
    SDL_GetMouseState(&mx, &my);
    return {mx, my};
}

ImVec2 get_relative_mouse(const state_t& s, ImVec2 window_origin) {
    ImVec2 mouse_pos = get_mouse_position();
    ImVec2 relative;

    relative.x = mouse_pos.x - window_origin.x - s.offset.x;
    relative.y = mouse_pos.y - window_origin.y - s.offset.y;

    return relative;
}

ktl::pos2_size get_cell_under_mouse(const state_t& s, ImVec2 window_origin) {
    auto relative = get_relative_mouse(s, window_origin);

    if (relative.x >= 0 && relative.y >= 0 && relative.x < (s.game.width * s.cell_size) &&
        relative.y < (s.game.height * s.cell_size)) {
        size_t gx = static_cast<size_t>(relative.x / s.cell_size);
        size_t gy = static_cast<size_t>(relative.y / s.cell_size);

        return {gx, gy};
    }

    return ktl::pos2_size::invalid();
}

void undo_action(state_t& s) {
    if (!s.undo_stack.empty()) {
        auto a = s.undo_stack.back();
        for (auto& change : a.changes) {
            auto& c = s.game.at(change.pos);
            if (c.type == change.new_c) { c.type = change.old_c; }
        }

        s.redo_stack.push_back(a);
        s.undo_stack.pop_back();
    }
}

void redo_action(state_t& s) {
    if (!s.redo_stack.empty()) {
        auto a = s.redo_stack.back();
        for (auto& change : a.changes) {
            auto& c = s.game.at(change.pos);
            if (c.type == change.old_c) { c.type = change.new_c; }
        }

        s.undo_stack.push_back(a);
        s.redo_stack.pop_back();
    }
}

bool is_ctrl_down() {
    const bool* keys = SDL_GetKeyboardState(nullptr);
    return keys[SDL_SCANCODE_LCTRL] || keys[SDL_SCANCODE_RCTRL];
}
bool is_shift_down() {
    const bool* keys = SDL_GetKeyboardState(nullptr);
    return keys[SDL_SCANCODE_LSHIFT] || keys[SDL_SCANCODE_RSHIFT];
}

void mouse_input(state_t& s, ImVec2 window_origin) {
    //ImGuiIO& io = ImGui::GetIO();
    //if (io.WantCaptureMouse) { return; }

    ktl::pos2_size click = get_cell_under_mouse(s, window_origin);

    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        if (is_ctrl_down()) {
            s.erase.start = get_cell_under_mouse(s, window_origin);
        } else {
            s.white_fill.start = click;
            if (click != ktl::pos2_size::invalid()) {
                auto& c = s.game.at(click);
                if (c.observer_value == -1) {
                    cell::type_t next_t;
                    switch (c.type) {
                        case cell::blank:
                            next_t = cell::white;
                            break;
                        case cell::black:
                            next_t = cell::blank;
                            break;
                        case cell::white:
                            next_t = cell::black;
                            break;
                    }

                    s.white_fill.drag_action.changes.push_back({click, c.type, next_t});
                    c.type = next_t;
                    if (next_t == cell::black && s.auto_surround) {
                        s.game.orthogonal_neighbors(click, [&](cell& c, ktl::pos2_size p) -> bool {
                            if (c.type == cell::blank) {
                                s.white_fill.drag_action.changes.push_back(
                                    {p, c.type, cell::white});
                                c.type = cell::white;
                            }
                            return true;
                        });
                    }
                    solve(s);
                }
            }
        }
    }

    if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
        if (is_ctrl_down()) {
            s.erase.dims = grid_rect_from_corners(s.erase.start, click);
            s.erase.rect = grid_region_rect(s, s.erase.dims);
        } else {
            if (click != s.white_fill.start && click != ktl::pos2_size::invalid()) {
                if (click.x != ktl::pos2_size::invalid_value) {
                    auto& c = s.game.at(click);
                    if (c.type == cell::blank) {
                        s.white_fill.drag_action.changes.push_back({click, c.type, cell::white});
                        c.type = cell::white;
                        solve(s);
                    }
                }
            }
        }
    }

    if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
        if (is_ctrl_down()) {
            for (auto [c, pos] : s.game.items()) {
                if (is_pos_in_rect(pos, s.erase.dims) && c.observer_value == -1) {
                    s.white_fill.drag_action.changes.push_back({pos, c.type, cell::blank});
                    c.type = cell::blank;
                }
            }
        }

        if (!s.white_fill.drag_action.changes.empty()) {
            s.undo_stack.push_back(s.white_fill.drag_action);
            s.redo_stack.clear();
            s.white_fill.drag_action.changes.clear();
        }
        s.white_fill.start = ktl::pos2_size::invalid();

        s.erase.start = ktl::pos2_size::invalid();
        s.erase.rect = {-1, -1, -1, -1};
        solve(s);
    }

    if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
        s.erase.start = ktl::pos2_size::invalid();
        s.erase.rect = {-1, -1, -1, -1};
    }

    if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
        s.measure.start = get_cell_under_mouse(s, window_origin);
    }

    if (ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
        s.measure.dims = grid_rect_from_corners(s.measure.start, click);
        s.measure.rect = grid_region_rect(s, s.measure.dims);
    }

    if (ImGui::IsMouseReleased(ImGuiMouseButton_Right)) {
        s.measure.start = ktl::pos2_size::invalid();
        s.measure.rect = {-1, -1, -1, -1};
    }
}

void keyboard_input(state_t& s) {
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureKeyboard) { return; }

    if (is_ctrl_down() && ImGui::IsKeyPressed(ImGuiKey_Z)) {
        if (is_shift_down()) {
            redo_action(s);
        } else {
            undo_action(s);
        }
        solve(s);
    }

    if (ImGui::IsKeyPressed(ImGuiKey_N)) {
        if (is_ctrl_down()) {
            s.seed = generate_board(s);
            s.redo_stack.clear();
            s.undo_stack.clear();
            s.solved = false;
        }
    }

    if (ImGui::IsKeyPressed(ImGuiKey_A)) { s.auto_surround = !s.auto_surround; }
}
