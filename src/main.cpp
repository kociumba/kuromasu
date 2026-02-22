#include <imgui_internal.h>
#include "common.h"
#include "external/FA6FreeSolidFontData.h"
#include "input.h"
#include "kuromasu.h"
#include "rendering.h"
#include "theme.h"
#include "ui.h"

#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL_main.h>

extern "C" {
extern const char _binary_Roboto_Regular_ttf_end[];
extern const char _binary_Roboto_Regular_ttf_start[];
}

constexpr int ICON_MIN_FA = 0xe005;
constexpr int ICON_MAX_FA = 0xf8ff;

TTF_Font* get_font(state_t& state, int size, const char* path) {
    auto it = state.fonts.find(size);
    if (it != state.fonts.end()) { return it->second; }

    TTF_Font* font = TTF_OpenFont(path, (float)size);
    if (!font) {
        SDL_Log("Failed to load font: %s", SDL_GetError());
        return nullptr;
    }

    state.fonts[size] = font;
    return font;
}

SDL_AppResult SDL_AppInit(void** appstate, int argc, char** argv) {
    auto* ctx = new ctx_t();
    *appstate = ctx;

    SDL_SetAppMetadata("kuromasu", "0.0.0", "xyz.kociumba.kuromasu");

    uint32_t window_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_HIGH_PIXEL_DENSITY;
#if defined(__ANDROID__)
    window_flags |= SDL_WINDOW_FULLSCREEN;
#else
    window_flags |= SDL_WINDOW_RESIZABLE;
    SDL_SetWindowSize(ctx->window, 800, 600);  // set before create on desktop
#endif

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!TTF_Init()) {
        SDL_Log("TTF_Init failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer(
            "kuromasu", 800, 600, window_flags, &ctx->window, &ctx->renderer)) {
        SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_SetRenderDrawBlendMode(ctx->renderer, SDL_BLENDMODE_BLEND);

    // ImGui setup
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::theme();  // your existing theme

    auto& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
#if defined(__ANDROID__)
    io.ConfigFlags |= ImGuiConfigFlags_IsTouchScreen;
    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
#endif

    ImGui_ImplSDL3_InitForSDLRenderer(ctx->window, ctx->renderer);
    ImGui_ImplSDLRenderer3_Init(ctx->renderer);

    static const ImWchar icons_ranges[] = {ICON_MIN_FA, ICON_MAX_FA, 0};
    ImFontConfig icons_config;
    icons_config.MergeMode = true;
    icons_config.PixelSnapH = true;
    icons_config.FontDataOwnedByAtlas = false;

    icons_config.GlyphMaxAdvanceX = std::numeric_limits<float>::max();
    icons_config.RasterizerMultiply = 1.0f;
    icons_config.OversampleH = 3;
    icons_config.OversampleV = 1;
    icons_config.GlyphOffset.x = -2.5f;

    icons_config.GlyphRanges = icons_ranges;

    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
#if defined(__ANDROID__)
    io.ConfigFlags |= ImGuiConfigFlags_IsTouchScreen;
    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
#endif

    ImFontConfig font;
    font.FontDataOwnedByAtlas = false;
    font.OversampleH = 2;
    font.OversampleV = 1;
    std::snprintf(font.Name, sizeof(font.Name), "%s", "Roboto Regular");
    const uint32_t size =
        (uint32_t)(_binary_Roboto_Regular_ttf_end - _binary_Roboto_Regular_ttf_start);
    auto imfont = io.Fonts->AddFontFromMemoryTTF(
        (void*)_binary_Roboto_Regular_ttf_start, size, scale_to_DPII(18, ctx->window), &font);
    io.Fonts->AddFontFromMemoryCompressedTTF((void*)fa_solid_900_compressed_data,
        fa_solid_900_compressed_size,
        scale_to_DPII(16, ctx->window),
        &icons_config,
        icons_ranges);
    io.FontDefault = imfont;

    auto& s = ImGui::GetStyle();
    s.ItemSpacing = {scale_to_DPIF(5, ctx->window), scale_to_DPIF(5, ctx->window)};
    s.ItemInnerSpacing = {scale_to_DPIF(3, ctx->window), scale_to_DPIF(3, ctx->window)};
#if defined(__ANDROID__)
    s.TouchExtraPadding = {3, 3};
    s.ScrollbarSize *= scale_to_DPIF(2, ctx->window);
#endif

    ctx->state.win_image = load_texture(ASSET_DIR "win.jpg", ctx->renderer);
    ctx->state.cursor = load_texture(ASSET_DIR "cursor.png", ctx->renderer);

    ctx->state.seed = generate_board(ctx->state);

    int w, h;
    SDL_GetWindowSizeInPixels(ctx->window, &w, &h);
    ctx->game_tex.resize(ctx->renderer, w, h);
    SDL_SetTextureBlendMode(ctx->game_tex.tex, SDL_BLENDMODE_BLEND);

    ctx->state.prev_time = SDL_GetTicksNS();

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
    auto* ctx = (ctx_t*)appstate;

    ImGui_ImplSDL3_ProcessEvent(event);

    if (event->type == SDL_EVENT_QUIT) { return SDL_APP_SUCCESS; }

    if (event->type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED) {
        ctx->game_tex.resize(ctx->renderer, event->window.data1, event->window.data2);
        ctx->state.needs_dock_rebuild = true;
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate) {
    auto* ctx = (ctx_t*)appstate;
    auto& state = ctx->state;

    auto current_time = SDL_GetTicksNS();
    state.dt = (double)(current_time - state.prev_time) / 1'000'000'000.0;
    state.prev_time = current_time;

    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    int fb_w, fb_h;
    SDL_GetWindowSizeInPixels(ctx->window, &fb_w, &fb_h);
    SDL_Rect viewport = {0, 0, fb_w, fb_h};
    SDL_SetRenderViewport(ctx->renderer, &viewport);
    SDL_SetRenderDrawColor(ctx->renderer, 0, 0, 0, 0);
    SDL_RenderClear(ctx->renderer);

    ImGuiID dockspace_id = ImGui::DockSpaceOverViewport(0,
        NULL,
        ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_NoTabBar |
            ImGuiDockNodeFlags_NoWindowMenuButton);

    if (state.needs_dock_rebuild) {
        state.needs_dock_rebuild = false;

        ImGui::DockBuilderRemoveNode(dockspace_id);
        ImGui::DockBuilderAddNode(dockspace_id,
            ImGuiDockNodeFlags_DockSpace | ImGuiDockNodeFlags_NoResize |
                ImGuiDockNodeFlags_NoDockingSplit | ImGuiDockNodeFlags_NoUndocking);

        ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->Size);

        ImGuiID dock_game, dock_controls;
        if (state.horizontal_layout) {
            ImGui::DockBuilderSplitNode(
                dockspace_id, ImGuiDir_Right, 0.75f, &dock_game, &dock_controls);
        } else {
            ImGui::DockBuilderSplitNode(
                dockspace_id, ImGuiDir_Up, 0.70f, &dock_game, &dock_controls);
        }

        ImGui::DockBuilderDockWindow("##game", dock_game);
        ImGui::DockBuilderDockWindow("Controlls", dock_controls);
        ImGui::DockBuilderFinish(dockspace_id);
    }

    ImGui::Begin("Controlls", nullptr);
    board_controls(ctx);
    ImGui::End();

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("##game",
        nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground |
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoNav);

    ImVec2 cursor_origin = ImGui::GetCursorScreenPos();
    if (ImGui::IsWindowHovered() || state.white_fill.start != ktl::pos2_size::invalid()) {
        mouse_input(ctx->state, cursor_origin);
    }
    keyboard_input(ctx->state);

    ImVec2 render_size = ImGui::GetContentRegionAvail();
    if (render_size.x != ctx->game_tex.w || render_size.y != ctx->game_tex.h) {
        if (render_size.x > 0 && render_size.y > 0) {
            ctx->game_tex.resize(ctx->renderer, render_size.x, render_size.y);
        }
    }

    auto fit_size = std::min(render_size.x, render_size.y);
    state.cell_size = fit_size / state.game.width;
    state.offset = {
        (render_size.x - (state.cell_size * state.game.width)) / 2,
        (render_size.y - (state.cell_size * state.game.height)) / 2,
    };

    SDL_SetRenderTarget(ctx->renderer, ctx->game_tex.tex);
    SDL_SetRenderDrawColor(ctx->renderer, 0, 0, 0, 0);
    SDL_RenderClear(ctx->renderer);

    draw_grid(ctx);

    draw_measure_overlay(ctx, cursor_origin, render_size);
    draw_erase_overlay(ctx);

    if (state.solved) {
        int win_x = (render_size.x / 2) - (state.win_image.w / 2);
        int win_y = (render_size.y / 2) - (state.win_image.h / 2);

        SDL_SetRenderDrawColor(ctx->renderer, 0, 0, 0, 150);
        SDL_RenderFillRect(ctx->renderer, nullptr);
        SDL_FRect dst_rect = {(float)win_x, (float)win_y, state.win_image.w, state.win_image.h};
        SDL_RenderTexture(ctx->renderer, state.win_image.tex, nullptr, &dst_rect);
    }

    SDL_SetRenderTarget(ctx->renderer, nullptr);

    ImGui::Image((ImTextureID)ctx->game_tex.tex, render_size);

    ImGui::End();
    ImGui::PopStyleVar();

    ImGui::Render();
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), ctx->renderer);

    if (state.custom_cursor) {
        if (SDL_HideCursor()) {
            auto& io = ImGui::GetIO();

            if (true) {
                float logical_x = io.MousePos.x;
                float logical_y = io.MousePos.y;

                float pixel_x = logical_x * scale_to_DPIF(1.0f, ctx->window);
                float pixel_y = logical_y * scale_to_DPIF(1.0f, ctx->window);

                float cursor_w =
                    static_cast<float>(state.cursor.w) * scale_to_DPIF(1.0f, ctx->window);
                float cursor_h =
                    static_cast<float>(state.cursor.h) * scale_to_DPIF(1.0f, ctx->window);

                SDL_FRect dest = {pixel_x, pixel_y, cursor_w, cursor_h};
                SDL_RenderTexture(ctx->renderer, state.cursor.tex, nullptr, &dest);
            }
        } else {
            SDL_Log("SDL_HideCursor failed: %s", SDL_GetError());
        }
    } else {
        if (!SDL_ShowCursor()) { SDL_Log("SDL_ShowCursor failed: %s", SDL_GetError()); }
    }

    SDL_RenderPresent(ctx->renderer);

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result) {
    auto* ctx = (ctx_t*)appstate;

    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    ctx->game_tex.free();
    ctx->state.cursor.free();
    ctx->state.win_image.free();
    SDL_DestroyRenderer(ctx->renderer);
    SDL_DestroyWindow(ctx->window);
    SDL_Quit();

    delete ctx;
}
