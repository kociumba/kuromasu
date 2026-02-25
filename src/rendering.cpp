#include "rendering.h"
#include <SDL3/SDL_render.h>
#include "common.h"
#include "external/memory_usage.h"
#include "input.h"
#include "math.h"

ImVec2 grid_to_screen_pos(const state_t& s, int grid_x, int grid_y) {
    zone_scoped_n("grid to screen pos");
    return {s.offset.x + (grid_x * s.cell_size), s.offset.y + (grid_y * s.cell_size)};
}

ImVec2 grid_to_screen_pos(const state_t& s, ktl::pos2_size grid_pos) {
    return grid_to_screen_pos(s, static_cast<int>(grid_pos.x), static_cast<int>(grid_pos.y));
}

SDL_FRect grid_cell_rect(const state_t& s, int grid_x, int grid_y) {
    zone_scoped_n("grid cell rect");
    ImVec2 pos = grid_to_screen_pos(s, grid_x, grid_y);
    return {pos.x, pos.y, s.cell_size, s.cell_size};
}

SDL_FRect grid_cell_rect(const state_t& s, ktl::pos2_size grid_pos) {
    return grid_cell_rect(s, static_cast<int>(grid_pos.x), static_cast<int>(grid_pos.y));
}

ImVec2 grid_cell_center(const state_t& s, int grid_x, int grid_y) {
    zone_scoped_n("grid cell center");
    SDL_FRect r = grid_cell_rect(s, grid_x, grid_y);
    return {r.x + r.w * 0.5f, r.y + r.h * 0.5f};
}

ImVec2 grid_cell_center(const state_t& s, ktl::pos2_size pos) {
    return grid_cell_center(s, static_cast<int>(pos.x), static_cast<int>(pos.y));
}

SDL_FRect grid_rect_from_corners(ktl::pos2_size a, ktl::pos2_size b) {
    zone_scoped_n("grid rect from corners");
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
    zone_scoped_n("grid rect from topleft and size");
    if (top_left.x == ktl::pos2_size::invalid_value ||
        top_left.y == ktl::pos2_size::invalid_value || size.x == ktl::pos2_size::invalid_value ||
        size.y == ktl::pos2_size::invalid_value) {
        return {0, 0, 0, 0};
    }

    return {(float)top_left.x, (float)top_left.y, (float)size.x, (float)size.y};
}

SDL_FRect grid_region_rect(const state_t& s, SDL_FRect grid_rect) {
    zone_scoped_n("grid region rect");
    if (grid_rect.w <= 0 || grid_rect.h <= 0) { return {0, 0, 0, 0}; }

    ImVec2 screen_pos =
        grid_to_screen_pos(s, static_cast<int>(grid_rect.x), static_cast<int>(grid_rect.y));

    return {screen_pos.x, screen_pos.y, grid_rect.w * s.cell_size, grid_rect.h * s.cell_size};
}

SDL_FRect grid_region_rect(const state_t& s, ktl::pos2_size top_left, ktl::pos2_size size) {
    zone_scoped_n("grid region rect");
    SDL_FRect grid_r = grid_rect_from_topleft_and_size(top_left, size);
    return grid_region_rect(s, grid_r);
}

SDL_FRect grid_region_rect(const state_t& s, ktl::pos2_size region) {
    return grid_region_rect(s, region, region);
}

bool is_pos_in_rect(ktl::pos2_size pos, SDL_FRect rect) {
    zone_scoped_n("is pos in rect");
    if (pos.x >= rect.x && pos.x <= (rect.x + rect.w) && pos.y >= rect.y &&
        pos.y <= (rect.y + rect.h)) {
        return true;
    }

    return false;
}

void set_render_color(SDL_Renderer* renderer, SDL_Color color) {
    zone_scoped_n("set render color");
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
}

void draw_filled_circle(SDL_Renderer* renderer,
    float centerX,
    float centerY,
    float radius,
    SDL_Color color) {
    zone_scoped_n("draw filled circle");

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

void draw_text(ctx_t* ctx, TTF_Font* font, const char* text, float x, float y, SDL_Color color) {
    zone_scoped_n("draw text");

    if (!text || !font || !ctx->renderer) return;

    uint64_t key = get_font_cache_key(font, text, color);
    auto it = ctx->state.font_texture_cache.find(key);
    if (it == ctx->state.font_texture_cache.end()) {
        zone_scoped_nc("cache miss", PROF_COLOR_RED);

        SDL_Surface* surface;
        SDL_Texture* texture;

        surface = TTF_RenderText_Blended(font, text, 0, color);
        if (!surface) return;

        texture = SDL_CreateTextureFromSurface(ctx->renderer, surface);
        if (texture) {
            SDL_FRect dst = {x, y, (float)surface->w, (float)surface->h};

            SDL_RenderTexture(ctx->renderer, texture, NULL, &dst);

            ctx->state.font_texture_cache[key] =
                Texture{texture, (float)surface->w, (float)surface->h};
        }
        SDL_DestroySurface(surface);
    } else {
        zone_scoped_nc("cache hit", PROF_COLOR_GREEN);

        SDL_FRect dst = {x, y, (float)it->second.w, (float)it->second.h};
        SDL_RenderTexture(ctx->renderer, it->second.tex, NULL, &dst);
    }
}

void draw_grid(ctx_t* ctx) {
    zone_scoped_n("draw grid");

    float delay_duration = 1.0f;
    float fade_speed = 2.0f;
    auto& s = ctx->state;

    for (const auto& item : s.game.items()) {
        zone_scoped_n("draw cell");
        // zone_text("draw cell (%d : %d)", item.position.x, item.position.y);

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
        {
            zone_scoped_n("render cell bg");
            SDL_RenderFillRect(ctx->renderer, &dest);
        }

        set_render_color(ctx->renderer, {0, 0, 0, 255});
        {
            zone_scoped_n("render cell border");
            SDL_RenderRect(ctx->renderer, &dest);
        }

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
            zone_scoped_n("text measuring");

            char* text;
            {
                zone_scoped_n("formatting");
                SDL_asprintf(&text, "%d", item.value.observer_value);
            }

            int font_size = static_cast<int>(s.cell_size * 0.6f);
            auto font = get_font(s, font_size);

            auto text_color = item.value.observer_satisfied ? SDL_Color{130, 130, 130, 255}
                                                            : SDL_Color{0, 0, 0, 255};

            int ascent, descent;
            {
                zone_scoped_n("query font params");
                ascent = TTF_GetFontAscent(font);
                descent = TTF_GetFontDescent(font);
            }
            int visual_height = ascent - descent;

            int advance_w = 0;
            int advance_h = 0;
            {
                zone_scoped_n("get string size");
                TTF_GetStringSize(font, text, 0, &advance_w, &advance_h);
            }

            float textX = center.x - advance_w * 0.5f;
            float textY = center.y - advance_h * 0.5f;

            draw_text(ctx, font, text, textX, textY, text_color);
            SDL_free(text);
        }
    }
}

void draw_tooltip(ctx_t* ctx, const char* text, ImVec2 pos, ImVec2 render_size) {
    zone_scoped_n("draw tooltip");

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

    draw_text(ctx, font, text, tooltip_rect.x + padding, tooltip_rect.y + padding, text_color);
}

void draw_measure_overlay(ctx_t* ctx, ImVec2 window_origin, ImVec2 render_size) {
    zone_scoped_n("draw measure overlay");

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
    SDL_free(txt);
}

void draw_erase_overlay(ctx_t* ctx) {
    zone_scoped_n("draw erase overlay");

    auto& s = ctx->state;
    if (s.erase.rect.w <= 0.0f || s.erase.rect.h <= 0.0f) return;

    SDL_Color blue = {0, 121, 241, 255};

    set_render_color(ctx->renderer, fade(blue, .20f));
    SDL_RenderFillRect(ctx->renderer, &s.erase.rect);
    set_render_color(ctx->renderer, blue);
    SDL_RenderRect(ctx->renderer, &s.erase.rect);
}

void debug_overlay(ctx_t* ctx, ImVec2 pos) {
    zone_scoped_n("debug overlay");

    static std::vector<float> frame_times;
    static float time_sum = 0.0f;
    constexpr int MAX_SAMPLES = 120;

    float dt_ms = ctx->state.dt * 1000.0f;
    frame_times.push_back(dt_ms);
    time_sum += dt_ms;

    if (frame_times.size() > MAX_SAMPLES) {
        time_sum -= frame_times.front();
        frame_times.erase(frame_times.begin());
    }

    const float avg_ms = time_sum / static_cast<float>(frame_times.size());
    const float avg_fps = (avg_ms > 0.0001f) ? 1000.0f / avg_ms : 0.0f;

    ImDrawList* draw_list = ImGui::GetForegroundDrawList();
    if (!draw_list) return;

    ImU32 color = IM_COL32(245, 245, 245, 255);
    ImU32 shadow = IM_COL32(10, 10, 20, 160);

    ImU32 fps_color = (avg_fps >= 60.f)   ? IM_COL32(80, 255, 120, 255)
                      : (avg_fps >= 30.f) ? IM_COL32(255, 220, 80, 255)
                                          : IM_COL32(255, 70, 70, 255);

    char buf[96];
    const float font_size = ImGui::GetFontSize();
    const float line_h = font_size * 1.10f;

    auto print = [&](ImU32 col, const char* fmt, ...) IM_FMTARGS(3) {
        va_list args;
        va_start(args, fmt);
        vsnprintf(buf, sizeof(buf), fmt, args);
        va_end(args);

        draw_list->AddText(
            ImGui::GetFont(), font_size, ImVec2(pos.x + 1.2f, pos.y + 1.2f), shadow, buf);

        draw_list->AddText(ImGui::GetFont(), font_size, pos, col, buf);

        pos.y += line_h;
    };

    print(fps_color,
        "FPS %.1f (%.2f ms/frame) using %s",
        avg_fps,
        avg_ms,
        SDL_GetRendererName(ctx->renderer));

    print(color, BUILD_IDENTIFIER);

    print(color, "%s", get_memory_usage_str_mb());

    ImGuiIO& io = ImGui::GetIO();
    bool mouse_valid = ImGui::IsMousePosValid(&io.MousePos);
    float mouse_x = mouse_valid ? io.MousePos.x : 0.0f;
    float mouse_y = mouse_valid ? io.MousePos.y : 0.0f;

    print(color, "Mouse %.0f, %.0f", mouse_x, mouse_y);

#if !defined(NDEBUG)
    print(color, "regions created: %d", g_arena.region_creations);
    print(color, "cross regions allocations: %d", g_arena.allocations_bigger_than_region_size);
#endif
}