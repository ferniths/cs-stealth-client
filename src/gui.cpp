#include "gui.h"
#include "config.h"
#define NOMINMAX
#include <Windows.h>
#include "imgui.h"

static bool toggle(const char* label, bool* v) {
    bool changed = ImGui::Checkbox(label, v);
    ImGui::SameLine(ImGui::GetContentRegionAvail().x - 30.0f);
    ImGui::TextColored(*v ? ImVec4(0.61f, 0.37f, 1.0f, 1.0f) : ImVec4(0.55f, 0.55f, 0.63f, 1.0f),
        "%s", *v ? "ON" : "OFF");
    return changed;
}

static void section(const char* text) {
    ImGui::Spacing();
    ImGui::TextColored(ImVec4(0.61f, 0.37f, 1.0f, 1.0f), "%s", text);
    ImVec2 p = ImGui::GetCursorScreenPos();
    float w = ImGui::GetContentRegionAvail().x;
    ImGui::GetWindowDrawList()->AddLine(p, {p.x + w, p.y}, IM_COL32(155, 95, 255, 50), 1.0f);
    ImGui::Separator();
    ImGui::Spacing();
}

static void esp_preview(const Config& cfg) {
    section("Preview");
    ImVec2 p = ImGui::GetCursorScreenPos();
    float w = ImGui::GetContentRegionAvail().x;
    float h = 100.0f;
    ImDrawList* dl = ImGui::GetWindowDrawList();

    dl->AddRectFilled(p, {p.x+w, p.y+h}, IM_COL32(8, 6, 16, 255), 4.0f);
    dl->AddRect({p.x,p.y},{p.x+w,p.y+h}, IM_COL32(40, 30, 65, 255), 4.0f);

    float cx = p.x + w * 0.5f;
    float cy = p.y + h * 0.5f;
    float bw = 40.0f, bh = 65.0f;
    float bx = cx - bw * 0.5f;
    float by = cy - bh * 0.5f + 2.0f;

    ImU32 col = IM_COL32(cfg.esp_box_r, cfg.esp_box_g, cfg.esp_box_b, 255);
    if (cfg.esp_box) {
        if (cfg.esp_box_style == 0)
            dl->AddRect({bx,by},{bx+bw,by+bh}, col, 0, 0, 1.5f);
        else if (cfg.esp_box_style == 1) {
            float c = 8.0f;
            dl->AddLine({bx,by},{bx+c,by},col,2.0f); dl->AddLine({bx,by},{bx,by+c},col,2.0f);
            dl->AddLine({bx+bw-c,by},{bx+bw,by},col,2.0f); dl->AddLine({bx+bw,by},{bx+bw,by+c},col,2.0f);
            dl->AddLine({bx,by+bh-c},{bx,by+bh},col,2.0f); dl->AddLine({bx,by+bh},{bx+c,by+bh},col,2.0f);
            dl->AddLine({bx+bw-c,by+bh},{bx+bw,by+bh},col,2.0f); dl->AddLine({bx+bw,by+bh-c},{bx+bw,by+bh},col,2.0f);
        } else {
            dl->AddRectFilled({bx+1,by+1},{bx+bw-1,by+bh-1}, IM_COL32(cfg.esp_box_r,cfg.esp_box_g,cfg.esp_box_b,40), 2.0f);
            dl->AddRect({bx,by},{bx+bw,by+bh}, col, 0, 0, 1.5f);
        }
    }
    if (cfg.esp_health) {
        float hx = bx - 5.0f;
        dl->AddRectFilled({hx,by},{hx+3,by+bh},IM_COL32(20,18,30,255), 1.0f);
        float hp_h = bh * 0.75f;
        dl->AddRectFilled({hx,by+bh-hp_h},{hx+3,by+bh},IM_COL32(40,230,100,255), 1.0f);
    }
    if (cfg.esp_name) {
        dl->AddRectFilled({bx-2,by-15},{bx+48,by-2}, IM_COL32(8,6,16,200), 2.0f);
        dl->AddText({bx,by-13}, IM_COL32(255,255,255,220), "Player");
    }
    if (cfg.esp_weapon) {
        dl->AddRectFilled({bx-2,by+bh+3},{bx+28,by+bh+15}, IM_COL32(8,6,16,200), 2.0f);
        dl->AddText({bx,by+bh+4}, IM_COL32(180,180,200,220), "AK");
    }
    if (cfg.esp_distance) {
        ImVec2 ts = ImGui::CalcTextSize("25m");
        dl->AddText({cx-ts.x*0.5f, by+bh+17}, IM_COL32(160,160,180,200), "25m");
    }
    if (cfg.esp_head_dot) {
        dl->AddCircleFilled({cx, by - 1.0f}, 3.0f, IM_COL32(255,255,255,200));
        dl->AddCircle({cx, by - 1.0f}, 3.0f, IM_COL32(155,95,255,140), 0, 1.0f);
    }

    ImGui::Dummy({0, h + 4});
}

void gui_init() {
    auto& s = ImGui::GetStyle();
    s.WindowRounding = 6.0f;
    s.FrameRounding = 4.0f;
    s.GrabRounding = 4.0f;
    s.TabRounding = 5.0f;
    s.FramePadding = ImVec2(10, 4);
    s.ItemSpacing = ImVec2(8, 5);
    s.WindowPadding = ImVec2(12, 10);
    s.ScrollbarSize = 5.0f;
    s.ChildRounding = 4.0f;

    auto& c = s.Colors;
    c[ImGuiCol_WindowBg] = ImVec4(0.055f, 0.035f, 0.09f, 0.97f);
    c[ImGuiCol_Border] = ImVec4(0.18f, 0.12f, 0.30f, 0.50f);
    c[ImGuiCol_ChildBg] = ImVec4(0, 0, 0, 0);
    c[ImGuiCol_TitleBg] = ImVec4(0.06f, 0.04f, 0.10f, 1.0f);
    c[ImGuiCol_TitleBgActive] = ImVec4(0.09f, 0.06f, 0.16f, 1.0f);
    c[ImGuiCol_Button] = ImVec4(0.14f, 0.10f, 0.24f, 1.0f);
    c[ImGuiCol_ButtonHovered] = ImVec4(0.22f, 0.15f, 0.38f, 1.0f);
    c[ImGuiCol_ButtonActive] = ImVec4(0.30f, 0.20f, 0.50f, 1.0f);
    c[ImGuiCol_SliderGrab] = ImVec4(0.61f, 0.37f, 1.0f, 1.0f);
    c[ImGuiCol_SliderGrabActive] = ImVec4(0.80f, 0.55f, 1.0f, 1.0f);
    c[ImGuiCol_FrameBg] = ImVec4(0.08f, 0.06f, 0.14f, 1.0f);
    c[ImGuiCol_FrameBgHovered] = ImVec4(0.12f, 0.08f, 0.20f, 1.0f);
    c[ImGuiCol_FrameBgActive] = ImVec4(0.16f, 0.11f, 0.28f, 1.0f);
    c[ImGuiCol_PopupBg] = ImVec4(0.06f, 0.04f, 0.10f, 0.98f);
    c[ImGuiCol_Header] = ImVec4(0.16f, 0.10f, 0.28f, 1.0f);
    c[ImGuiCol_HeaderHovered] = ImVec4(0.24f, 0.16f, 0.42f, 1.0f);
    c[ImGuiCol_HeaderActive] = ImVec4(0.32f, 0.22f, 0.55f, 1.0f);
    c[ImGuiCol_Tab] = ImVec4(0.10f, 0.07f, 0.16f, 1.0f);
    c[ImGuiCol_TabHovered] = ImVec4(0.20f, 0.13f, 0.34f, 1.0f);
    c[ImGuiCol_TabSelected] = ImVec4(0.22f, 0.14f, 0.38f, 1.0f);
    c[ImGuiCol_Separator] = ImVec4(0.15f, 0.10f, 0.25f, 0.40f);
    c[ImGuiCol_Text] = ImVec4(0.86f, 0.86f, 0.90f, 1.0f);
}

void gui_draw(Config& cfg, bool& menu_open) {
    ImGui::SetNextWindowSize(ImVec2(310, 480), ImGuiCond_Once);
    ImGui::SetNextWindowPos(ImVec2(50, 50), ImGuiCond_Once);

    ImGui::Begin("Stealth Client", &menu_open, ImGuiWindowFlags_NoCollapse);

    ImGui::TextColored(ImVec4(0.61f, 0.37f, 1.0f, 1.0f), "Stealth Client");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.55f, 0.55f, 0.63f, 1.0f), "v1.0");
    ImGui::SameLine(ImGui::GetContentRegionAvail().x - 44);
    if (ImGui::SmallButton("Reset")) cfg.defaults();
    ImGui::Separator();

    if (ImGui::BeginTabBar("##tabs")) {
        if (ImGui::BeginTabItem("  ESP  ")) {
            toggle("Enable ESP", &cfg.esp);
            if (cfg.esp) {
                section("Elements");
                toggle("Box", &cfg.esp_box);
                toggle("Name", &cfg.esp_name);
                toggle("Health Bar", &cfg.esp_health);
                toggle("Head Dot", &cfg.esp_head_dot);
                toggle("Skeleton", &cfg.esp_skeleton);
                toggle("Weapon", &cfg.esp_weapon);
                toggle("Distance", &cfg.esp_distance);
                toggle("Snapline", &cfg.esp_snapline);

                section("Style");
                const char* styles[] = { "Full", "Corner", "Filled" };
                int s = cfg.esp_box_style;
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                if (ImGui::Combo("Box Style", &s, styles, 3)) cfg.esp_box_style = s;
                float col[3] = { cfg.esp_box_r/255.0f, cfg.esp_box_g/255.0f, cfg.esp_box_b/255.0f };
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                if (ImGui::ColorEdit3("Box Color", col, ImGuiColorEditFlags_NoInputs)) {
                    cfg.esp_box_r = static_cast<int>(col[0] * 255);
                    cfg.esp_box_g = static_cast<int>(col[1] * 255);
                    cfg.esp_box_b = static_cast<int>(col[2] * 255);
                }

                esp_preview(cfg);
            }
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("  Aim  ")) {
            toggle("Enable Aimbot", &cfg.aimbot);
            if (cfg.aimbot) {
                section("Target");
                const char* keys[] = { "None", "RMouse", "LAlt", "LCtrl", "LShift", "Mouse4", "Mouse5" };
                int ki = 0;
                if (cfg.aim_key == VK_RBUTTON) ki = 1;
                else if (cfg.aim_key == VK_LMENU) ki = 2;
                else if (cfg.aim_key == VK_LCONTROL) ki = 3;
                else if (cfg.aim_key == VK_LSHIFT) ki = 4;
                else if (cfg.aim_key == 0x06) ki = 5;
                else if (cfg.aim_key == 0x07) ki = 6;
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                if (ImGui::Combo("Keybind", &ki, keys, 7)) {
                    switch (ki) {
                    case 1: cfg.aim_key = VK_RBUTTON; break;
                    case 2: cfg.aim_key = VK_LMENU; break;
                    case 3: cfg.aim_key = VK_LCONTROL; break;
                    case 4: cfg.aim_key = VK_LSHIFT; break;
                    case 5: cfg.aim_key = 0x06; break;
                    case 6: cfg.aim_key = 0x07; break;
                    default: cfg.aim_key = 0; break;
                    }
                }

                const char* bones[] = { "Head", "Neck", "Chest", "Stomach" };
                int bi = 0;
                if (cfg.aim_bone == 7) bi = 0;
                else if (cfg.aim_bone == 6) bi = 1;
                else if (cfg.aim_bone == 4) bi = 2;
                else if (cfg.aim_bone == 3) bi = 3;
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                if (ImGui::Combo("Bone", &bi, bones, 4)) {
                    switch (bi) {
                    case 0: cfg.aim_bone = 7; break;
                    case 1: cfg.aim_bone = 6; break;
                    case 2: cfg.aim_bone = 4; break;
                    case 3: cfg.aim_bone = 3; break;
                    }
                }

                toggle("Visible Only", &cfg.aim_visible_only);
                toggle("Aim Lock", &cfg.aim_lock);

                section("Behavior");
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                ImGui::SliderFloat("FOV", &cfg.aim_fov, 0.5f, 20.0f, "%.1f deg");
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                ImGui::SliderFloat("Smooth", &cfg.aim_smooth, 1.0f, 30.0f, "%.1f");
                toggle("Prediction", &cfg.aim_pred);
                if (cfg.aim_pred) {
                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                    ImGui::SliderFloat("Lead", &cfg.aim_lead, 0.0f, 0.1f, "%.3f s");
                }
                toggle("Fire Rate Limit", &cfg.aim_fire_rate);
            }
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("  Misc  ")) {
            section("Bunny Hop");
            toggle("Auto Bhop", &cfg.rage_bhop);
            if (cfg.rage_bhop) {
                const char* keys[] = { "None", "Mouse4", "Mouse5", "LCtrl", "LAlt", "Space", "V", "B" };
                int bki = 0;
                if (cfg.rage_bhop_key == 0x06) bki = 1;
                else if (cfg.rage_bhop_key == 0x07) bki = 2;
                else if (cfg.rage_bhop_key == VK_LCONTROL) bki = 3;
                else if (cfg.rage_bhop_key == VK_LMENU) bki = 4;
                else if (cfg.rage_bhop_key == VK_SPACE) bki = 5;
                else if (cfg.rage_bhop_key == 'V') bki = 6;
                else if (cfg.rage_bhop_key == 'B') bki = 7;
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                if (ImGui::Combo("Bhop Key", &bki, keys, 8)) {
                    switch (bki) {
                    case 1: cfg.rage_bhop_key = 0x06; break; case 2: cfg.rage_bhop_key = 0x07; break;
                    case 3: cfg.rage_bhop_key = VK_LCONTROL; break; case 4: cfg.rage_bhop_key = VK_LMENU; break;
                    case 5: cfg.rage_bhop_key = VK_SPACE; break; case 6: cfg.rage_bhop_key = 'V'; break;
                    case 7: cfg.rage_bhop_key = 'B'; break; default: cfg.rage_bhop_key = 0; break;
                    }
                }
            }

            section("Overlay");
            toggle("FOV Circle", &cfg.aim_fov_circle);

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}
