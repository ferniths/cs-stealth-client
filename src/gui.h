#pragma once
#include <cstdint>
struct Config;
void gui_init();
void gui_draw(Config& cfg, bool& menu_open);

// Live read-chain diagnostics, filled by main loop, shown in menu footer.
struct HudInfo {
    std::uintptr_t ctrl = 0;   // local controller ptr
    std::uintptr_t pawn = 0;   // local pawn ptr
    std::uintptr_t es = 0;     // entity system ptr
    unsigned highest = 0;      // raw dwHighestEntityIndex value
    int pawns = 0;             // entity slots with non-null pawn
    int players = 0;           // players after filters (what ESP draws)
    int cam_w = 0, cam_h = 0;
    int local_hp = 0, local_team = 0;
    float vm0 = 0.0f;          // view matrix [0] sanity value
};
extern HudInfo hud;
