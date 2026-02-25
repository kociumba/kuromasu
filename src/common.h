#ifndef COMMON_H
#define COMMON_H

#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#define KTL_ARENA_BACKEND 1
#include <arena.h>
#include <grid.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>
#include <unordered_map>

#ifdef TRACY_ENABLE
#include "tracy/Tracy.hpp"

#define frame_mark() FrameMark
#define zone_scoped_n(name) ZoneScopedN(name)
#define zone_text(fmt, ...) ZoneTextF(fmt, ##__VA_ARGS__)

#define zone_color(color) ZoneColor(color)
#define zone_scoped_nc(name, color) ZoneScopedNC(name, color)

#define PROF_COLOR_RED 0xFF0000
#define PROF_COLOR_GREEN 0x00FF00
#define PROF_COLOR_BLUE 0x0000FF
#define PROF_COLOR_YELLOW 0xFFFF00
#define PROF_COLOR_MAGENTA 0xFF00FF
#define PROF_COLOR_CYAN 0x00FFFF
#define PROF_COLOR_WHITE 0xFFFFFF

#else

#define frame_mark()
#define zone_scoped_n(name)
#define zone_text(fmt, ...)
#define zone_color(color)
#define zone_scoped_nc(name, color)

#endif

#ifndef BUILD_IDENTIFIER
#define BUILD_IDENTIFIER "unknown-dev"
#endif

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

constexpr ImVec2 grid_size = {9, 9};

struct Texture {
    SDL_Texture* tex = nullptr;
    float w = 0, h = 0;

    void free() {
        if (tex) SDL_DestroyTexture(tex);
        tex = nullptr;
    }

    void resize(SDL_Renderer* renderer, float new_w, float new_h) {
        if (tex && w == new_w && h == new_h) return;

        free();

        tex = SDL_CreateTexture(
            renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, (int)new_w, (int)new_h);
        SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);

        if (!tex) {
            SDL_Log("Failed to create render target: %s", SDL_GetError());
            return;
        }

        w = new_w;
        h = new_h;
    }
};

inline Texture load_texture(const char* path, SDL_Renderer* renderer) {
    Texture t;

    t.tex = IMG_LoadTexture(renderer, path);
    if (!t.tex) {
        SDL_Log("Failed to load texture %s : %s", path, SDL_GetError());
        return {};
    }

    SDL_GetTextureSize(t.tex, &t.w, &t.h);
    return t;
}

using kuromasu_grid = ktl::grid<cell /*, ktl::ArenaAllocator<cell>*/>;

extern ktl::Arena g_arena;
extern ktl::ArenaAllocator<cell> g_cell_alloc;

struct state_t {
    kuromasu_grid game = kuromasu_grid(grid_size.x,
        grid_size.y,
        // g_cell_alloc,
        cell{.type = cell::blank, .observer_value = -1},
        ktl::GRID_GROW_OUTWARD | ktl::GRID_NO_RETAIN_STATE);

    uint32_t seed = 0;
    float dt = 0;
    uint64_t prev_time = 0;

    bool solved = false;
    bool auto_surround = false;

    kuromasu_grid solved_state = kuromasu_grid(grid_size.x,
        grid_size.y,
        // g_cell_alloc,
        cell{.type = cell::blank, .observer_value = -1},
        ktl::GRID_GROW_OUTWARD | ktl::GRID_NO_RETAIN_STATE);
    kuromasu_grid starting_pos = kuromasu_grid(grid_size.x,
        grid_size.y,
        // g_cell_alloc,
        cell{.type = cell::blank, .observer_value = -1},
        ktl::GRID_GROW_OUTWARD | ktl::GRID_NO_RETAIN_STATE);

    ImVec2 offset;
    float cell_size;

    std::unordered_map<int, TTF_Font*> fonts;
    std::unordered_map<int64_t, Texture> font_texture_cache;
    Texture win_image;

#if defined(__ANDROID__)
    bool horizontal_layout = false;
#else
    bool horizontal_layout = true;
#endif
    bool needs_dock_rebuild = true;

    bool custom_cursor = false;
    Texture cursor;

    std::vector<action> undo_stack;
    std::vector<action> redo_stack;

    struct {
        ktl::pos2_size start = ktl::pos2_size::invalid();
        SDL_FRect rect = {-1, -1, -1, -1};
        SDL_FRect dims = {0, 0, 0, 0};
    } measure;

    struct {
        ktl::pos2_size start = ktl::pos2_size::invalid();
        SDL_FRect rect = {-1, -1, -1, -1};
        SDL_FRect dims = {0, 0, 0, 0};
    } erase;

    struct {
        ktl::pos2_size start = ktl::pos2_size::invalid();
        action drag_action;
    } white_fill;
};

struct ctx_t {
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    bool running = true;

    state_t state;

    Texture game_tex;
};

TTF_Font* get_font(state_t& state, int size, const char* path = ASSET_DIR "Roboto-Regular.ttf");

#endif /* COMMON_H */
