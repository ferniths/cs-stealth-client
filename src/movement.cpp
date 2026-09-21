#include "movement.h"
#include "config.h"
#include "offsets.h"
#define NOMINMAX
#include <Windows.h>
#include <cmath>
#include <algorithm>

static float normalize_yaw(float yaw) {
    while (yaw > 180.0f) yaw -= 360.0f;
    while (yaw < -180.0f) yaw += 360.0f;
    return yaw;
}

// ============================================
// AUTO BHOP
// Non-blocking: press space on ground->air, release next frame.
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
// When airborne: adjusts yaw toward velocity direction
// and taps A/D in sync for max air acceleration.
// Uses mouse_move for yaw (more natural) + key taps.
// ============================================
static bool g_strafe_l = false, g_strafe_r = false;

static void tick_autostrafe(Memory& mem, std::uintptr_t pawn) {
    bool on_ground = (mem.read<std::uint32_t>(pawn + SCH::m_fFlags) & 1) != 0;
    auto vel = mem.read_vec3(pawn + SCH::m_vecVelocity);
    float speed = std::sqrt(vel.x * vel.x + vel.y * vel.y);

    // Release from last frame
    if (g_strafe_l) { mem.key_release('A'); g_strafe_l = false; }
    if (g_strafe_r) { mem.key_release('D'); g_strafe_r = false; }

    if (on_ground || speed < 50.0f) return;

    // Calculate where velocity is pointing vs where we're looking
    float vel_yaw = atan2f(vel.y, vel.x) * (180.0f / 3.14159265f);
    float view_yaw = mem.read<float>(mem.client_base + CLIENT::dwViewAngles + 4);
    float diff = normalize_yaw(vel_yaw - view_yaw);

    // Smoothly turn toward velocity direction (15% per frame)
    if (std::abs(diff) > 30.0f) {
        float adj = view_yaw + diff * 0.15f;
        mem.write<float>(mem.client_base + CLIENT::dwViewAngles + 4, normalize_yaw(adj));
    }

    // Tap strafe key in the direction that increases speed
    // Positive diff = velocity is to the right of view = press D
    // Negative diff = velocity is to the left of view = press A
    if (diff > 10.0f) { mem.key_press('D'); g_strafe_r = true; }
    else if (diff < -10.0f) { mem.key_press('A'); g_strafe_l = true; }
}

// ============================================
// COUNTER-STRAFE (NON-BLOCKING STATE MACHINE)
// When shoot key pressed while moving, taps opposite
// key to stop instantly for accurate shot.
//
// Frame 1: shoot pressed + moving → press opposite key
// Frame 2: release opposite key
// No Sleep() calls — runs across frames.
// ============================================
enum class CSState { IDLE, TAPPING, RELEASING };

static CSState g_cs_state = CSState::IDLE;
static int g_cs_opposite = 0;
static int g_cs_frames = 0;

static void tick_counterstrafe(Memory& mem, const Config& cfg) {
    if (cfg.rage_counterstrafe_key == 0) return;

    bool shoot = GetAsyncKeyState(cfg.rage_counterstrafe_key) & 0x8000;
    bool a = GetAsyncKeyState('A') & 0x8000;
    bool d = GetAsyncKeyState('D') & 0x8000;
    bool w = GetAsyncKeyState('W') & 0x8000;
    bool s = GetAsyncKeyState('S') & 0x8000;
    bool moving = a || d || w || s;

    switch (g_cs_state) {
    case CSState::IDLE:
        if (shoot && moving) {
            // Determine opposite key
            if (a) g_cs_opposite = 'D';
            else if (d) g_cs_opposite = 'A';
            else if (w) g_cs_opposite = 'S';
            else if (s) g_cs_opposite = 'W';
            else { g_cs_state = CSState::IDLE; return; }

            mem.key_press(g_cs_opposite);
            g_cs_frames = 0;
            g_cs_state = CSState::TAPPING;
        }
        break;

    case CSState::TAPPING:
        g_cs_frames++;
        if (g_cs_frames >= 3) {  // ~3 frames = ~50ms at 60fps
            mem.key_release(g_cs_opposite);
            g_cs_frames = 0;
            g_cs_state = CSState::RELEASING;
        }
        break;

    case CSState::RELEASING:
        g_cs_frames++;
        if (g_cs_frames >= 2) {  // Small cooldown
            g_cs_state = CSState::IDLE;
        }
        break;
    }
}

// ============================================
// SLOW WALK (NON-BLOCKING)
// ============================================
static bool g_sw_held = false;

static void tick_slowwalk(Memory& mem, std::uintptr_t pawn) {
    auto vel = mem.read_vec3(pawn + SCH::m_vecVelocity);
    float speed = std::sqrt(vel.x * vel.x + vel.y * vel.y);

    if (g_sw_held) { mem.key_release(VK_LSHIFT); g_sw_held = false; }

    if (speed > 10.0f) {
        mem.key_press(VK_LSHIFT);
        g_sw_held = true;
    }
}

// ============================================
// MAIN MOVEMENT TICK
// ============================================
void tick_movement(Memory& mem, const Config& cfg, std::uintptr_t pawn) {
    if (cfg.rage_bhop && GetAsyncKeyState(cfg.rage_bhop_key) & 0x8000)
        tick_bhop(mem, pawn);

    if (cfg.rage_autostrafe)
        tick_autostrafe(mem, pawn);

    if (cfg.rage_slowwalk)
        tick_slowwalk(mem, pawn);

    if (cfg.rage_counterstrafe)
        tick_counterstrafe(mem, cfg);
}
