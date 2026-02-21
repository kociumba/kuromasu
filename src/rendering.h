#ifndef RENDERING_H
#define RENDERING_H

#include "common.h"

ImVec2 grid_to_screen_pos(const state_t& s, int grid_x, int grid_y);
ImVec2 grid_to_screen_pos(const state_t& s, ktl::pos2_size grid_pos);

SDL_FRect grid_cell_rect(const state_t& s, int grid_x, int grid_y);
SDL_FRect grid_cell_rect(const state_t& s, ktl::pos2_size grid_pos);

ImVec2 grid_cell_center(const state_t& s, int grid_x, int grid_y);
ImVec2 grid_cell_center(const state_t& s, ktl::pos2_size pos);

SDL_FRect grid_rect_from_corners(ktl::pos2_size a, ktl::pos2_size b);
SDL_FRect grid_rect_from_topleft_and_size(ktl::pos2_size top_left, ktl::pos2_size size);

SDL_FRect grid_region_rect(const state_t& s, SDL_FRect grid_rect);
SDL_FRect grid_region_rect(const state_t& s, ktl::pos2_size top_left, ktl::pos2_size size);
SDL_FRect grid_region_rect(const state_t& s, ktl::pos2_size region);

bool is_pos_in_rect(ktl::pos2_size pos, SDL_FRect rect);

void set_render_color(SDL_Renderer* renderer, SDL_Color color);

inline int fade(int value, float scale) {
    scale = std::clamp(scale, 0.0f, 1.0f);
    return static_cast<int>(value * scale + 0.5f);
}

inline SDL_Color fade(SDL_Color color, float alpha) {
    if (alpha < 0.0f) alpha = 0.0f;
    if (alpha > 1.0f) alpha = 1.0f;

    SDL_Color result = color;

    result.a = (Uint8)(color.a * alpha + 0.5f);

    return result;
}

void draw_filled_circle(SDL_Renderer* renderer,
    float centerX,
    float centerY,
    float radius,
    SDL_Color color);

void draw_text(SDL_Renderer* renderer,
    TTF_Font* font,
    const char* text,
    float x,
    float y,
    SDL_Color color);

void draw_grid(ctx_t* ctx);
void draw_tooltip(ctx_t* ctx, const char* text, ImVec2 pos, ImVec2 render_size);
void draw_measure_overlay(ctx_t* ctx, ImVec2 window_origin, ImVec2 render_size);
void draw_erase_overlay(ctx_t* ctx);

inline float get_window_dpi_scale(SDL_Window* window) {
    if (!window) return 1.0f;
    float scale = SDL_GetWindowDisplayScale(window);
    if (scale <= 0.0f) {
        SDL_DisplayID display = SDL_GetDisplayForWindow(window);
        if (display != 0) { scale = SDL_GetDisplayContentScale(display); }
        if (scale <= 0.0f) scale = 1.0f;
    }
    return scale;
}

inline float scale_to_DPIF(float value, SDL_Window* window) {
    return value * get_window_dpi_scale(window);
}
inline int scale_to_DPII(int value, SDL_Window* window) {
    return (int)(value * get_window_dpi_scale(window) + 0.5f);
}

#endif /* RENDERING_H */
