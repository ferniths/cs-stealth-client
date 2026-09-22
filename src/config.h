#pragma once
#include <cstdint>
#include <string>

struct Config {
    bool esp = true;
    bool esp_box = true;
    bool esp_name = true;
    bool esp_health = true;
    bool esp_visible_only = false;
    bool esp_skeleton = false;
    bool esp_tracer = false;
    bool esp_distance = false;
    bool esp_snapline = false;
    bool esp_head_dot = false;
    bool esp_weapon = false;
    int esp_box_style = 0;
    int esp_box_r = 155;
    int esp_box_g = 95;
    int esp_box_b = 255;

    void defaults();
    void load(const std::string& path);
    void save(const std::string& path) const;
};
