#pragma once
#include <cstdint>

namespace CLIENT {
    extern std::uintptr_t dwLocalPlayerController;
    extern std::uintptr_t dwLocalPlayerPawn;
    extern std::uintptr_t dwGameEntitySystem;
    extern std::uintptr_t dwViewAngles;
    extern std::uintptr_t dwViewMatrix;
    extern std::uintptr_t dwHighestEntityIndex;
    extern std::uintptr_t dwChunkPointers;
    extern std::uintptr_t dwSlotStride;
}

namespace ENGINE2 {
    extern std::uintptr_t dwWindowWidth;
    extern std::uintptr_t dwWindowHeight;
}

namespace SCH {
    extern std::uintptr_t m_pGameSceneNode;
    extern std::uintptr_t m_iHealth;
    extern std::uintptr_t m_vecVelocity;
    extern std::uintptr_t m_lifeState;
    extern std::uintptr_t m_iTeamNum;
    extern std::uintptr_t m_fFlags;
    extern std::uintptr_t m_vecAbsOrigin;
    extern std::uintptr_t m_flDuckAmount;
    extern std::uintptr_t m_pMovementServices;
    extern std::uintptr_t m_hOriginalController;
    extern std::uintptr_t m_entitySpottedState;
    extern std::uintptr_t m_bSpotted;
    extern std::uintptr_t m_hPlayerPawn;
    extern std::uintptr_t m_sSanitizedPlayerName;
    extern std::uintptr_t m_pWeaponServices;
    extern std::uintptr_t m_hActiveWeapon;
    extern std::uintptr_t m_AttributeManager;
    extern std::uintptr_t m_Item;
    extern std::uintptr_t m_iItemDefinitionIndex;
    extern std::uintptr_t m_modelState;
    extern std::uintptr_t m_iPing;
    extern std::uintptr_t m_pCameraServices;
    extern std::uintptr_t m_vecCsViewPunchAngle;
    extern std::uintptr_t m_nButtons;
    extern std::uintptr_t m_bDormant;
}

// Resolve offsets at launch. Tries: download -> cache -> hardcoded.
// Returns true only if a fresh GitHub download was applied; false for
// cache/hardcoded so the caller can run a live pattern scan after attach.
bool resolve_offsets();

// Call after Memory::attach() to pattern-scan the LIVE modules. This is the
// authoritative source: dumps can lag a game update. Returns true if scan
// succeeded for at least one base offset.
bool resolve_offsets_runtime(void* hProcess, std::uintptr_t client_base, std::uintptr_t engine_base,
                              size_t client_size, size_t engine_size);

// Human-readable source of the currently loaded offsets ("github", "cache",
// "pattern scan", "builtin", ...). For on-screen diagnostics.
const char* offsets_source();
