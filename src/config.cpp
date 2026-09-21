#include "config.h"
#include <fstream>
#include <json.hpp>
using json = nlohmann::json;

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

    if (j.contains("aimbot")) aimbot = j["aimbot"].get<bool>();
    if (j.contains("aim_fov")) aim_fov = j["aim_fov"].get<float>();
    if (j.contains("aim_smooth")) aim_smooth = j["aim_smooth"].get<float>();
    if (j.contains("aim_key")) aim_key = j["aim_key"].get<int>();
    if (j.contains("aim_bone")) aim_bone = j["aim_bone"].get<int>();
    if (j.contains("aim_visible_only")) aim_visible_only = j["aim_visible_only"].get<bool>();
    if (j.contains("aim_pred")) aim_pred = j["aim_pred"].get<bool>();
    if (j.contains("aim_lead")) aim_lead = j["aim_lead"].get<float>();

    if (j.contains("rage_bhop")) rage_bhop = j["rage_bhop"].get<bool>();
    if (j.contains("rage_bhop_key")) rage_bhop_key = j["rage_bhop_key"].get<int>();
    if (j.contains("rage_autostrafe")) rage_autostrafe = j["rage_autostrafe"].get<bool>();
    if (j.contains("rage_counterstrafe")) rage_counterstrafe = j["rage_counterstrafe"].get<bool>();
    if (j.contains("rage_counterstrafe_key")) rage_counterstrafe_key = j["rage_counterstrafe_key"].get<int>();
    if (j.contains("rage_slowwalk")) rage_slowwalk = j["rage_slowwalk"].get<bool>();
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

    j["aimbot"] = aimbot; j["aim_fov"] = aim_fov; j["aim_smooth"] = aim_smooth;
    j["aim_key"] = aim_key; j["aim_bone"] = aim_bone;
    j["aim_visible_only"] = aim_visible_only; j["aim_pred"] = aim_pred;
    j["aim_lead"] = aim_lead;

    j["rage_bhop"] = rage_bhop; j["rage_bhop_key"] = rage_bhop_key;
    j["rage_autostrafe"] = rage_autostrafe;
    j["rage_counterstrafe"] = rage_counterstrafe;
    j["rage_counterstrafe_key"] = rage_counterstrafe_key;
    j["rage_slowwalk"] = rage_slowwalk;

    std::ofstream f(path);
    if (f.is_open()) f << j.dump(2) << std::endl;
}
