#pragma once
#define NOMINMAX
#include <Windows.h>
#include <string>
#include <cstdint>
#include <vector>
#include <array>

struct Vec3 { float x = 0, y = 0, z = 0; };

struct Player {
    int index = 0;
    std::uintptr_t pawn = 0;
    int health = 0;
    bool alive = false;
    bool dormant = false;
    int team = 0;
    bool spotted = false;
    float duck = 0.0f;
    Vec3 origin{};
    int weapon_id = 0;
    std::string name;
};

struct Camera {
    Vec3 origin;
    float pitch = 0, yaw = 0;
    int w = 0, h = 0;
    int team = 0;
    std::array<float, 16> vm{};
};

class Memory {
public:
    bool attach();
    bool alive() const;
    template<typename T>
    T read(std::uintptr_t addr) const {
        T val{};
        ReadProcessMemory(hProcess, reinterpret_cast<LPCVOID>(addr), &val, sizeof(T), nullptr);
        return val;
    }

    std::vector<std::uint8_t> read_bytes(std::uintptr_t addr, size_t size) const;
    std::string read_string(std::uintptr_t addr, size_t max_len = 32) const;
    std::uintptr_t read_ptr(std::uintptr_t addr) const;
    Vec3 read_vec3(std::uintptr_t addr) const;
    Vec3 read_bone(std::uintptr_t pawn, int bone_id) const;

    std::uintptr_t entity_from_index(std::uintptr_t es, std::uintptr_t idx) const;
    std::uintptr_t entity_from_handle(std::uintptr_t h) const;
    std::pair<std::uintptr_t, std::uintptr_t> read_local() const;
    Player read_player(std::uintptr_t pawn, std::uintptr_t idx) const;
    std::vector<Player> read_players(std::uintptr_t es, int* out_pawns = nullptr) const;
    Camera read_camera() const;
    std::array<float, 16> read_view_matrix() const;

    HANDLE hProcess = INVALID_HANDLE_VALUE;
    DWORD pid = 0;
    std::uintptr_t client_base = 0;
    std::uintptr_t engine_base = 0;
    size_t client_size = 0;
    size_t engine_size = 0;
};
