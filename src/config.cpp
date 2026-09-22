#include "config.h"
#include <fstream>
#include <json.hpp>
using json = nlohmann::json;

void Config::defaults() {
    esp = true; esp_box = true; esp_name = true; esp_health = true;
    esp_visible_only = false; esp_skeleton = false; esp_tracer = false;
    esp_distance = false; esp_snapline = false; esp_head_dot = false;
    esp_weapon = false; esp_box_style = 0;
    esp_box_r = 155; esp_box_g = 95; esp_box_b = 255;
}

void Config::load(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) return;
    json j;
    try { f >> j; } catch (...) { return; }

    if (j.contains("esp"))  esp = j["esp"].get<bool>();
    if (j.contains("esp_box"))  esp_box = j["esp_box"].get<bool>();
    if (j.contains("esp_name")) esp_name = j["esp_name"].get<bool>();
    if (j.contains("esp_health")) esp_health = j["esp_health"].get<bool>();
    if (j.contains("esp_visible_only")) esp_visible_only = j["esp_visible_only"].get<bool>();
    if (j.contains("esp_skeleton")) esp_skeleton = j["esp_skeleton"].get<bool>();
    if (j.contains("esp_tracer")) esp_tracer = j["esp_tracer"].get<bool>();
    if (j.contains("esp_distance")) esp_distance = j["esp_distance"].get<bool>();
    if (j.contains("esp_snapline")) esp_snapline = j["esp_snapline"].get<bool>();
    if (j.contains("esp_head_dot")) esp_head_dot = j["esp_head_dot"].get<bool>();
    if (j.contains("esp_weapon")) esp_weapon = j["esp_weapon"].get<bool>();
    if (j.contains("esp_box_style")) esp_box_style = j["esp_box_style"].get<int>();
    if (j.contains("esp_box_r")) esp_box_r = j["esp_box_r"].get<int>();
    if (j.contains("esp_box_g")) esp_box_g = j["esp_box_g"].get<int>();
    if (j.contains("esp_box_b")) esp_box_b = j["esp_box_b"].get<int>();
}

void Config::save(const std::string& path) const {
    json j;
    j["esp"] = esp; j["esp_box"] = esp_box; j["esp_name"] = esp_name;
    j["esp_health"] = esp_health; j["esp_visible_only"] = esp_visible_only;
    j["esp_skeleton"] = esp_skeleton; j["esp_tracer"] = esp_tracer;
    j["esp_distance"] = esp_distance; j["esp_snapline"] = esp_snapline;
    j["esp_head_dot"] = esp_head_dot; j["esp_weapon"] = esp_weapon;
    j["esp_box_style"] = esp_box_style;
    j["esp_box_r"] = esp_box_r; j["esp_box_g"] = esp_box_g; j["esp_box_b"] = esp_box_b;

    std::ofstream f(path);
    if (f.is_open()) f << j.dump(2) << std::endl;
}
