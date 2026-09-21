#include "movement.h"
#include "config.h"
#include "offsets.h"
#define NOMINMAX
#include <Windows.h>

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

void tick_movement(Memory& mem, const Config& cfg, std::uintptr_t pawn) {
    if (cfg.rage_bhop && GetAsyncKeyState(cfg.rage_bhop_key) & 0x8000)
        tick_bhop(mem, pawn);
}
