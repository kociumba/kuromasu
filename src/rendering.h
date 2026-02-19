#ifndef RENDERING_H
#define RENDERING_H

#include <raylib.h>
#include "common.h"

Vector2 grid_to_screen_pos(const state_t& s, int grid_x, int grid_y);
Vector2 grid_to_screen_pos(const state_t& s, ktl::pos2_size grid_pos);

Rectangle grid_cell_rect(const state_t& s, int grid_x, int grid_y);
Rectangle grid_cell_rect(const state_t& s, ktl::pos2_size grid_pos);

Vector2 grid_cell_center(const state_t& s, int grid_x, int grid_y);
Vector2 grid_cell_center(const state_t& s, ktl::pos2_size pos);

Rectangle grid_rect_from_corners(ktl::pos2_size a, ktl::pos2_size b);
Rectangle grid_rect_from_topleft_and_size(ktl::pos2_size top_left, ktl::pos2_size size);

Rectangle grid_region_rect(const state_t& s, Rectangle grid_rect);
Rectangle grid_region_rect(const state_t& s, ktl::pos2_size top_left, ktl::pos2_size size);
Rectangle grid_region_rect(const state_t& s, ktl::pos2_size region);

bool is_pos_in_rect(ktl::pos2_size pos, Rectangle rect);

void draw_grid(state_t& s);
void draw_tooltip(state_t& s, const char* text, Vector2 mouse_pos, Vector2 render_size);
void draw_measure_overlay(state_t& s, Vector2 window_origin, Vector2 render_size);
void draw_erase_overlay(state_t& s);

#if defined(__ANDROID__)
#include <sys/system_properties.h>

inline float GetAndroidScale() {
    char prop_value[PROP_VALUE_MAX];
    if (__system_property_get("ro.sf.lcd_density", prop_value) > 0) {
        float dpi = std::atof(prop_value);
        return dpi / 160.0f;
    }
    return 1.0f;
}

inline float scale_to_DPIF(float value) { return value * GetAndroidScale(); }
inline int scale_to_DPII(int value) { return value * GetAndroidScale(); }
#else
inline float scale_to_DPIF(float value) { return GetWindowScaleDPI().x * value; }
inline int scale_to_DPII(int value) { return GetWindowScaleDPI().x * value; }
#endif

#endif /* RENDERING_H */
