#include "rendering.h"
#include "input.h"
#include "math.h"

ImVec2 grid_to_screen_pos(const state_t& s, int grid_x, int grid_y) {
    return {s.offset.x + (grid_x * s.cell_size), s.offset.y + (grid_y * s.cell_size)};
}

ImVec2 grid_to_screen_pos(const state_t& s, ktl::pos2_size grid_pos) {
    return grid_to_screen_pos(s, static_cast<int>(grid_pos.x), static_cast<int>(grid_pos.y));
}

SDL_FRect grid_cell_rect(const state_t& s, int grid_x, int grid_y) {
    ImVec2 pos = grid_to_screen_pos(s, grid_x, grid_y);
    return {pos.x, pos.y, s.cell_size, s.cell_size};
}

SDL_FRect grid_cell_rect(const state_t& s, ktl::pos2_size grid_pos) {
    return grid_cell_rect(s, static_cast<int>(grid_pos.x), static_cast<int>(grid_pos.y));
}

ImVec2 grid_cell_center(const state_t& s, int grid_x, int grid_y) {
    SDL_FRect r = grid_cell_rect(s, grid_x, grid_y);
    return {r.x + r.w * 0.5f, r.y + r.h * 0.5f};
}

ImVec2 grid_cell_center(const state_t& s, ktl::pos2_size pos) {
    return grid_cell_center(s, static_cast<int>(pos.x), static_cast<int>(pos.y));
}

SDL_FRect grid_rect_from_corners(ktl::pos2_size a, ktl::pos2_size b) {
    if (a.x == ktl::pos2_size::invalid_value || a.y == ktl::pos2_size::invalid_value ||
        b.x == ktl::pos2_size::invalid_value || b.y == ktl::pos2_size::invalid_value) {
        return {0, 0, 0, 0};
    }

    int min_x = std::min(static_cast<int>(a.x), static_cast<int>(b.x));
    int min_y = std::min(static_cast<int>(a.y), static_cast<int>(b.y));
    int max_x = std::max(static_cast<int>(a.x), static_cast<int>(b.x));
    int max_y = std::max(static_cast<int>(a.y), static_cast<int>(b.y));

    return {(float)min_x, (float)min_y, (float)(max_x - min_x + 1), (float)(max_y - min_y + 1)};
}

SDL_FRect grid_rect_from_topleft_and_size(ktl::pos2_size top_left, ktl::pos2_size size) {
    if (top_left.x == ktl::pos2_size::invalid_value ||
        top_left.y == ktl::pos2_size::invalid_value || size.x == ktl::pos2_size::invalid_value ||
        size.y == ktl::pos2_size::invalid_value) {
        return {0, 0, 0, 0};
    }

    return {(float)top_left.x, (float)top_left.y, (float)size.x, (float)size.y};
}

SDL_FRect grid_region_rect(const state_t& s, SDL_FRect grid_rect) {
    if (grid_rect.w <= 0 || grid_rect.h <= 0) { return {0, 0, 0, 0}; }

    ImVec2 screen_pos =
        grid_to_screen_pos(s, static_cast<int>(grid_rect.x), static_cast<int>(grid_rect.y));

    return {screen_pos.x, screen_pos.y, grid_rect.w * s.cell_size, grid_rect.h * s.cell_size};
}

SDL_FRect grid_region_rect(const state_t& s, ktl::pos2_size top_left, ktl::pos2_size size) {
    SDL_FRect grid_r = grid_rect_from_topleft_and_size(top_left, size);
    return grid_region_rect(s, grid_r);
}

SDL_FRect grid_region_rect(const state_t& s, ktl::pos2_size region) {
    return grid_region_rect(s, region, region);
}

bool is_pos_in_rect(ktl::pos2_size pos, SDL_FRect rect) {
    if (pos.x >= rect.x && pos.x <= (rect.x + rect.w) && pos.y >= rect.y &&
        pos.y <= (rect.y + rect.h)) {
        return true;
    }

    return false;
}

void set_render_color(SDL_Renderer* renderer, SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
}

void draw_filled_circle(SDL_Renderer* renderer,
    float centerX,
    float centerY,
    float radius,
    SDL_Color color) {
    const int segments = 64;
    SDL_Vertex vertices[segments + 2];
    int indices[segments * 3];

    SDL_FColor fc = {color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f};
    vertices[0] = {{centerX, centerY}, fc, {0, 0}};

    for (int i = 0; i <= segments; i++) {
        float angle = i * 2.0f * SDL_PI_F / segments;
        vertices[i + 1] = {
            {centerX + radius * SDL_cosf(angle), centerY + radius * SDL_sinf(angle)}, fc, {0, 0}};
    }

    for (int i = 0; i < segments; i++) {
        indices[i * 3 + 0] = 0;
        indices[i * 3 + 1] = i + 1;
        indices[i * 3 + 2] = i + 2;
    }

    SDL_RenderGeometry(renderer, NULL, vertices, segments + 2, indices, segments * 3);
}

void draw_text(SDL_Renderer* renderer,
    TTF_Font* font,
    const char* text,
    float x,
    float y,
    SDL_Color color) {
    if (!text || !font || !renderer) return;

    SDL_Surface* surface = TTF_RenderText_Blended(font, text, 0, color);
    if (!surface) return;

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture) {
        SDL_FRect dst = {x, y, (float)surface->w, (float)surface->h};

        SDL_RenderTexture(renderer, texture, NULL, &dst);

        SDL_DestroyTexture(texture);
    }
    SDL_DestroySurface(surface);
}

void draw_grid(ctx_t* ctx) {
    float delay_duration = 1.0f;
    float fade_speed = 2.0f;
    auto& s = ctx->state;

    for (const auto& item : s.game.items()) {
        ktl::pos2_size pos = item.position;
        SDL_FRect dest = grid_cell_rect(s, pos);
        ImVec2 center = grid_cell_center(s, pos);

        SDL_Color cell_color = {245, 245, 245, 255};
        switch (item.value.type) {
            case cell::black:
                cell_color = {0, 0, 0, 255};
                break;
            case cell::white:
                cell_color = {255, 255, 255, 255};
                break;
            case cell::blank:
                cell_color = {80, 80, 80, 255};
                break;
        }

        set_render_color(ctx->renderer, cell_color);
        SDL_RenderFillRect(ctx->renderer, &dest);

        set_render_color(ctx->renderer, {0, 0, 0, 255});
        SDL_RenderRect(ctx->renderer, &dest);

        if (item.value.mistake) {
            if (item.value.mistake_delay > 0) {
                item.value.mistake_delay -= s.dt;
            } else {
                item.value.mistake_alpha =
                    fminf(item.value.mistake_alpha + s.dt * fade_speed, 1.0f);
            }
        } else {
            item.value.mistake_delay = delay_duration;
            item.value.mistake_alpha = 0.0f;
        }

        if (item.value.mistake_alpha > 0.0f) {
            SDL_Color animated_red = {255, 0, 0, (uint8_t)fade(255, item.value.mistake_alpha)};
            float radius = (s.cell_size / 2) - (s.cell_size / 10);
            draw_filled_circle(ctx->renderer, center.x, center.y, radius, animated_red);
        }

        if (item.value.type == cell::white && item.value.observer_value != -1) {
            char* text;
            SDL_asprintf(&text, "%d", item.value.observer_value);

            int font_size = static_cast<int>(s.cell_size * 0.6f);
            auto font = get_font(s, font_size);

            auto text_color = item.value.observer_satisfied ? SDL_Color{130, 130, 130, 255}
                                                            : SDL_Color{0, 0, 0, 255};

            int ascent = TTF_GetFontAscent(font);
            int descent = TTF_GetFontDescent(font);
            int visual_height = ascent - descent;

            int advance_w = 0;
            int advance_h = 0;
            TTF_GetStringSize(font, text, 0, &advance_w, &advance_h);

            float textX = center.x - advance_w * 0.5f;
            float textY = center.y - advance_h * 0.5f;

            draw_text(ctx->renderer, font, text, textX, textY, text_color);
        }
    }
}

void draw_tooltip(ctx_t* ctx, const char* text, ImVec2 pos, ImVec2 render_size) {
    float font_size = 24.0f;
    float padding = 10.0f;
    SDL_Color text_color = {245, 245, 245, 255};
    SDL_Color bg = {32, 32, 32, 255};
    auto& s = ctx->state;

    auto font = get_font(s, font_size);
    ImVec2 text_size;
    TTF_GetStringSize(font, text, 0, (int*)&text_size.x, (int*)&text_size.y);

    ImVec2 draw_pos = {pos.x + s.offset.x, pos.y + s.offset.y};

    auto x = scale_to_DPIF(15, ctx->window);
    auto y = scale_to_DPIF(padding * 2, ctx->window);
    SDL_FRect tooltip_rect = {draw_pos.x + x, draw_pos.y + x, text_size.x + y, text_size.y + y};

    if (tooltip_rect.x + tooltip_rect.w > render_size.x)
        tooltip_rect.x = draw_pos.x - tooltip_rect.w - 5;

    if (tooltip_rect.y + tooltip_rect.h > render_size.y)
        tooltip_rect.y = draw_pos.y - tooltip_rect.h - 5;

    set_render_color(ctx->renderer, bg);
    SDL_RenderFillRect(ctx->renderer, &tooltip_rect);

    draw_text(
        ctx->renderer, font, text, tooltip_rect.x + padding, tooltip_rect.y + padding, text_color);
}

void draw_measure_overlay(ctx_t* ctx, ImVec2 window_origin, ImVec2 render_size) {
    auto& s = ctx->state;
    if (s.measure.rect.w <= 0.0f || s.measure.rect.h <= 0.0f) return;

    SDL_Color orange = {255, 161, 0, 255};
    set_render_color(ctx->renderer, fade(orange, .20f));
    SDL_RenderFillRect(ctx->renderer, &s.measure.rect);
    set_render_color(ctx->renderer, orange);
    SDL_RenderRect(ctx->renderer, &s.measure.rect);

    ImVec2 rel_mouse = get_relative_mouse(s, window_origin);
    char* txt;
    SDL_asprintf(&txt, "%.0f, %.0f", s.measure.dims.w, s.measure.dims.h);
    draw_tooltip(ctx, txt, rel_mouse, render_size);
}

void draw_erase_overlay(ctx_t* ctx) {
    auto& s = ctx->state;
    if (s.erase.rect.w <= 0.0f || s.erase.rect.h <= 0.0f) return;

    SDL_Color blue = {0, 121, 241, 255};

    set_render_color(ctx->renderer, fade(blue, .20f));
    SDL_RenderFillRect(ctx->renderer, &s.erase.rect);
    set_render_color(ctx->renderer, blue);
    SDL_RenderRect(ctx->renderer, &s.erase.rect);
}
