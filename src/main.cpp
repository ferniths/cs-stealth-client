#include "overlay.h"
#include "gui.h"
#include "config.h"
#include "memory.h"
#include "offsets.h"
#include "esp.h"
#include "aimbot.h"
#include "movement.h"
#define NOMINMAX
#include <Windows.h>
#include "imgui.h"
#include <string>

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

    bool running = true;

    while (running) {
        if (GetAsyncKeyState(VK_DELETE) & 1) { running = false; break; }
        if (GetAsyncKeyState(VK_INSERT) & 1) overlay.set_open(!overlay.is_open());

        if (!mem.alive()) {
            overlay.begin_frame();
            if (overlay.is_open()) gui_draw(cfg, overlay.menu_open);
            overlay.end_frame();
            Sleep(1000);
            if (!mem.attach()) { Sleep(100); continue; }
            continue;
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

        tick_aimbot(mem, cfg, cam, players, pawn);
        tick_movement(mem, cfg, pawn);

        overlay.begin_frame();
        if (overlay.is_open()) gui_draw(cfg, overlay.menu_open);
        draw_esp(mem, cam, players, cfg);
        draw_fov_circle(cam, cfg);

        {
            ImDrawList* dl = ImGui::GetBackgroundDrawList();
            ImVec2 d = ImGui::GetIO().DisplaySize;
            dl->AddRectFilled({8, d.y-22}, {152, d.y-4}, IM_COL32(14,10,28,200));
            dl->AddText({12, d.y-20}, IM_COL32(155,95,255,255), "Stealth Client v1.0");
        }

        overlay.end_frame();
        cfg.save("config.json");
        Sleep(1);
    }

    overlay.destroy();
    return 0;
}
