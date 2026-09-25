#include "esp.h"
#include "config.h"
#define NOMINMAX
#include "imgui.h"
#include <cmath>
#include <algorithm>
#include <unordered_map>
#include <chrono>

struct EspSmooth { float fx, fy, hx, hy; std::chrono::steady_clock::time_point t; };
static std::unordered_map<std::uintptr_t, EspSmooth> sm;

static const char* weapon_name(int id) {
    switch (id) {
    case 1: return "Deagle"; case 2: return "Dual"; case 3: return "5-7";
    case 4: return "Glock"; case 7: return "AK"; case 8: return "AUG";
    case 9: return "AWP"; case 10: return "FAMAS"; case 11: return "G3SG1";
    case 13: return "Galil"; case 16: return "M4A4"; case 17: return "MAC-10";
    case 19: return "MP9"; case 23: return "MP7"; case 24: return "UMP";
    case 26: return "Nova"; case 28: return "P250"; case 29: return "P90";
    case 33: return "MAG-7"; case 38: return "SCAR"; case 39: return "SG553";
    case 40: return "SSG"; case 42: return "Knife"; case 60: return "M4A1-S";
    case 61: return "USP"; case 63: return "CZ"; case 64: return "R8";
    default: return "";
    }
}

static bool w2s(const std::array<float, 16>& vm, const Vec3& pos, float& sx, float& sy, int w, int h) {
    float rx = vm[0]*pos.x + vm[1]*pos.y + vm[2]*pos.z + vm[3];
    float ry = vm[4]*pos.x + vm[5]*pos.y + vm[6]*pos.z + vm[7];
    float rw = vm[12]*pos.x + vm[13]*pos.y + vm[14]*pos.z + vm[15];
    if (rw < 0.01f) return false;
    float iw = 1.0f / rw;
    sx = (w*0.5f) + 0.5f * rx * iw * w + 0.5f;
    sy = (h*0.5f) - 0.5f * ry * iw * h + 0.5f;
    return true;
}

static ImU32 health_color(int hp) {
    float t = std::clamp(hp / 100.0f, 0.0f, 1.0f);
    int r = static_cast<int>(255 * (1.0f - t) + 40 * t);
    int g = static_cast<int>(60 * (1.0f - t) + 230 * t);
    int b = static_cast<int>(60 * (1.0f - t) + 100 * t);
    return IM_COL32(r, g, b, 255);
}

// Dark halo underlay so lines/text stay readable on any map brightness
static const ImU32 kHalo = IM_COL32(0, 0, 0, 160);

static void halo_rect(ImDrawList* d, ImVec2 a, ImVec2 b, ImU32 col, float rounding = 0.0f) {
    d->AddRect(a, b, kHalo, rounding, 0, 3.0f);
    d->AddRect(a, b, col, rounding, 0, 1.5f);
}

static void halo_line(ImDrawList* d, ImVec2 a, ImVec2 b, ImU32 col, float thick = 1.5f) {
    d->AddLine(a, b, kHalo, thick + 2.0f);
    d->AddLine(a, b, col, thick);
}

static void shadow_text(ImDrawList* d, ImVec2 pos, ImU32 col, const char* s) {
    d->AddText(ImVec2(pos.x + 1.0f, pos.y + 1.0f), IM_COL32(0, 0, 0, 220), s);
    d->AddText(pos, col, s);
}

void draw_esp(Memory& mem, const Camera& cam, const std::vector<Player>& players, const Config& cfg) {
    if (!cfg.esp) return;
    ImDrawList* d = ImGui::GetBackgroundDrawList();
    ImVec2 disp = ImGui::GetIO().DisplaySize;
    auto now = std::chrono::steady_clock::now();

    for (const auto& p : players) {
        if (!p.alive || p.team == cam.team) continue;

        float fsx, fsy, hsx, hsy;
        if (!w2s(cam.vm, p.origin, fsx, fsy, cam.w, cam.h)) { sm.erase(p.pawn); continue; }
        Vec3 hp = p.origin; hp.z += 72.0f - 26.0f * p.duck;
        if (!w2s(cam.vm, hp, hsx, hsy, cam.w, cam.h)) { sm.erase(p.pawn); continue; }

        auto it = sm.find(p.pawn);
        if (it != sm.end()) {
            float dt = std::min(0.05f, std::chrono::duration<float>(now - it->second.t).count());
            float a = std::min(1.0f, 40.0f * dt);
            fsx = it->second.fx + (fsx - it->second.fx) * a;
            fsy = it->second.fy + (fsy - it->second.fy) * a;
            hsx = it->second.hx + (hsx - it->second.hx) * a;
            hsy = it->second.hy + (hsy - it->second.hy) * a;
        }
        sm[p.pawn] = {fsx, fsy, hsx, hsy, now};

        if (fsx < -500 || fsx > disp.x + 500 || fsy < -500 || fsy > disp.y + 500) continue;
        float hgt = fsy - hsy;
        if (hgt < 4.0f) continue;
        float wd = hgt * 0.5f;
        float x = fsx - wd * 0.5f, y = hsy;

        ImU32 col = ImColor(cfg.esp_box_r/255.0f, cfg.esp_box_g/255.0f, cfg.esp_box_b/255.0f, 1.0f);

        if (cfg.esp_box) {
            if (cfg.esp_box_style == 0) {
                halo_rect(d, {x, y}, {x+wd, y+hgt}, col, 2.0f);
            } else if (cfg.esp_box_style == 1) {
                float c = std::max(4.0f, wd/5.0f);
                halo_line(d, {x,y}, {x+c,y}, col);   halo_line(d, {x,y}, {x,y+c}, col);
                halo_line(d, {x+wd-c,y}, {x+wd,y}, col); halo_line(d, {x+wd,y}, {x+wd,y+c}, col);
                halo_line(d, {x,y+hgt-c}, {x,y+hgt}, col); halo_line(d, {x,y+hgt}, {x+c,y+hgt}, col);
                halo_line(d, {x+wd-c,y+hgt}, {x+wd,y+hgt}, col); halo_line(d, {x+wd,y+hgt-c}, {x+wd,y+hgt}, col);
            } else {
                d->AddRectFilled({x+1,y+1},{x+wd-1,y+hgt-1},IM_COL32(cfg.esp_box_r,cfg.esp_box_g,cfg.esp_box_b,55), 2.0f);
                halo_rect(d, {x, y}, {x+wd, y+hgt}, col);
            }
        }

        if (cfg.esp_health) {
            float hp_pct = std::clamp(p.health/100.0f, 0.0f, 1.0f);
            float bx = x - 8.0f;
            d->AddRectFilled({bx-1,y-1},{bx+5,y+hgt+1}, IM_COL32(0,0,0,220)); // border
            d->AddRectFilled({bx,y},{bx+4,y+hgt},IM_COL32(25,30,40,255));
            float bh = std::max(2.0f, hgt * hp_pct);

            int segments = static_cast<int>(bh);
            if (segments < 1) segments = 1;
            float seg_h = bh / segments;
            for (int i = 0; i < segments; i++) {
                float seg_hp = hp_pct * 100.0f * (static_cast<float>(i + 1) / segments);
                ImU32 sc = health_color(static_cast<int>(seg_hp));
                float sy_start = y + hgt - (i + 1) * seg_h;
                float sy_end = y + hgt - i * seg_h;
                d->AddRectFilled({bx, sy_start}, {bx+4, sy_end}, sc);
            }
        }

        if (cfg.esp_name && !p.name.empty()) {
            ImVec2 ts = ImGui::CalcTextSize(p.name.c_str());
            float nx = std::max(2.0f, std::min(x+(wd-ts.x)*0.5f, disp.x-ts.x-12.0f));
            d->AddRectFilled({nx-3,y-ts.y-7},{nx+ts.x+3,y-1}, IM_COL32(8,10,15,215), 3.0f);
            shadow_text(d, {nx, y-ts.y-4}, IM_COL32(240,240,245,255), p.name.c_str());
        }

        if (cfg.esp_distance) {
            float dist = std::sqrt((p.origin.x-cam.origin.x)*(p.origin.x-cam.origin.x)+(p.origin.y-cam.origin.y)*(p.origin.y-cam.origin.y)+(p.origin.z-cam.origin.z)*(p.origin.z-cam.origin.z));
            char buf[32]; sprintf_s(buf, "%.0fm", dist/39.37f);
            ImVec2 ts = ImGui::CalcTextSize(buf);
            shadow_text(d, {fsx-ts.x*0.5f, fsy+4}, IM_COL32(210,210,215,255), buf);
        }

        if (cfg.esp_skeleton) {
            static constexpr int bp[][2] = {{7,6},{6,4},{4,1},{6,9},{9,10},{10,11},{6,13},{13,14},{14,15},{1,17},{17,18},{18,19},{1,20},{20,21},{21,22}};
            for (auto& [a,b] : bp) {
                Vec3 ba = mem.read_bone(p.pawn, a), bb = mem.read_bone(p.pawn, b);
                if ((ba.x==0&&ba.y==0)||(bb.x==0&&bb.y==0)) continue;
                float ax,ay,bx,by;
                if (!w2s(cam.vm,ba,ax,ay,cam.w,cam.h)||!w2s(cam.vm,bb,bx,by,cam.w,cam.h)) continue;
                halo_line(d, {ax,ay},{bx,by}, IM_COL32(245,245,248,190), 1.2f);
            }
        }

        if (cfg.esp_tracer) halo_line(d, {disp.x*0.5f,disp.y},{fsx,fsy}, IM_COL32(235,235,235,90), 1.2f);
        if (cfg.esp_snapline) halo_line(d, {disp.x*0.5f,disp.y*0.75f},{fsx,fsy}, IM_COL32(165,110,255,120), 1.2f);
        if (cfg.esp_head_dot) {
            float head_r = std::max(3.0f, hgt * 0.035f);
            d->AddCircleFilled({hsx,hsy}, head_r + 1.5f, IM_COL32(0,0,0,180));
            d->AddCircleFilled({hsx,hsy}, head_r, IM_COL32(250,250,252,220));
            d->AddCircle({hsx,hsy}, head_r, IM_COL32(165,110,255,170), 0, 1.5f);
        }

        if (cfg.esp_weapon && p.weapon_id > 0) {
            const char* wn = weapon_name(p.weapon_id);
            if (wn[0]) {
                ImVec2 ts = ImGui::CalcTextSize(wn);
                float wx = std::max(2.0f, std::min(x+(wd-ts.x)*0.5f, disp.x-ts.x-12.0f));
                d->AddRectFilled({wx-3,y+hgt+3},{wx+ts.x+3,y+hgt+3+ts.y+6}, IM_COL32(8,10,15,215), 3.0f);
                shadow_text(d, {wx, y+hgt+6}, IM_COL32(205,205,210,255), wn);
            }
        }
    }
}
