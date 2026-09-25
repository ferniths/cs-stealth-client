#include "overlay.h"
#include "gui.h"
#include "config.h"
#include "memory.h"
#include "offsets.h"
#include "esp.h"
#define NOMINMAX
#include <Windows.h>
#include "imgui.h"
#include <string>
#include <chrono>

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    Overlay overlay;
    if (!overlay.create()) {
        MessageBoxW(nullptr, L"Failed to create overlay", L"Stealth Client", MB_ICONERROR);
        return 1;
    }

    Memory mem;
    Config cfg;
    cfg.load("config.json");
    gui_init();

    bool downloaded = resolve_offsets();

    bool running = true;
    auto last_save = std::chrono::steady_clock::now();
    bool offsets_resolved = downloaded;

    while (running) {
        if (GetAsyncKeyState(VK_DELETE) & 1) { running = false; break; }
        if (GetAsyncKeyState(VK_INSERT) & 1) overlay.set_open(!overlay.is_open());

        if (!mem.alive()) {
            overlay.begin_frame();
            if (overlay.is_open()) gui_draw(cfg, overlay.menu_open);
            overlay.end_frame();
            Sleep(1000);
            if (!mem.attach()) { Sleep(100); continue; }
            // Re-resolve on every attach: CS2 may have updated while we were detached
            downloaded = resolve_offsets();
            offsets_resolved = downloaded;
            continue;
        }

        if (!offsets_resolved) {
            if (resolve_offsets_runtime(mem.hProcess, mem.client_base, mem.engine_base))
                downloaded = true;
            offsets_resolved = true;
        }

        HWND cs_hwnd = FindWindowW(nullptr, L"Counter-Strike 2");
        if (!cs_hwnd) {
            overlay.begin_frame();
            if (overlay.is_open()) gui_draw(cfg, overlay.menu_open);
            overlay.end_frame();
            Sleep(500); continue;
        }

        RECT cs_rect{};
        if (!GetClientRect(cs_hwnd, &cs_rect)) {
            overlay.begin_frame();
            if (overlay.is_open()) gui_draw(cfg, overlay.menu_open);
            overlay.end_frame();
            Sleep(100); continue;
        }

        POINT cs_pos{ cs_rect.left, cs_rect.top };
        ClientToScreen(cs_hwnd, &cs_pos);
        int cs_w = cs_rect.right - cs_rect.left;
        int cs_h = cs_rect.bottom - cs_rect.top;
        if (cs_w < 100 || cs_h < 100) { Sleep(100); continue; }

        MoveWindow(overlay.hwnd, cs_pos.x, cs_pos.y, cs_w, cs_h, TRUE);
        overlay.width = cs_w; overlay.height = cs_h;

        auto [ctrl, pawn] = mem.read_local();
        if (!pawn) {
            overlay.begin_frame();
            if (overlay.is_open()) gui_draw(cfg, overlay.menu_open);
            overlay.end_frame();
            Sleep(100); continue;
        }

        auto es = mem.read_ptr(mem.client_base + CLIENT::dwGameEntitySystem);
        if (!es) {
            overlay.begin_frame();
            if (overlay.is_open()) gui_draw(cfg, overlay.menu_open);
            overlay.end_frame();
            Sleep(100); continue;
        }

        auto players = mem.read_players(es);
        auto cam = mem.read_camera();
        if (cam.w < 100 || cam.h < 100) {
            overlay.begin_frame();
            if (overlay.is_open()) gui_draw(cfg, overlay.menu_open);
            overlay.end_frame();
            Sleep(100); continue;
        }

        overlay.begin_frame();
        if (overlay.is_open()) gui_draw(cfg, overlay.menu_open);
        draw_esp(mem, cam, players, cfg);

        {
            ImDrawList* dl = ImGui::GetBackgroundDrawList();
            ImVec2 d = ImGui::GetIO().DisplaySize;
            dl->AddRectFilled({8, d.y-22}, {168, d.y-4}, IM_COL32(14,10,28,200));
            dl->AddText({12, d.y-20}, IM_COL32(155,95,255,255), "Stealth Client v1.0");
            dl->AddText({d.x - 120, d.y-20}, IM_COL32(120,120,120,255), "[Insert] Menu");
        }

        overlay.end_frame();

        auto now = std::chrono::steady_clock::now();
        float save_dt = std::chrono::duration<float>(now - last_save).count();
        if (save_dt > 2.0f) {
            cfg.save("config.json");
            last_save = now;
        }

        Sleep(1);
    }

    overlay.destroy();
    return 0;
}
