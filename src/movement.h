#pragma once
#include "memory.h"
struct Config;
struct Camera;

void tick_movement(Memory& mem, const Config& cfg, std::uintptr_t pawn);
