#include "movement.h"
#include "config.h"
#include "offsets.h"
#define NOMINMAX
#include <Windows.h>
#include <cmath>

static float normalize_yaw(float yaw) {
    while (yaw > 180.0f) yaw -= 360.0f;
    while (yaw < -180.0f) yaw += 360.0f;
    return yaw;
}

// ============================================
// AUTO BHOP
// ============================================
static bool g_bh_ground = false;
static bool g_bh_held = false;

static void tick_bhop(Memory& mem, std::uintptr_t pawn) {
    bool on_ground = (mem.read<std::uint32_t>(pawn + SCH::m_fFlags) & 1) != 0;
    if (g_bh_held) { mem.key_release(VK_SPACE); g_bh_held = false; }
    if (on_ground && !g_bh_ground) { mem.key_press(VK_SPACE); g_bh_held = true; }
    g_bh_ground = on_ground;
}

// ============================================
// AUTO-STRAFE
// ============================================
static bool g_strafe_l = false, g_strafe_r = false;

static void tick_autostrafe(Memory& mem, std::uintptr_t pawn) {
    bool on_ground = (mem.read<std::uint32_t>(pawn + SCH::m_fFlags) & 1) != 0;
    auto vel = mem.read_vec3(pawn + SCH::m_vecVelocity);
    float speed = std::sqrt(vel.x * vel.x + vel.y * vel.y);

    if (g_strafe_l) { mem.key_release('A'); g_strafe_l = false; }
    if (g_strafe_r) { mem.key_release('D'); g_strafe_r = false; }
    if (on_ground || speed < 50.0f) return;

    float vel_yaw = atan2f(vel.y, vel.x) * (180.0f / 3.14159265f);
    float view_yaw = mem.read<float>(mem.client_base + CLIENT::dwViewAngles + 4);
    float diff = normalize_yaw(vel_yaw - view_yaw);

    if (std::abs(diff) > 30.0f) {
        float adj = view_yaw + diff * 0.15f;
        mem.write<float>(mem.client_base + CLIENT::dwViewAngles + 4, normalize_yaw(adj));
    }

    if (diff > 10.0f) { mem.key_press('D'); g_strafe_r = true; }
    else if (diff < -10.0f) { mem.key_press('A'); g_strafe_l = true; }
}

// ============================================
// COUNTER-STRAFE
// ============================================
static bool g_cs_active = false;

static void tick_counterstrafe(Memory& mem, const Config& cfg) {
    if (cfg.rage_counterstrafe_key == 0) return;
    bool shoot = GetAsyncKeyState(cfg.rage_counterstrafe_key) & 0x8000;
    bool a = GetAsyncKeyState('A') & 0x8000, d = GetAsyncKeyState('D') & 0x8000;
    bool w = GetAsyncKeyState('W') & 0x8000, s = GetAsyncKeyState('S') & 0x8000;

    if (shoot && !g_cs_active) {
        g_cs_active = true;
        if (a) { mem.key_press('D'); Sleep(50); mem.key_release('D'); }
        else if (d) { mem.key_press('A'); Sleep(50); mem.key_release('A'); }
        else if (w) { mem.key_press('S'); Sleep(50); mem.key_release('S'); }
        else if (s) { mem.key_press('W'); Sleep(50); mem.key_release('W'); }
    }
    if (!shoot) g_cs_active = false;
}

// ============================================
// SLOW WALK
// ============================================
static void tick_slowwalk(Memory& mem, std::uintptr_t pawn) {
    auto vel = mem.read_vec3(pawn + SCH::m_vecVelocity);
    float speed = std::sqrt(vel.x * vel.x + vel.y * vel.y);
    if (speed > 10.0f) { mem.key_press(VK_LSHIFT); Sleep(1); mem.key_release(VK_LSHIFT); }
}

// ============================================
// MAIN MOVEMENT TICK
// ============================================
void tick_movement(Memory& mem, const Config& cfg, std::uintptr_t pawn) {
    if (cfg.rage_bhop && GetAsyncKeyState(cfg.rage_bhop_key) & 0x8000) tick_bhop(mem, pawn);
    if (cfg.rage_autostrafe) tick_autostrafe(mem, pawn);
    if (cfg.rage_slowwalk) tick_slowwalk(mem, pawn);
    if (cfg.rage_counterstrafe) tick_counterstrafe(mem, cfg);
}
