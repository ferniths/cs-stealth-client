#define NOMINMAX
#include "memory.h"
#include "offsets.h"
#include <tlhelp32.h>
#include <algorithm>
#include <chrono>

static DWORD find_pid(const wchar_t* name) {
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return 0;
    PROCESSENTRY32W pe{};
    pe.dwSize = sizeof(pe);
    if (Process32FirstW(snap, &pe)) {
        do {
            if (_wcsicmp(pe.szExeFile, name) == 0) {
                CloseHandle(snap);
                return pe.th32ProcessID;
            }
        } while (Process32NextW(snap, &pe));
    }
    CloseHandle(snap);
    return 0;
}

static std::uintptr_t find_module(HANDLE hProc, DWORD pid, const wchar_t* name) {
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
    if (snap == INVALID_HANDLE_VALUE) return 0;
    MODULEENTRY32W me{};
    me.dwSize = sizeof(me);
    if (Module32FirstW(snap, &me)) {
        do {
            if (_wcsicmp(me.szModule, name) == 0) {
                CloseHandle(snap);
                return reinterpret_cast<std::uintptr_t>(me.modBaseAddr);
            }
        } while (Module32NextW(snap, &me));
    }
    CloseHandle(snap);
    return 0;
}

bool Memory::attach() {
    pid = find_pid(L"cs2.exe");
    if (!pid) return false;
    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (!hProcess || hProcess == INVALID_HANDLE_VALUE) return false;
    client_base = find_module(hProcess, pid, L"client.dll");
    engine_base = find_module(hProcess, pid, L"engine2.dll");
    return client_base && engine_base;
}

bool Memory::alive() const {
    DWORD code = 0;
    return hProcess != INVALID_HANDLE_VALUE && GetExitCodeProcess(hProcess, &code) && code == STILL_ACTIVE;
}

std::vector<std::uint8_t> Memory::read_bytes(std::uintptr_t addr, size_t size) const {
    std::vector<std::uint8_t> buf(size);
    SIZE_T read = 0;
    ReadProcessMemory(hProcess, reinterpret_cast<LPCVOID>(addr), buf.data(), size, &read);
    buf.resize(read);
    return buf;
}

std::string Memory::read_string(std::uintptr_t addr, size_t max_len) const {
    std::string s;
    s.reserve(max_len);
    for (size_t i = 0; i < max_len; ++i) {
        char c = read<char>(addr + i);
        if (c == 0) break;
        s += c;
    }
    return s;
}

std::uintptr_t Memory::read_ptr(std::uintptr_t addr) const {
    return read<std::uintptr_t>(addr);
}

Vec3 Memory::read_vec3(std::uintptr_t addr) const {
    Vec3 v;
    v.x = read<float>(addr);
    v.y = read<float>(addr + 4);
    v.z = read<float>(addr + 8);
    return v;
}

std::uintptr_t Memory::entity_from_index(std::uintptr_t es, std::uintptr_t idx) const {
    if (idx <= 0) return 0;
    auto chunk = idx >> 9;
    auto slot = idx & 0x1FF;
    auto chunk_ptr = read_ptr(es + CLIENT::dwChunkPointers + 8 * chunk);
    if (!chunk_ptr) return 0;
    return read_ptr(chunk_ptr + CLIENT::dwSlotStride * slot);
}

std::uintptr_t Memory::entity_from_handle(std::uintptr_t h) const {
    auto idx = h & 0x7FFF;
    if (idx == 0 || idx == 0x7FFF) return 0;
    auto es = read_ptr(client_base + CLIENT::dwGameEntitySystem);
    if (!es) return 0;
    return entity_from_index(es, idx);
}

Vec3 Memory::read_bone(std::uintptr_t pawn, int bone_id) const {
    auto sn = read_ptr(pawn + SCH::m_pGameSceneNode);
    if (!sn) return {};
    auto bm = read_ptr(sn + SCH::m_modelState + 0x80);
    if (!bm) return {};
    return read_vec3(bm + bone_id * 32);
}

std::pair<std::uintptr_t, std::uintptr_t> Memory::read_local() const {
    auto ctrl = read_ptr(client_base + CLIENT::dwLocalPlayerController);
    if (!ctrl) return {0, 0};
    auto hpawn = read<std::uint32_t>(ctrl + SCH::m_hPlayerPawn);
    if (!hpawn) return {ctrl, 0};
    return {ctrl, entity_from_handle(hpawn)};
}

Player Memory::read_player(std::uintptr_t es, std::uintptr_t idx) const {
    Player p;
    auto pawn = entity_from_index(es, idx);
    if (!pawn) return p;

    p.index = static_cast<int>(idx);
    p.pawn = pawn;
    p.health = read<std::int32_t>(pawn + SCH::m_iHealth);
    auto life = read<std::uint8_t>(pawn + SCH::m_lifeState);
    p.team = read<std::uint8_t>(pawn + SCH::m_iTeamNum);
    if (p.team != 2 && p.team != 3) return {};
    p.alive = (life == 0);
    p.dormant = read<std::uint8_t>(pawn + 0x8) != 0;
    p.spotted = read<std::uint8_t>(pawn + SCH::m_entitySpottedState + SCH::m_bSpotted) != 0;
    auto flags = read<std::uint32_t>(pawn + SCH::m_fFlags);

    auto sn = read_ptr(pawn + SCH::m_pGameSceneNode);
    if (sn) {
        auto origin_sn = read_vec3(sn + SCH::m_vecAbsOrigin);
        if (origin_sn.x != 0 || origin_sn.y != 0 || origin_sn.z != 0) {
            p.origin = origin_sn;
        } else {
            auto bm = read_ptr(sn + SCH::m_modelState + 0x80);
            if (bm) p.origin = read_vec3(bm + 0x140);
        }
    }

    if (flags & 0x06) {
        auto mov = read_ptr(pawn + SCH::m_pMovementServices);
        if (mov) p.duck = std::clamp(read<float>(mov + SCH::m_flDuckAmount), 0.0f, 1.0f);
    }

    auto hctrl = read<std::uint32_t>(pawn + SCH::m_hOriginalController);
    if (hctrl && (hctrl & 0x7FFF) != 0 && (hctrl & 0x7FFF) != 0x7FFF) {
        auto ctrl = entity_from_handle(hctrl);
        if (ctrl) {
            auto nptr = read_ptr(ctrl + SCH::m_sSanitizedPlayerName);
            if (nptr) p.name = read_string(nptr, 31);
        }
    }

    auto wsv = read_ptr(pawn + SCH::m_pWeaponServices);
    if (wsv) {
        auto hw = read<std::uint32_t>(wsv + SCH::m_hActiveWeapon);
        if (hw && (hw & 0x7FFF) != 0 && (hw & 0x7FFF) != 0x7FFF) {
            auto wpn = entity_from_handle(hw);
            if (wpn) {
                auto iv = wpn + SCH::m_AttributeManager + SCH::m_Item;
                p.weapon_id = read<std::int16_t>(iv + SCH::m_iItemDefinitionIndex);
            }
        }
    }

    return p;
}

std::vector<Player> Memory::read_players(std::uintptr_t es) const {
    std::vector<Player> out;
    auto highest = read<std::uint32_t>(client_base + CLIENT::dwHighestEntityIndex);
    if (!highest) highest = 64;
    highest = std::min(highest, 1024u);
    for (std::uintptr_t i = 1; i <= highest; ++i) {
        auto p = read_player(es, i);
        if (p.pawn && p.health > 0 && !p.dormant) out.push_back(p);
    }
    return out;
}

Camera Memory::read_camera() const {
    Camera c;
    auto [ctrl, pawn] = read_local();
    if (!pawn) return c;

    auto sn = read_ptr(pawn + SCH::m_pGameSceneNode);
    if (sn) {
        auto origin_sn = read_vec3(sn + SCH::m_vecAbsOrigin);
        if (origin_sn.x != 0 || origin_sn.y != 0 || origin_sn.z != 0) {
            c.origin = origin_sn;
        } else {
            auto bm = read_ptr(sn + SCH::m_modelState + 0x80);
            if (bm) c.origin = read_vec3(bm + 0x140);
        }
    }

    c.pitch = read<float>(client_base + CLIENT::dwViewAngles);
    c.yaw = read<float>(client_base + CLIENT::dwViewAngles + 4);
    c.w = read<std::int32_t>(engine_base + ENGINE2::dwWindowWidth);
    c.h = read<std::int32_t>(engine_base + ENGINE2::dwWindowHeight);
    c.team = read<std::uint8_t>(pawn + SCH::m_iTeamNum);
    c.vm = read_view_matrix();
    return c;
}

std::array<float, 16> Memory::read_view_matrix() const {
    auto bytes = read_bytes(client_base + CLIENT::dwViewMatrix, 64);
    std::array<float, 16> vm{};
    if (bytes.size() == 64) memcpy(vm.data(), bytes.data(), 64);
    return vm;
}
