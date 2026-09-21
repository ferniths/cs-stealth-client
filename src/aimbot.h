#pragma once
#include "memory.h"
struct Config;
void tick_aimbot(Memory& mem, Config& cfg, const Camera& cam,
                 const std::vector<Player>& players, std::uintptr_t pawn);
void draw_fov_circle(const Camera& cam, const Config& cfg);
