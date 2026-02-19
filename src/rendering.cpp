#include "rendering.h"
#include "input.h"
#include "math.h"

Vector2 grid_to_screen_pos(const state_t& s, int grid_x, int grid_y) {
    return {s.offset.x + (grid_x * s.cell_size), s.offset.y + (grid_y * s.cell_size)};
}

Vector2 grid_to_screen_pos(const state_t& s, ktl::pos2_size grid_pos) {
    return grid_to_screen_pos(s, static_cast<int>(grid_pos.x), static_cast<int>(grid_pos.y));
}

Rectangle grid_cell_rect(const state_t& s, int grid_x, int grid_y) {
    Vector2 pos = grid_to_screen_pos(s, grid_x, grid_y);
    return {pos.x, pos.y, s.cell_size, s.cell_size};
}

Rectangle grid_cell_rect(const state_t& s, ktl::pos2_size grid_pos) {
    return grid_cell_rect(s, static_cast<int>(grid_pos.x), static_cast<int>(grid_pos.y));
}

Vector2 grid_cell_center(const state_t& s, int grid_x, int grid_y) {
    Rectangle r = grid_cell_rect(s, grid_x, grid_y);
    return {r.x + r.width * 0.5f, r.y + r.height * 0.5f};
}

Vector2 grid_cell_center(const state_t& s, ktl::pos2_size pos) {
    return grid_cell_center(s, static_cast<int>(pos.x), static_cast<int>(pos.y));
}

Rectangle grid_rect_from_corners(ktl::pos2_size a, ktl::pos2_size b) {
    if (a.x == ktl::pos2_size::invalid_value || a.y == ktl::pos2_size::invalid_value ||
        b.x == ktl::pos2_size::invalid_value || b.y == ktl::pos2_size::invalid_value) {
        return {0, 0, 0, 0};
    }

    int min_x = std::min(static_cast<int>(a.x), static_cast<int>(b.x));
    int min_y = std::min(static_cast<int>(a.y), static_cast<int>(b.y));
    int max_x = std::max(static_cast<int>(a.x), static_cast<int>(b.x));
    int max_y = std::max(static_cast<int>(a.y), static_cast<int>(b.y));

    return {static_cast<float>(min_x),
        static_cast<float>(min_y),
        static_cast<float>(max_x - min_x + 1),
        static_cast<float>(max_y - min_y + 1)};
}

Rectangle grid_rect_from_topleft_and_size(ktl::pos2_size top_left, ktl::pos2_size size) {
    if (top_left.x == ktl::pos2_size::invalid_value ||
        top_left.y == ktl::pos2_size::invalid_value || size.x == ktl::pos2_size::invalid_value ||
        size.y == ktl::pos2_size::invalid_value) {
        return {0, 0, 0, 0};
    }

    return {static_cast<float>(top_left.x),
        static_cast<float>(top_left.y),
        static_cast<float>(size.x),
        static_cast<float>(size.y)};
}

Rectangle grid_region_rect(const state_t& s, Rectangle grid_rect) {
    if (grid_rect.width <= 0 || grid_rect.height <= 0) { return {0, 0, 0, 0}; }

    Vector2 screen_pos =
        grid_to_screen_pos(s, static_cast<int>(grid_rect.x), static_cast<int>(grid_rect.y));

    return {
        screen_pos.x, screen_pos.y, grid_rect.width * s.cell_size, grid_rect.height * s.cell_size};
}

Rectangle grid_region_rect(const state_t& s, ktl::pos2_size top_left, ktl::pos2_size size) {
    Rectangle grid_r = grid_rect_from_topleft_and_size(top_left, size);
    return grid_region_rect(s, grid_r);
}

Rectangle grid_region_rect(const state_t& s, ktl::pos2_size region) {
    return grid_region_rect(s, region, region);
}

bool is_pos_in_rect(ktl::pos2_size pos, Rectangle rect) {
    if (pos.x >= rect.x && pos.x <= (rect.x + rect.width) && pos.y >= rect.y &&
        pos.y <= (rect.y + rect.height)) {
        return true;
    }

    return false;
}

void draw_grid(state_t& s) {
    float dt = GetFrameTime();
    float delay_duration = 1.0f;
    float fade_speed = 2.0f;

    for (const auto& item : s.game.items()) {
        ktl::pos2_size pos = item.position;
        Rectangle dest = grid_cell_rect(s, pos);
        Vector2 center = grid_cell_center(s, pos);

        Color cell_color = RAYWHITE;
        switch (item.value.type) {
            case cell::black:
                cell_color = BLACK;
                break;
            case cell::white:
                cell_color = WHITE;
                break;
            case cell::blank:
                cell_color = DARKGRAY;
                break;
        }

        DrawRectangleRec(dest, cell_color);

        DrawRectangleLinesEx(dest, 1.0f, BLACK);

        if (item.value.mistake) {
            if (item.value.mistake_delay > 0) {
                item.value.mistake_delay -= dt;
            } else {
                item.value.mistake_alpha = fminf(item.value.mistake_alpha + dt * fade_speed, 1.0f);
            }
        } else {
            item.value.mistake_delay = delay_duration;
            item.value.mistake_alpha = 0.0f;
        }

        if (item.value.mistake_alpha > 0.0f) {
            Color animated_red = Fade(RED, item.value.mistake_alpha);
            float radius = (s.cell_size / 2) - (s.cell_size / 10);
            DrawCircleV(center, radius, animated_red);
        }

        if (item.value.type == cell::white && item.value.observer_value != -1) {
            const char* text = TextFormat("%d", item.value.observer_value);

            int font_size = static_cast<int>(s.cell_size * 0.6f);
            Font font = get_font(s, font_size);
            Vector2 text_size = MeasureTextEx(font, text, font_size, 1.0f);

            float textX = center.x - text_size.x * 0.5f;
            float textY = center.y - font_size * 0.5f;

            DrawTextEx(font,
                text,
                {textX, textY},
                font_size,
                1.0f,
                item.value.observer_satisfied ? GRAY : BLACK);
        }
    }
}

void draw_tooltip(state_t& s, const char* text, Vector2 pos, Vector2 render_size) {
    float font_size = 24.0f;
    float padding = 10.0f;
    Color text_color = RAYWHITE;

    auto font = get_font(s, font_size);
    Vector2 text_size = MeasureTextEx(font, text, font_size, 1.0f);

    Vector2 draw_pos = {pos.x + s.offset.x, pos.y + s.offset.y};

    Rectangle tooltip_rect = {
        draw_pos.x + 15, draw_pos.y + 15, text_size.x + (padding * 2), text_size.y + (padding * 2)};

    if (tooltip_rect.x + tooltip_rect.width > render_size.x)
        tooltip_rect.x = draw_pos.x - tooltip_rect.width - 5;

    if (tooltip_rect.y + tooltip_rect.height > render_size.y)
        tooltip_rect.y = draw_pos.y - tooltip_rect.height - 5;

    DrawRectangleRounded(tooltip_rect, 0.3f, 32, GetColor(0x202020EE));

    DrawTextEx(font,
        text,
        Vector2{tooltip_rect.x + padding, tooltip_rect.y + padding},
        font_size,
        1.0f,
        text_color);
}

void draw_measure_overlay(state_t& s, Vector2 window_origin, Vector2 render_size) {
    if (s.measure.rect.width <= 0.0f || s.measure.rect.height <= 0.0f) return;

    DrawRectangleRec(s.measure.rect, Fade(ORANGE, 0.20f));
    DrawRectangleLinesEx(s.measure.rect, 3.0f, ORANGE);

    Vector2 rel_mouse = get_relative_mouse(state, window_origin);
    draw_tooltip(s,
        TextFormat("%.0f, %.0f", s.measure.dims.width, s.measure.dims.height),
        rel_mouse,
        render_size);
}

void draw_erase_overlay(state_t& s) {
    if (s.erase.rect.width <= 0.0f || s.erase.rect.height <= 0.0f) return;

    DrawRectangleRec(s.erase.rect, Fade(BLUE, 0.20f));
    DrawRectangleLinesEx(s.erase.rect, 3.0f, BLUE);
}
