#pragma once
#include <cstdint>

namespace CLIENT {
    constexpr std::uintptr_t dwLocalPlayerController = 0x23A78D0;
    constexpr std::uintptr_t dwLocalPlayerPawn       = 0x23CCC08;
    constexpr std::uintptr_t dwGameEntitySystem       = 0x2577BE0;
    constexpr std::uintptr_t dwViewAngles             = 0x23E2C98;
    constexpr std::uintptr_t dwViewMatrix             = 0x23D21F0;
    constexpr std::uintptr_t dwHighestEntityIndex     = 0x2090;
    constexpr std::uintptr_t dwChunkPointers          = 0x10;
    constexpr std::uintptr_t dwSlotStride             = 0x70;
}

namespace ENGINE2 {
    constexpr std::uintptr_t dwWindowWidth            = 0x912AC0;
    constexpr std::uintptr_t dwWindowHeight           = 0x912AC4;
}

namespace SCH {
    constexpr std::uintptr_t m_pGameSceneNode         = 0x330;
    constexpr std::uintptr_t m_iHealth                = 0x34C;
    constexpr std::uintptr_t m_vecVelocity            = 0x430;
    constexpr std::uintptr_t m_lifeState              = 0x354;
    constexpr std::uintptr_t m_iTeamNum               = 0x3E7;
    constexpr std::uintptr_t m_fFlags                 = 0x3F4;
    constexpr std::uintptr_t m_vecAbsOrigin           = 0xC8;
    constexpr std::uintptr_t m_flDuckAmount           = 0x40C;
    constexpr std::uintptr_t m_pMovementServices      = 0x1248;
    constexpr std::uintptr_t m_hOriginalController    = 0x1478;
    constexpr std::uintptr_t m_entitySpottedState     = 0x1C60;
    constexpr std::uintptr_t m_bSpotted               = 0x8;
    constexpr std::uintptr_t m_hPlayerPawn            = 0x914;
    constexpr std::uintptr_t m_sSanitizedPlayerName   = 0x868;
    constexpr std::uintptr_t m_pWeaponServices        = 0x1208;
    constexpr std::uintptr_t m_hActiveWeapon          = 0x60;
    constexpr std::uintptr_t m_AttributeManager       = 0x11A8;
    constexpr std::uintptr_t m_Item                   = 0x50;
    constexpr std::uintptr_t m_iItemDefinitionIndex   = 0x1BA;
    constexpr std::uintptr_t m_modelState             = 0x140;
    constexpr std::uintptr_t m_iPing                  = 0x830;
    constexpr std::uintptr_t m_pCameraServices        = 0x1240;
    constexpr std::uintptr_t m_vecCsViewPunchAngle    = 0x48;
    constexpr std::uintptr_t m_nButtons               = 0x50;
}
