#include "gui.h"
#include "config.h"
#define NOMINMAX
#include <Windows.h>
#include "imgui.h"

static bool toggle(const char* label, bool* v) {
    bool changed = ImGui::Checkbox(label, v);
    ImGui::SameLine(ImGui::GetContentRegionAvail().x - 10.0f);
    ImGui::TextColored(ImVec4(155/255.0f, 95/255.0f, 255/255.0f, 1.0f), "%s", *v ? "ON" : "OFF");
    return changed;
}

static void section_header(const char* text) {
    ImGui::Spacing();
    ImGui::TextColored(ImVec4(155/255.0f, 95/255.0f, 255/255.0f, 1.0f), "%s", text);
    ImGui::Separator();
}

void gui_init() {
    ImGui::GetStyle().WindowRounding = 6.0f;
    ImGui::GetStyle().FrameRounding = 4.0f;
    ImGui::GetStyle().GrabRounding = 4.0f;
    ImGui::GetStyle().Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.04f, 0.10f, 0.95f);
    ImGui::GetStyle().Colors[ImGuiCol_Border] = ImVec4(0.30f, 0.20f, 0.50f, 0.80f);
    ImGui::GetStyle().Colors[ImGuiCol_TitleBg] = ImVec4(0.08f, 0.05f, 0.15f, 1.0f);
    ImGui::GetStyle().Colors[ImGuiCol_TitleBgActive] = ImVec4(0.12f, 0.08f, 0.22f, 1.0f);
    ImGui::GetStyle().Colors[ImGuiCol_Button] = ImVec4(0.18f, 0.12f, 0.30f, 1.0f);
    ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered] = ImVec4(0.28f, 0.18f, 0.48f, 1.0f);
    ImGui::GetStyle().Colors[ImGuiCol_ButtonActive] = ImVec4(0.38f, 0.22f, 0.62f, 1.0f);
    ImGui::GetStyle().Colors[ImGuiCol_SliderGrab] = ImVec4(0.60f, 0.38f, 1.0f, 1.0f);
    ImGui::GetStyle().Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.75f, 0.50f, 1.0f, 1.0f);
    ImGui::GetStyle().Colors[ImGuiCol_FrameBg] = ImVec4(0.10f, 0.07f, 0.18f, 1.0f);
    ImGui::GetStyle().Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.15f, 0.10f, 0.25f, 1.0f);
    ImGui::GetStyle().Colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.05f, 0.12f, 0.96f);
    ImGui::GetStyle().Colors[ImGuiCol_Header] = ImVec4(0.20f, 0.13f, 0.35f, 1.0f);
    ImGui::GetStyle().Colors[ImGuiCol_HeaderHovered] = ImVec4(0.30f, 0.20f, 0.50f, 1.0f);
    ImGui::GetStyle().Colors[ImGuiCol_HeaderActive] = ImVec4(0.40f, 0.25f, 0.65f, 1.0f);
}

void gui_draw(Config& cfg, bool& menu_open) {
    ImGui::SetNextWindowSize(ImVec2(320, 480), ImGuiCond_Once);
    ImGui::SetNextWindowPos(ImVec2(50, 50), ImGuiCond_Once);
    ImGui::Begin("Stealth Client", &menu_open, ImGuiWindowFlags_NoCollapse);

    ImGui::TextColored(ImVec4(155/255.0f, 95/255.0f, 255/255.0f, 1.0f), "Stealth Client v1.0");
    ImGui::Separator();

    if (ImGui::BeginTabBar("##tabs")) {
        // ===== ESP TAB =====
        if (ImGui::BeginTabItem("ESP")) {
            toggle("Enable ESP", &cfg.esp);
            if (cfg.esp) {
                section_header("Visuals");
                toggle("Box", &cfg.esp_box);
                toggle("Name", &cfg.esp_name);
                toggle("Health", &cfg.esp_health);
                toggle("Skeleton", &cfg.esp_skeleton);
                toggle("Head Dot", &cfg.esp_head_dot);
                toggle("Weapon", &cfg.esp_weapon);
                toggle("Distance", &cfg.esp_distance);
                toggle("Snapline", &cfg.esp_snapline);
                toggle("Tracer", &cfg.esp_tracer);

                section_header("Style");
                const char* styles[] = { "Full", "Corner", "Filled" };
                int s = cfg.esp_box_style;
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                if (ImGui::Combo("Box Style", &s, styles, 3)) cfg.esp_box_style = s;
                float col[3] = { cfg.esp_box_r/255.0f, cfg.esp_box_g/255.0f, cfg.esp_box_b/255.0f };
                if (ImGui::ColorEdit3("Box Color", col)) {
                    cfg.esp_box_r = static_cast<int>(col[0] * 255);
                    cfg.esp_box_g = static_cast<int>(col[1] * 255);
                    cfg.esp_box_b = static_cast<int>(col[2] * 255);
                }
            }
            ImGui::EndTabItem();
        }

        // ===== AIMBOT TAB =====
        if (ImGui::BeginTabItem("Aimbot")) {
            toggle("Enable Aimbot", &cfg.aimbot);
            if (cfg.aimbot) {
                section_header("Targeting");
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

                section_header("Behavior");
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                ImGui::SliderFloat("FOV", &cfg.aim_fov, 0.5f, 20.0f, "%.1f deg");
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                ImGui::SliderFloat("Smooth", &cfg.aim_smooth, 1.0f, 30.0f, "%.1f");
                toggle("Prediction", &cfg.aim_pred);
                if (cfg.aim_pred) {
                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                    ImGui::SliderFloat("Lead", &cfg.aim_lead, 0.0f, 0.1f, "%.3f s");
                }
            }
            ImGui::EndTabItem();
        }

        // ===== MOVEMENT TAB =====
        if (ImGui::BeginTabItem("Movement")) {
            section_header("Bunny Hop");
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

            section_header("Overlay");
            toggle("FOV Circle", &cfg.aim_fov_circle);
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}
