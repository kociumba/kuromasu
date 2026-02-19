#ifndef COMMON_H
#define COMMON_H

#include <grid.h>
#include <imgui.h>
#include <raylib.h>
#include <unordered_map>

struct cell {
    enum type_t {
        blank,
        black,
        white,
    } type = blank;

    int observer_value = -1;
    bool observer_satisfied = false;
    bool mistake = false;

    float mistake_alpha = 0.0f;
    float mistake_delay = 1.0f;
};

struct cell_change {
    ktl::pos2_size pos;
    cell::type_t old_c;
    cell::type_t new_c;
};

struct action {
    std::vector<cell_change> changes;
};

constexpr Vector2 grid_size = {9, 9};

struct state_t {
    ktl::grid<cell> game = ktl::grid<cell>(grid_size.x,
        grid_size.y,
        cell{.type = cell::blank, .observer_value = -1},
        ktl::GRID_GROW_OUTWARD | ktl::GRID_NO_RETAIN_STATE);

    uint32_t seed = 0;

    bool solved = false;
    bool auto_surround = false;

    ktl::grid<cell> solved_state = ktl::grid<cell>(grid_size.x,
        grid_size.y,
        cell{.type = cell::blank, .observer_value = -1},
        ktl::GRID_GROW_OUTWARD | ktl::GRID_NO_RETAIN_STATE);
    ktl::grid<cell> starting_pos = ktl::grid<cell>(grid_size.x,
        grid_size.y,
        cell{.type = cell::blank, .observer_value = -1},
        ktl::GRID_GROW_OUTWARD | ktl::GRID_NO_RETAIN_STATE);

    Vector2 offset;
    float cell_size;

    std::unordered_map<int, Font> fonts;
    Texture2D win_image;

#if defined(__ANDROID__)
    bool horizontal_layout = false;
#else
    bool horizontal_layout = true;
#endif
    bool needs_dock_rebuild = true;

    bool custom_cursor = false;
    Texture2D cursor;

    std::vector<action> undo_stack;
    std::vector<action> redo_stack;

    struct {
        ktl::pos2_size start = ktl::pos2_size::invalid();
        Rectangle rect = {-1, -1, -1, -1};
        Rectangle dims = {0, 0, 0, 0};
    } measure;

    struct {
        ktl::pos2_size start = ktl::pos2_size::invalid();
        Rectangle rect = {-1, -1, -1, -1};
        Rectangle dims = {0, 0, 0, 0};
    } erase;

    struct {
        ktl::pos2_size start = ktl::pos2_size::invalid();
        action drag_action;
    } white_fill;
};

extern state_t state;

Font& get_font(state_t& state, int size, const char* path = ASSET_DIR "Roboto-Regular.ttf");

#endif /* COMMON_H */
