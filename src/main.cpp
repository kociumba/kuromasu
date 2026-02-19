#include <extras/FA6FreeSolidFontData.h>
#include <imgui_internal.h>
#include <rlImGui.h>
#include "common.h"
#include "input.h"
#include "kuromasu.h"
#include "rendering.h"
#include "theme.h"
#include "ui.h"

state_t state;

extern "C" {
extern const char _binary_Roboto_Regular_ttf_end[];
extern const char _binary_Roboto_Regular_ttf_start[];
}

#if defined(__ANDROID__)
Vector2 initial_size = {0, 0};
#else
Vector2 initial_size = {800, 600};
#endif

void set_flags() {
    auto flags = FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT;
#if defined(__ANDROID__)
    flags = flags | FLAG_WINDOW_MAXIMIZED;
#else
    flags = flags | FLAG_WINDOW_RESIZABLE;
#endif
    SetConfigFlags(flags);
}

Font& get_font(state_t& state, int size, const char* path) {
    auto it = state.fonts.find(size);

    if (it != state.fonts.end()) { return it->second; }

    state.fonts[size] = LoadFontEx(path, size, nullptr, 0);
    SetTextureFilter(state.fonts[size].texture, TEXTURE_FILTER_BILINEAR);

    return state.fonts[size];
}

int main() {
    // __debugbreak();
    set_flags();
    InitWindow(initial_size.x, initial_size.y, "kuromasu");
    set_flags();  // some flags only work before or after init window, and I don't want to have to know which ones
    SetExitKey(NULL);

    rlImGuiSetup(true);
    ImGui::theme();

    get_font(state, 24);

    state.seed = generate_board(state);

    state.win_image = LoadTexture(ASSET_DIR "win.jpg");
    SetTextureFilter(state.win_image, TEXTURE_FILTER_BILINEAR);

    auto c = LoadImage(ASSET_DIR "cursor.png");
    ImageResize(&c, scale_to_DPII(32), scale_to_DPII(32));
    state.cursor = LoadTextureFromImage(c);
    UnloadImage(c);
    SetTextureFilter(state.cursor, TEXTURE_FILTER_BILINEAR);

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

    auto& io = ImGui::GetIO();
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
        (void*)_binary_Roboto_Regular_ttf_start, size, scale_to_DPII(18), &font);
    io.Fonts->AddFontFromMemoryCompressedTTF((void*)fa_solid_900_compressed_data,
        fa_solid_900_compressed_size,
        scale_to_DPII(16),
        &icons_config,
        icons_ranges);
    io.FontDefault = imfont;

    RenderTexture2D game_tex = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());

    auto& s = ImGui::GetStyle();
    s.ItemSpacing = {scale_to_DPIF(5), scale_to_DPIF(5)};
    s.ItemInnerSpacing = {scale_to_DPIF(3), scale_to_DPIF(3)};
#if defined(__ANDROID__)
    s.TouchExtraPadding = {3, 3};
    s.ScrollbarSize *= scale_to_DPIF(2);
#endif

    TraceLog(LOG_INFO, "dpi scale: %d", scale_to_DPII(1));

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLANK);
        rlImGuiBegin();

        ImGuiID dockspace_id = ImGui::DockSpaceOverViewport(0,
            NULL,
            ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_NoTabBar |
                ImGuiDockNodeFlags_NoWindowMenuButton);

        // ImGui::ShowDemoWindow();

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
                    dockspace_id, ImGuiDir_Right, 0.80f, &dock_game, &dock_controls);
            } else {
                ImGui::DockBuilderSplitNode(
                    dockspace_id, ImGuiDir_Up, 0.70f, &dock_game, &dock_controls);
            }

            ImGui::DockBuilderDockWindow("##game", dock_game);
            ImGui::DockBuilderDockWindow("Controlls", dock_controls);
            ImGui::DockBuilderFinish(dockspace_id);
        }

        ImGui::Begin("Controlls", nullptr);
        board_controls(state);
        ImGui::End();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::Begin("##game",
            nullptr,
            ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground |
                ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoNav);

        ImVec2 pos = ImGui::GetCursorScreenPos();
        Vector2 cursor_origin = {pos.x, pos.y};

        if (ImGui::IsWindowHovered() || state.white_fill.start != ktl::pos2_size::invalid()) {
            mouse_input(state, cursor_origin);
        }
        keyboard_input(state);

        Vector2 render_size = {ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y};
        if (render_size.x != game_tex.texture.width || render_size.y != game_tex.texture.height) {
            if (render_size.x > 0 && render_size.y > 0) {
                UnloadRenderTexture(game_tex);
                game_tex = LoadRenderTexture((int)render_size.x, (int)render_size.y);
            }
        }

        auto fit_size = std::min(render_size.x, render_size.y);
        state.cell_size = fit_size / state.game.width;
        state.offset = {
            (render_size.x - (state.cell_size * state.game.width)) / 2,
            (render_size.y - (state.cell_size * state.game.height)) / 2,
        };

        BeginTextureMode(game_tex);
        ClearBackground(BLANK);

        draw_grid(state);

        draw_measure_overlay(state, cursor_origin, render_size);
        draw_erase_overlay(state);

        if (state.solved) {
            int win_x = (render_size.x / 2) - (state.win_image.width / 2);
            int win_y = (render_size.y / 2) - (state.win_image.height / 2);

            DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.5f));
            DrawTexture(state.win_image, win_x, win_y, WHITE);
        }

        EndTextureMode();

        rlImGuiImageRenderTexture(&game_tex);
        ImGui::End();
        ImGui::PopStyleVar();

        rlImGuiEnd();

#if !defined(NDEBUG)
        DrawFPS(10, 10);
#endif

        if (state.custom_cursor) {
            HideCursor();
            auto p = GetMousePosition();
            DrawTexture(state.cursor, p.x, p.y, WHITE);
        } else {
            ShowCursor();
        }

        EndDrawing();
    }

    rlImGuiShutdown();
    for (auto& [_, font] : state.fonts) {
        UnloadFont(font);
    }
    UnloadRenderTexture(game_tex);
    UnloadTexture(state.cursor);
    UnloadTexture(state.win_image);
    CloseWindow();
}

#if defined(NDEBUG) && defined(_WIN32)
int __stdcall WinMain(void* hInstance, void* hPrevInstance, char* lpCmdLine, int nShowCmd) {
    return main();
}
#endif
