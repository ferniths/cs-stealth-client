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

    bool aimbot = false;
    float aim_fov = 3.0f;
    float aim_smooth = 6.0f;
    int aim_key = 0xA0;
    int aim_bone = 7;
    bool aim_visible_only = true;
    bool aim_pred = true;
    float aim_lead = 0.02f;

    bool rage_bhop = false;
    int rage_bhop_key = 0x06;
    bool rage_autostrafe = false;
    bool rage_counterstrafe = false;
    int rage_counterstrafe_key = 0;
    bool rage_slowwalk = false;

    void load(const std::string& path);
    void save(const std::string& path) const;
};
