#include "aimbot.h"
#include "config.h"
#include "offsets.h"
#include "imgui.h"
#define NOMINMAX
#include <Windows.h>
#include <cmath>
#include <algorithm>
#include <random>
#include <chrono>
#include <unordered_map>

static std::mt19937 aim_rng(std::random_device{}());

static bool w2s(const std::array<float, 16>& vm, float x, float y, float z,
                float& sx, float& sy, int w, int h) {
    float rx = vm[0]*x + vm[1]*y + vm[2]*z + vm[3];
    float ry = vm[4]*x + vm[5]*y + vm[6]*z + vm[7];
    float rw = vm[12]*x + vm[13]*y + vm[14]*z + vm[15];
    if (rw < 0.01f) return false;
    float iw = 1.0f / rw;
    sx = (w*0.5f) + 0.05f * rx * iw * w;
    sy = (h*0.5f) - 0.5f * ry * iw * h + 0.5f;
    return true;
}

static float get_fire_rate_ms(int weapon_id) {
    switch (weapon_id) {
    case 1:  return 450.0f;   // Deagle
    case 2:  return 100.0f;   // Dual Berettas
    case 3:  return 150.0f;   // 5-7
    case 4:  return 150.0f;   // Glock
    case 7:  return 100.0f;   // AK-47
    case 8:  return 90.0f;    // AUG
    case 9:  return 1500.0f;  // AWP
    case 10: return 90.0f;    // FAMAS
    case 11: return 1500.0f;  // G3SG1
    case 13: return 90.0f;    // Galil
    case 16: return 90.0f;    // M4A4
    case 17: return 70.0f;    // MAC-10
    case 19: return 70.0f;    // MP9
    case 23: return 80.0f;    // MP7
    case 24: return 80.0f;    // UMP
    case 26: return 800.0f;   // Nova
    case 28: return 150.0f;   // P250
    case 29: return 65.0f;    // P90
    case 33: return 800.0f;   // MAG-7
    case 38: return 90.0f;    // SCAR
    case 39: return 90.0f;    // SG553
    case 40: return 1250.0f;  // SSG
    case 42: return 400.0f;   // Knife
    case 60: return 90.0f;    // M4A1-S
    case 61: return 170.0f;   // USP
    case 63: return 100.0f;   // CZ
    case 64: return 825.0f;   // R8
    default: return 100.0f;
    }
}

static auto aim_start = std::chrono::steady_clock::now();
static auto aim_last_target = std::uintptr_t(0);
static float aim_react = 0.05f;
static auto aim_react_start = std::chrono::steady_clock::now();
static auto aim_last_shot = std::chrono::steady_clock::now();
static int aim_locked_weapon = 0;

void tick_aimbot(Memory& mem, Config& cfg, const Camera& cam,
                 const std::vector<Player>& players, std::uintptr_t local_pawn) {
    if (!cfg.aimbot) return;
    if (cfg.aim_key != 0 && !(GetAsyncKeyState(cfg.aim_key) & 0x8000)) {
        aim_last_target = 0;
        return;
    }

    auto now = std::chrono::steady_clock::now();
    float dt = std::chrono::duration<float>(now - aim_start).count();
    aim_start = now;
    if (dt < 0.001f) dt = 0.016f;
    if (dt > 0.1f) dt = 0.1f;

    float fov_px = std::tan(cfg.aim_fov * 3.14159265f / 360.0f) * (cam.h * 0.5f);
    fov_px = std::max(6.0f, fov_px);
    float cxp = cam.w * 0.5f;
    float cyp = cam.h * 0.5f;

    auto local_vel = mem.read_vec3(local_pawn + SCH::m_vecVelocity);
    float ping_ms = static_cast<float>(cam.ping);
    float bspeed = 2400.0f;

    float best_metric = 1e9f;
    float best_sx = 0, best_sy = 0;
    std::uintptr_t best_pawn = 0;

    for (const auto& p : players) {
        if (!p.alive) continue;
        if (p.team == cam.team) continue;
        if (cfg.aim_visible_only && !p.spotted) continue;

        Vec3 bone = mem.read_bone(p.pawn, cfg.aim_bone);
        float tx, ty, tz;
        if (bone.x != 0 || bone.y != 0 || bone.z != 0) {
            tx = bone.x; ty = bone.y; tz = bone.z;
        } else {
            tx = p.origin.x; ty = p.origin.y;
            tz = p.origin.z + 64.0f - 26.0f * p.duck;
        }

        if (cfg.aim_pred) {
            float dx = tx - cam.origin.x;
            float dy = ty - cam.origin.y;
            float dz = tz - cam.origin.z;
            float dist = std::sqrt(dx*dx + dy*dy + dz*dz);
            float t = dist / std::max(1.0f, bspeed) + ping_ms / 2000.0f;
            t = std::max(0.02f, std::min(0.18f, t + cfg.aim_lead / 1000.0f));
            float vx = p.vel.x - local_vel.x;
            float vy = p.vel.y - local_vel.y;
            float vz = p.vel.z - local_vel.z;
            float sp = std::sqrt(vx*vx + vy*vy + vz*vz);
            if (sp > 0.01f && sp < 2000.0f) {
                tx += vx * t; ty += vy * t; tz += vz * t;
            }
            tz += std::min(0.5f * 800.0f * t * t, 4.0f);
        }

        float sx, sy;
        if (!w2s(cam.vm, tx, ty, tz, sx, sy, cam.w, cam.h)) continue;
        float d = std::sqrt((sx-cxp)*(sx-cxp) + (sy-cyp)*(sy-cyp));
        if (d > fov_px) continue;

        float entity_dist = std::sqrt((p.origin.x-cam.origin.x)*(p.origin.x-cam.origin.x)+(p.origin.y-cam.origin.y)*(p.origin.y-cam.origin.y)+(p.origin.z-cam.origin.z)*(p.origin.z-cam.origin.z));
        float metric = d + entity_dist * 0.01f;
        if (metric < best_metric) {
            best_metric = metric;
            best_sx = sx; best_sy = sy;
            best_pawn = p.pawn;
        }
    }

    if (!cfg.aim_lock && !best_pawn) return;
    if (cfg.aim_lock && !best_pawn) {
        best_pawn = aim_last_target;
        if (!best_pawn) return;
    }

    if (cfg.aim_lock && aim_last_target) {
        bool target_alive = false;
        for (const auto& p : players) {
            if (p.pawn == aim_last_target && p.alive) {
                target_alive = true;
                break;
            }
        }
        if (!target_alive) {
            aim_last_target = 0;
            return;
        }
    }

    float d = std::sqrt((best_sx-cxp)*(best_sx-cxp) + (best_sy-cyp)*(best_sy-cyp));
    if (!cfg.aim_lock && d < 2.5f) return;

    if (best_pawn != aim_last_target) {
        aim_last_target = best_pawn;
        aim_react_start = now;
        aim_react = 0.04f + (aim_rng() % 60) / 1000.0f;
    }
    float elapsed = std::chrono::duration<float>(now - aim_react_start).count();
    if (elapsed < aim_react) return;

    if (cfg.aim_fire_rate) {
        float fire_ms = get_fire_rate_ms(aim_locked_weapon);
        float since_shot = std::chrono::duration<float>(now - aim_last_shot).count() * 1000.0f;
        if (since_shot < fire_ms) return;
    }

    if (!cfg.aim_lock && d < 4.0f) return;

    float sens = cam.sens;
    float k = (1.0f / (cam.w * 0.5f)) / (sens * 0.001221730f) * 4.0f;
    float dx = (best_sx - cxp) * k;
    float dy = (best_sy - cyp) * k;

    float smoothness = std::clamp(cfg.aim_smooth / 10.0f, 0.25f, 0.95f);
    float base = std::clamp(1.0f / cfg.aim_smooth, 0.05f, 0.9f);
    float step = std::clamp(base * 1.2f * std::min(1.0f, dt * 120.0f), 0.02f, 0.95f);
    float dist_ratio = std::min(1.0f, d / fov_px);
    float noise_amt = (0.03f + 0.04f * dist_ratio) * smoothness;

    float ndx, ndy;
    if (d < 8.0f) {
        float micro = (1.0f - d / 8.0f) * 0.6f * smoothness;
        ndx = std::normal_distribution<float>(dx * step, std::abs(dx * step) * noise_amt + micro)(aim_rng);
        ndy = std::normal_distribution<float>(dy * step, std::abs(dy * step) * noise_amt + micro)(aim_rng);
    } else {
        ndx = std::normal_distribution<float>(dx * step, std::abs(dx * step) * noise_amt + 0.01f)(aim_rng);
        ndy = std::normal_distribution<float>(dy * step, std::abs(dy * step) * noise_amt + 0.01f)(aim_rng);
    }

    mem.mouse_move(ndx, ndy);
}

void draw_fov_circle(const Camera& cam, const Config& cfg) {
    if (!cfg.aimbot || !cfg.aim_fov_circle) return;

    ImDrawList* draw = ImGui::GetBackgroundDrawList();
    ImVec2 center = ImVec2(cam.w * 0.5f, cam.h * 0.5f);
    float rad = std::tan(cfg.aim_fov * 3.14159265f / 360.0f) * (cam.h * 0.5f);
    rad = std::max(8.0f, std::min(rad, cam.h * 0.45f));
    draw->AddCircle(center, rad, IM_COL32(155, 95, 255, 120), 64);
}
