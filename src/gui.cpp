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
    ImGui::SetNextWindowSize(ImVec2(320, 420), ImGuiCond_Once);
    ImGui::SetNextWindowPos(ImVec2(50, 50), ImGuiCond_Once);
    ImGui::Begin("Stealth Client", &menu_open, ImGuiWindowFlags_NoCollapse);

    ImGui::TextColored(ImVec4(155/255.0f, 95/255.0f, 255/255.0f, 1.0f), "Stealth Client v1.0");
    ImGui::Separator();

    if (ImGui::BeginTabBar("##tabs")) {
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
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                float col[3] = { cfg.esp_box_r/255.0f, cfg.esp_box_g/255.0f, cfg.esp_box_b/255.0f };
                if (ImGui::ColorEdit3("Box Color", col)) {
                    cfg.esp_box_r = static_cast<int>(col[0] * 255);
                    cfg.esp_box_g = static_cast<int>(col[1] * 255);
                    cfg.esp_box_b = static_cast<int>(col[2] * 255);
                }
            }
            ImGui::EndTabItem();
        }

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

            section_header("Air");
            toggle("Auto-Strafe", &cfg.rage_autostrafe);
            toggle("Slow Walk", &cfg.rage_slowwalk);

            section_header("Counter-Strafe");
            toggle("Counter-Strafe", &cfg.rage_counterstrafe);
            if (cfg.rage_counterstrafe) {
                const char* cs_keys[] = { "None", "LMB", "RMouse", "LAlt", "LCtrl", "LShift" };
                int csi = 0;
                if (cfg.rage_counterstrafe_key == VK_LBUTTON) csi = 1;
                else if (cfg.rage_counterstrafe_key == VK_RBUTTON) csi = 2;
                else if (cfg.rage_counterstrafe_key == VK_LMENU) csi = 3;
                else if (cfg.rage_counterstrafe_key == VK_LCONTROL) csi = 4;
                else if (cfg.rage_counterstrafe_key == VK_LSHIFT) csi = 5;
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                if (ImGui::Combo("Shoot Key", &csi, cs_keys, 6)) {
                    switch (csi) {
                    case 1: cfg.rage_counterstrafe_key = VK_LBUTTON; break;
                    case 2: cfg.rage_counterstrafe_key = VK_RBUTTON; break;
                    case 3: cfg.rage_counterstrafe_key = VK_LMENU; break;
                    case 4: cfg.rage_counterstrafe_key = VK_LCONTROL; break;
                    case 5: cfg.rage_counterstrafe_key = VK_LSHIFT; break;
                    default: cfg.rage_counterstrafe_key = 0; break;
                    }
                }
            }
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::End();
}
