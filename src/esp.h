#pragma once
#include "memory.h"
struct Config;
void draw_esp(Memory& mem, const Camera& cam, const std::vector<Player>& players, const Config& cfg);
