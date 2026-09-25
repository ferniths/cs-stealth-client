#define NOMINMAX
#include "offsets.h"
#include <Windows.h>
#include <winhttp.h>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <json.hpp>
#include <cstring>
#include <algorithm>

#ifndef WINHTTP_ACCESS_TYPE_PRECONFIG
#define WINHTTP_ACCESS_TYPE_PRECONFIG 1
#endif

using json = nlohmann::json;

// ─── Hardcoded fallback (last known good: 2026-09-25) ──────────────────────
namespace CLIENT {
    std::uintptr_t dwLocalPlayerController = 0x2535598;
    std::uintptr_t dwLocalPlayerPawn       = 0x255E658;
    std::uintptr_t dwGameEntitySystem       = 0x27130E8;
    std::uintptr_t dwViewAngles             = 0x25741C8;
    std::uintptr_t dwViewMatrix             = 0x25639A0;
    std::uintptr_t dwHighestEntityIndex     = 0x2120;
    std::uintptr_t dwChunkPointers          = 0x10;
    std::uintptr_t dwSlotStride             = 0x70;
}
namespace ENGINE2 {
    std::uintptr_t dwWindowWidth            = 0x91F540;
    std::uintptr_t dwWindowHeight           = 0x91F544;
}
namespace SCH {
    std::uintptr_t m_pGameSceneNode         = 0x330;
    std::uintptr_t m_iHealth                = 0x34C;
    std::uintptr_t m_vecVelocity            = 0x430;
    std::uintptr_t m_lifeState              = 0x354;
    std::uintptr_t m_iTeamNum               = 0x3E7;
    std::uintptr_t m_fFlags                 = 0x3F4;
    std::uintptr_t m_vecAbsOrigin           = 0xC8;
    std::uintptr_t m_flDuckAmount           = 0x40C;
    std::uintptr_t m_pMovementServices      = 0x1330;
    std::uintptr_t m_hOriginalController    = 0x1560;
    std::uintptr_t m_entitySpottedState     = 0x1E88;
    std::uintptr_t m_bSpotted               = 0x8;
    std::uintptr_t m_hPlayerPawn            = 0x92C;
    std::uintptr_t m_sSanitizedPlayerName   = 0x878;
    std::uintptr_t m_pWeaponServices        = 0x12F0;
    std::uintptr_t m_hActiveWeapon          = 0x60;
    std::uintptr_t m_AttributeManager       = 0x1290;
    std::uintptr_t m_Item                   = 0x50;
    std::uintptr_t m_iItemDefinitionIndex   = 0x1BA;
    std::uintptr_t m_modelState             = 0x140;
    std::uintptr_t m_iPing                  = 0x838;
    std::uintptr_t m_pCameraServices        = 0x1328;
    std::uintptr_t m_vecCsViewPunchAngle    = 0x48;
    std::uintptr_t m_nButtons               = 0x50;
    std::uintptr_t m_bDormant               = 0x103;
}

// ─── HTTP fetch (WinHTTP) ───────────────────────────────────────────────────
static bool http_get(const wchar_t* url, std::string& out) {
    URL_COMPONENTS uc{};
    uc.dwStructSize = sizeof(uc);
    wchar_t host[256]{}, path[2048]{};
    uc.lpszHostName = host; uc.dwHostNameLength = 255;
    uc.lpszUrlPath = path;  uc.dwUrlPathLength = 2047;
    if (!WinHttpCrackUrl(url, 0, 0, &uc)) return false;

    HINTERNET hInt = WinHttpOpen(L"StealthClient/1.0", WINHTTP_ACCESS_TYPE_PRECONFIG,
                                 WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hInt) return false;
    HINTERNET hConn = WinHttpConnect(hInt, host, uc.nPort, 0);
    if (!hConn) { WinHttpCloseHandle(hInt); return false; }
    DWORD flags = (uc.nScheme == INTERNET_SCHEME_HTTPS) ? WINHTTP_FLAG_SECURE : 0;
    HINTERNET hReq = WinHttpOpenRequest(hConn, L"GET", path, nullptr, WINHTTP_NO_REFERER,
                                        WINHTTP_DEFAULT_ACCEPT_TYPES, flags);
    if (!hReq) { WinHttpCloseHandle(hConn); WinHttpCloseHandle(hInt); return false; }

    BOOL ok = WinHttpSendRequest(hReq, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                                 WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
    if (ok) ok = WinHttpReceiveResponse(hReq, nullptr);
    if (ok) {
        char buf[8192];
        DWORD read = 0, avail = 0;
        out.clear();
        while (WinHttpQueryDataAvailable(hReq, &avail) && avail > 0) {
            DWORD got = 0;
            if (!WinHttpReadData(hReq, buf, (avail < sizeof(buf)) ? avail : sizeof(buf), &got)) break;
            out.append(buf, got);
            if (got < avail) break;
        }
    }
    WinHttpCloseHandle(hReq);
    WinHttpCloseHandle(hConn);
    WinHttpCloseHandle(hInt);
    return ok && !out.empty();
}

// ─── Cache ──────────────────────────────────────────────────────────────────
static const char* CACHE_PATH = "offsets_cache.json";

static json dump_current() {
    json j;
    j["client"]["dwLocalPlayerController"] = CLIENT::dwLocalPlayerController;
    j["client"]["dwLocalPlayerPawn"]       = CLIENT::dwLocalPlayerPawn;
    j["client"]["dwGameEntitySystem"]       = CLIENT::dwGameEntitySystem;
    j["client"]["dwViewAngles"]             = CLIENT::dwViewAngles;
    j["client"]["dwViewMatrix"]             = CLIENT::dwViewMatrix;
    j["client"]["dwHighestEntityIndex"]     = CLIENT::dwHighestEntityIndex;
    j["engine2"]["dwWindowWidth"]           = ENGINE2::dwWindowWidth;
    j["engine2"]["dwWindowHeight"]          = ENGINE2::dwWindowHeight;
    j["sch"]["m_pGameSceneNode"]            = SCH::m_pGameSceneNode;
    j["sch"]["m_iHealth"]                   = SCH::m_iHealth;
    j["sch"]["m_vecVelocity"]               = SCH::m_vecVelocity;
    j["sch"]["m_lifeState"]                 = SCH::m_lifeState;
    j["sch"]["m_iTeamNum"]                  = SCH::m_iTeamNum;
    j["sch"]["m_fFlags"]                    = SCH::m_fFlags;
    j["sch"]["m_vecAbsOrigin"]              = SCH::m_vecAbsOrigin;
    j["sch"]["m_flDuckAmount"]              = SCH::m_flDuckAmount;
    j["sch"]["m_pMovementServices"]         = SCH::m_pMovementServices;
    j["sch"]["m_hOriginalController"]       = SCH::m_hOriginalController;
    j["sch"]["m_entitySpottedState"]        = SCH::m_entitySpottedState;
    j["sch"]["m_bSpotted"]                  = SCH::m_bSpotted;
    j["sch"]["m_hPlayerPawn"]               = SCH::m_hPlayerPawn;
    j["sch"]["m_sSanitizedPlayerName"]      = SCH::m_sSanitizedPlayerName;
    j["sch"]["m_pWeaponServices"]           = SCH::m_pWeaponServices;
    j["sch"]["m_hActiveWeapon"]             = SCH::m_hActiveWeapon;
    j["sch"]["m_AttributeManager"]          = SCH::m_AttributeManager;
    j["sch"]["m_Item"]                      = SCH::m_Item;
    j["sch"]["m_iItemDefinitionIndex"]      = SCH::m_iItemDefinitionIndex;
    j["sch"]["m_modelState"]                = SCH::m_modelState;
    j["sch"]["m_iPing"]                     = SCH::m_iPing;
    j["sch"]["m_pCameraServices"]           = SCH::m_pCameraServices;
    j["sch"]["m_vecCsViewPunchAngle"]       = SCH::m_vecCsViewPunchAngle;
    j["sch"]["m_nButtons"]                  = SCH::m_nButtons;
    j["sch"]["m_bDormant"]                  = SCH::m_bDormant;
    return j;
}

static bool load_cache() {
    std::ifstream f(CACHE_PATH);
    if (!f.is_open()) return false;
    json j;
    try { f >> j; } catch (...) { return false; }
    if (!j.contains("client") || !j.contains("sch")) return false;

    auto& c = j["client"];
    if (c.contains("dwLocalPlayerController")) CLIENT::dwLocalPlayerController = c["dwLocalPlayerController"];
    if (c.contains("dwLocalPlayerPawn"))       CLIENT::dwLocalPlayerPawn       = c["dwLocalPlayerPawn"];
    if (c.contains("dwGameEntitySystem"))       CLIENT::dwGameEntitySystem       = c["dwGameEntitySystem"];
    if (c.contains("dwViewAngles"))             CLIENT::dwViewAngles             = c["dwViewAngles"];
    if (c.contains("dwViewMatrix"))             CLIENT::dwViewMatrix             = c["dwViewMatrix"];
    if (c.contains("dwHighestEntityIndex"))     CLIENT::dwHighestEntityIndex     = c["dwHighestEntityIndex"];

    auto& e = j["engine2"];
    if (e.contains("dwWindowWidth"))  ENGINE2::dwWindowWidth  = e["dwWindowWidth"];
    if (e.contains("dwWindowHeight")) ENGINE2::dwWindowHeight = e["dwWindowHeight"];

    auto& s = j["sch"];
    auto set = [&](const char* k, std::uintptr_t& dst) { if (s.contains(k)) dst = s[k]; };
    set("m_pGameSceneNode", SCH::m_pGameSceneNode);
    set("m_iHealth", SCH::m_iHealth);
    set("m_vecVelocity", SCH::m_vecVelocity);
    set("m_lifeState", SCH::m_lifeState);
    set("m_iTeamNum", SCH::m_iTeamNum);
    set("m_fFlags", SCH::m_fFlags);
    set("m_vecAbsOrigin", SCH::m_vecAbsOrigin);
    set("m_flDuckAmount", SCH::m_flDuckAmount);
    set("m_pMovementServices", SCH::m_pMovementServices);
    set("m_hOriginalController", SCH::m_hOriginalController);
    set("m_entitySpottedState", SCH::m_entitySpottedState);
    set("m_bSpotted", SCH::m_bSpotted);
    set("m_hPlayerPawn", SCH::m_hPlayerPawn);
    set("m_sSanitizedPlayerName", SCH::m_sSanitizedPlayerName);
    set("m_pWeaponServices", SCH::m_pWeaponServices);
    set("m_hActiveWeapon", SCH::m_hActiveWeapon);
    set("m_AttributeManager", SCH::m_AttributeManager);
    set("m_Item", SCH::m_Item);
    set("m_iItemDefinitionIndex", SCH::m_iItemDefinitionIndex);
    set("m_modelState", SCH::m_modelState);
    set("m_iPing", SCH::m_iPing);
    set("m_pCameraServices", SCH::m_pCameraServices);
    set("m_vecCsViewPunchAngle", SCH::m_vecCsViewPunchAngle);
    set("m_nButtons", SCH::m_nButtons);
    set("m_bDormant", SCH::m_bDormant);
    return true;
}

static void save_cache() {
    std::ofstream f(CACHE_PATH);
    if (f.is_open()) f << dump_current().dump(2);
}

// ─── Apply downloaded offsets.json (a2x/cs2-dumper format) ──────────────────
static bool apply_offsets_json(const std::string& body) {
    json j;
    try {
        j = json::parse(body);
        if (!j.contains("client.dll") || !j.contains("engine2.dll")) return false;

        auto& c = j["client.dll"];
        if (!c.contains("dwLocalPlayerController") || !c.contains("dwViewMatrix")) return false;

        CLIENT::dwLocalPlayerController = c["dwLocalPlayerController"];
        CLIENT::dwViewMatrix             = c["dwViewMatrix"];
        if (c.contains("dwViewAngles"))   CLIENT::dwViewAngles = c["dwViewAngles"];
        if (c.contains("dwLocalPlayerPawn")) CLIENT::dwLocalPlayerPawn = c["dwLocalPlayerPawn"];
        if (c.contains("dwGameEntitySystem")) CLIENT::dwGameEntitySystem = c["dwGameEntitySystem"];
        else if (c.contains("dwEntityList"))  CLIENT::dwGameEntitySystem = c["dwEntityList"];
        if (c.contains("dwGameEntitySystem_highestEntityIndex"))
            CLIENT::dwHighestEntityIndex = c["dwGameEntitySystem_highestEntityIndex"];

        auto& e = j["engine2.dll"];
        if (e.contains("dwWindowWidth"))  ENGINE2::dwWindowWidth  = e["dwWindowWidth"];
        if (e.contains("dwWindowHeight")) ENGINE2::dwWindowHeight = e["dwWindowHeight"];
        return true;
    } catch (...) {
        return false;
    }
}

// ─── Apply downloaded client_dll.json (schema classes) ─────────────────────
static bool apply_schema_json(const std::string& body) {
    json j;
    try { j = json::parse(body); } catch (...) { return false; }
    if (!j.contains("client.dll") || !j["client.dll"].contains("classes")) return false;

    auto& classes = j["client.dll"]["classes"];
    auto get = [&](const char* cls, const char* field) -> std::uintptr_t {
        try {
            if (classes.contains(cls) && classes[cls].contains("fields") && classes[cls]["fields"].contains(field))
                return classes[cls]["fields"][field].get<std::uintptr_t>();
        } catch (...) {}
        return 0;
    };

    struct { const char* cls; const char* field; std::uintptr_t* dst; } map[] = {
        {"C_BaseEntity",            "m_pGameSceneNode",       &SCH::m_pGameSceneNode},
        {"C_BaseEntity",            "m_iHealth",              &SCH::m_iHealth},
        {"C_BaseEntity",            "m_vecVelocity",          &SCH::m_vecVelocity},
        {"C_BaseEntity",            "m_lifeState",            &SCH::m_lifeState},
        {"C_BaseEntity",            "m_iTeamNum",             &SCH::m_iTeamNum},
        {"C_BaseEntity",            "m_fFlags",               &SCH::m_fFlags},
        {"CGameSceneNode",          "m_vecAbsOrigin",         &SCH::m_vecAbsOrigin},
        {"CCSPlayer_MovementServices","m_flDuckAmount",       &SCH::m_flDuckAmount},
        {"C_BasePlayerPawn",        "m_pMovementServices",    &SCH::m_pMovementServices},
        {"C_CSPlayerPawnBase",      "m_hOriginalController",  &SCH::m_hOriginalController},
        {"C_CSPlayerPawn",          "m_entitySpottedState",   &SCH::m_entitySpottedState},
        {"EntitySpottedState_t",    "m_bSpotted",             &SCH::m_bSpotted},
        {"CCSPlayerController",     "m_hPlayerPawn",          &SCH::m_hPlayerPawn},
        {"CCSPlayerController",     "m_sSanitizedPlayerName", &SCH::m_sSanitizedPlayerName},
        {"CCSPlayerController",     "m_iPing",                &SCH::m_iPing},
        {"C_BasePlayerPawn",        "m_pWeaponServices",      &SCH::m_pWeaponServices},
        {"C_BasePlayerPawn",        "m_pCameraServices",      &SCH::m_pCameraServices},
        {"CPlayer_WeaponServices",  "m_hActiveWeapon",        &SCH::m_hActiveWeapon},
        {"C_EconEntity",            "m_AttributeManager",     &SCH::m_AttributeManager},
        {"C_AttributeContainer",    "m_Item",                 &SCH::m_Item},
        {"C_EconItemView",          "m_iItemDefinitionIndex", &SCH::m_iItemDefinitionIndex},
        {"CSkeletonInstance",       "m_modelState",           &SCH::m_modelState},
        {"CPlayer_CameraServices",  "m_vecCsViewPunchAngle",  &SCH::m_vecCsViewPunchAngle},
        {"CPlayer_MovementServices","m_nButtons",             &SCH::m_nButtons},
        {"CGameSceneNode",          "m_bDormant",             &SCH::m_bDormant},
    };

    // Stage first: only commit if most fields resolved, so a renamed class
    // can't leave a mix of old and new schema values behind.
    std::vector<std::uintptr_t> vals(sizeof(map) / sizeof(map[0]));
    int found = 0;
    for (size_t i = 0; i < vals.size(); ++i) {
        vals[i] = get(map[i].cls, map[i].field);
        if (vals[i] != 0) ++found;
    }
    if (found < 15) return false; // most must resolve

    for (size_t i = 0; i < vals.size(); ++i)
        if (vals[i] != 0) *map[i].dst = vals[i];
    return true;
}

// ─── Pattern scanner (fallback if download fails) ───────────────────────────
struct Pat { const char* hex; int disp_off; int instr_len; };

static std::vector<uint8_t> parse_pattern(const char* s, std::vector<int>& mask) {
    std::vector<uint8_t> bytes;
    mask.clear();
    while (*s) {
        if (*s == ' ') { ++s; continue; }
        if (s[0] == '?' && (s[1] == '?' || s[1] == '\0' || s[1] == ' ')) {
            bytes.push_back(0); mask.push_back(0);
            s += (s[1] == '?') ? 2 : 1;
        } else {
            auto hex = [](char c) -> int {
                if (c >= '0' && c <= '9') return c - '0';
                if (c >= 'a' && c <= 'f') return c - 'a' + 10;
                if (c >= 'A' && c <= 'F') return c - 'A' + 10;
                return -1;
            };
            int hi = hex(s[0]), lo = hex(s[1]);
            if (hi < 0 || lo < 0) break;
            bytes.push_back((uint8_t)((hi << 4) | lo));
            mask.push_back(1);
            s += 2;
        }
    }
    return bytes;
}

static long find_pattern(const uint8_t* data, size_t size, const char* pat) {
    std::vector<int> mask;
    auto bytes = parse_pattern(pat, mask);
    if (bytes.empty() || bytes.size() > size) return -1;
    for (size_t i = 0; i <= size - bytes.size(); ++i) {
        bool ok = true;
        for (size_t j = 0; j < bytes.size(); ++j) {
            if (mask[j] && data[i + j] != bytes[j]) { ok = false; break; }
        }
        if (ok) return (long)i;
    }
    return -1;
}

static std::uintptr_t scan_module(HANDLE hProc, std::uintptr_t base, size_t size, const char* pat, int disp_off, int instr_len) {
    std::vector<uint8_t> buf(size);
    SIZE_T read = 0;
    if (!ReadProcessMemory(hProc, (LPCVOID)base, buf.data(), size, &read)) return 0;
    if (read < 16) return 0;

    long pos = find_pattern(buf.data(), read, pat);
    if (pos < 0) return 0;

    int32_t disp = 0;
    memcpy(&disp, buf.data() + pos + disp_off, 4);
    return (std::uintptr_t)(pos + instr_len + disp);
}

// ─── Pattern-scan client.dll + engine2.dll for base offsets ────────────────
static bool pattern_scan_base(HANDLE hProc, std::uintptr_t client, std::uintptr_t engine, size_t client_sz, size_t engine_sz) {
    bool any = false;

    // dwLocalPlayerController: 48 8B 05 [disp32] 41 89 BE
    auto v = scan_module(hProc, client, client_sz, "48 8B 05 ?? ?? ?? ?? 41 89 BE", 3, 7);
    if (v) { CLIENT::dwLocalPlayerController = v; any = true; }

    // dwGameEntitySystem: 48 8B 1D [disp32] 48 89 1D [disp32] 4C 63 B3
    v = scan_module(hProc, client, client_sz, "48 8B 1D ?? ?? ?? ?? 48 89 1D ?? ?? ?? ?? 4C 63 B3", 3, 7);
    if (v) { CLIENT::dwGameEntitySystem = v; any = true; }

    // dwViewMatrix: 48 8D 0D [disp32] 48 C1 E0 06
    v = scan_module(hProc, client, client_sz, "48 8D 0D ?? ?? ?? ?? 48 C1 E0 06", 3, 7);
    if (v) { CLIENT::dwViewMatrix = v; any = true; }

    // dwViewAngles via dwCSGOInput callback pattern: f2 42 0f 10 84 28 [disp32]
    // First find dwCSGOInput, then secondary — but simpler: direct pattern
    v = scan_module(hProc, client, client_sz, "F2 42 0F 10 84 28 ?? ?? ?? ??", 5, 9);
    // This pattern is relative to dwCSGOInput — skip if not found cleanly

    // dwWindowWidth: 8B 05 [disp32] 89 07
    v = scan_module(hProc, engine, engine_sz, "8B 05 ?? ?? ?? ?? 89 07", 2, 6);
    if (v) { ENGINE2::dwWindowWidth = v; any = true; }

    // dwWindowHeight: 8B 05 [disp32] 89 03
    v = scan_module(hProc, engine, engine_sz, "8B 05 ?? ?? ?? ?? 89 03", 2, 6);
    if (v) { ENGINE2::dwWindowHeight = v; any = true; }

    // dwHighestEntityIndex: FF 81 [disp32] 48 85 D2 — disp is direct offset not RIP-rel
    {
        std::vector<int> mask;
        auto bytes = parse_pattern("FF 81 ?? ?? ?? ?? 48 85 D2", mask);
        std::vector<uint8_t> buf(client_sz);
        SIZE_T rd = 0;
        if (ReadProcessMemory(hProc, (LPCVOID)client, buf.data(), client_sz, &rd)) {
            long pos = find_pattern(buf.data(), rd, "FF 81 ?? ?? ?? ?? 48 85 D2");
            if (pos >= 0) {
                uint32_t off = 0;
                memcpy(&off, buf.data() + pos + 2, 4);
                CLIENT::dwHighestEntityIndex = off;
                any = true;
            }
        }
    }

    return any;
}

// ─── Schema field discovery via schema system pattern walk ──────────────────
// If download fails, we can't reliably walk SchemaSystem without more reverse-engineering.
// The hardcoded SCH:: values serve as fallback here.

// ─── Public API ─────────────────────────────────────────────────────────────
static const char* g_source = "builtin";
const char* offsets_source() { return g_source; }

bool resolve_offsets() {
    // 1. Try download from a2x/cs2-dumper (freshest, survives CS2 updates)
    std::string offsets_body, schema_body;
    bool got_offsets = http_get(L"https://raw.githubusercontent.com/a2x/cs2-dumper/main/output/offsets.json", offsets_body);
    bool got_schema  = http_get(L"https://raw.githubusercontent.com/a2x/cs2-dumper/main/output/client_dll.json", schema_body);

    bool off_ok  = got_offsets && apply_offsets_json(offsets_body);
    bool sch_ok  = got_schema  && apply_schema_json(schema_body);

    if (off_ok || sch_ok) {
        g_source = (off_ok && sch_ok) ? "github" : "github (partial)";
        save_cache();
        return off_ok; // base offsets fresh -> no pattern scan needed
    }

    // 2. Download failed — try cache (may be stale after a CS2 update,
    //    so return false and let the caller pattern-scan after attach)
    if (load_cache()) {
        g_source = "cache";
        return false;
    }

    // 3. Everything failed — hardcoded fallback already loaded
    g_source = "builtin";
    return false;
}

bool resolve_offsets_runtime(void* hProc, std::uintptr_t client_base, std::uintptr_t engine_base) {
    // Called after Memory::attach() — pattern-scan as live fallback
    HANDLE h = (HANDLE)hProc;
    if (!h || h == INVALID_HANDLE_VALUE) return false;

    // Get module sizes via virtual query
    MEMORY_BASIC_INFORMATION mbi{};
    size_t client_sz = 0, engine_sz = 0;
    if (VirtualQueryEx(h, (LPCVOID)client_base, &mbi, sizeof(mbi)))
        client_sz = mbi.RegionSize;
    if (VirtualQueryEx(h, (LPCVOID)engine_base, &mbi, sizeof(mbi)))
        engine_sz = mbi.RegionSize;

    // Cap to reasonable size (first 32MB should cover .text+.data)
    if (client_sz > 0x2000000) client_sz = 0x2000000;
    if (engine_sz > 0x2000000) engine_sz = 0x2000000;
    if (!client_sz || !engine_sz) return false;

    if (!pattern_scan_base(h, client_base, engine_base, client_sz, engine_sz))
        return false;

    g_source = "pattern scan";
    save_cache();
    return true;
}
