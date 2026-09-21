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
            if (cfg.esp_box_style == 0) d->AddRect(ImVec2(x, y), ImVec2(x+wd, y+hgt), col);
            else if (cfg.esp_box_style == 1) {
                float c = std::max(4.0f, wd/5.0f);
                d->AddLine({x,y},{x+c,y},col,1.5f); d->AddLine({x,y},{x,y+c},col,1.5f);
                d->AddLine({x+wd-c,y},{x+wd,y},col,1.5f); d->AddLine({x+wd,y},{x+wd,y+c},col,1.5f);
                d->AddLine({x,y+hgt-c},{x,y+hgt},col,1.5f); d->AddLine({x,y+hgt},{x+c,y+hgt},col,1.5f);
                d->AddLine({x+wd-c,y+hgt},{x+wd,y+hgt},col,1.5f); d->AddLine({x+wd,y+hgt-c},{x+wd,y+hgt},col,1.5f);
            } else {
                d->AddRectFilled({x+1,y+1},{x+wd-1,y+hgt-1},IM_COL32(cfg.esp_box_r,cfg.esp_box_g,cfg.esp_box_b,50));
                d->AddRect({x,y},{x+wd,y+hgt},col);
            }
        }

        if (cfg.esp_health) {
            float hp_pct = std::clamp(p.health/100.0f, 0.0f, 1.0f);
            float bx = x - 7.0f;
            d->AddRectFilled({bx,y},{bx+3,y+hgt},IM_COL32(25,30,40,255));
            float bh = std::max(2.0f, hgt * hp_pct);
            ImU32 hc = p.health > 60 ? IM_COL32(110,220,150,255) : p.health > 30 ? IM_COL32(240,200,90,255) : IM_COL32(255,90,90,255);
            d->AddRectFilled({bx, y+hgt-bh},{bx+3,y+hgt},hc);
        }

        if (cfg.esp_name && !p.name.empty()) {
            ImVec2 ts = ImGui::CalcTextSize(p.name.c_str());
            float nx = std::max(2.0f, std::min(x+(wd-ts.x)*0.5f, disp.x-ts.x-12.0f));
            d->AddRectFilled({nx-2,y-ts.y-6},{nx+ts.x+2,y-2},IM_COL32(10,13,18,200));
            d->AddText({nx,y-ts.y-4},IM_COL32(255,255,255,255),p.name.c_str());
        }

        if (cfg.esp_distance) {
            float dist = std::sqrt((p.origin.x-cam.origin.x)*(p.origin.x-cam.origin.x)+(p.origin.y-cam.origin.y)*(p.origin.y-cam.origin.y)+(p.origin.z-cam.origin.z)*(p.origin.z-cam.origin.z));
            char buf[32]; sprintf_s(buf, "%.0fm", dist/39.37f);
            ImVec2 ts = ImGui::CalcTextSize(buf);
            d->AddText({fsx-ts.x*0.5f, fsy+4}, IM_COL32(200,200,200,255), buf);
        }

        if (cfg.esp_skeleton) {
            static constexpr int bp[][2] = {{7,6},{6,4},{4,1},{6,9},{9,10},{10,11},{6,13},{13,14},{14,15},{1,17},{17,18},{18,19},{1,20},{20,21},{21,22}};
            for (auto& [a,b] : bp) {
                Vec3 ba = mem.read_bone(p.pawn, a), bb = mem.read_bone(p.pawn, b);
                if ((ba.x==0&&ba.y==0)||(bb.x==0&&bb.y==0)) continue;
                float ax,ay,bx,by;
                if (!w2s(cam.vm,ba,ax,ay,cam.w,cam.h)||!w2s(cam.vm,bb,bx,by,cam.w,cam.h)) continue;
                d->AddLine({ax,ay},{bx,by},IM_COL32(255,255,255,180),1.0f);
            }
        }

        if (cfg.esp_tracer) d->AddLine({disp.x*0.5f,disp.y},{fsx,fsy},IM_COL32(255,255,255,80),1.0f);
        if (cfg.esp_snapline) d->AddLine({disp.x*0.5f,disp.y*0.75f},{fsx,fsy},IM_COL32(155,95,255,100),1.0f);
        if (cfg.esp_head_dot) d->AddCircleFilled({hsx,hsy},3.0f,IM_COL32(255,255,255,200));

        if (cfg.esp_weapon && p.weapon_id > 0) {
            const char* wn = weapon_name(p.weapon_id);
            if (wn[0]) {
                ImVec2 ts = ImGui::CalcTextSize(wn);
                float wx = std::max(2.0f, std::min(x+(wd-ts.x)*0.5f, disp.x-ts.x-12.0f));
                d->AddRectFilled({wx-2,y+hgt+2},{wx+ts.x+2,y+hgt+2+ts.y+4},IM_COL32(10,13,18,200));
                d->AddText({wx,y+hgt+4},IM_COL32(200,200,200,255),wn);
            }
        }
    }
}
