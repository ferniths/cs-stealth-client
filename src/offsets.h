#pragma once
#include <cstdint>

namespace CLIENT {
    constexpr std::uintptr_t dwLocalPlayerController = 0x25324D8;
    constexpr std::uintptr_t dwLocalPlayerPawn       = 0x255B598;
    constexpr std::uintptr_t dwGameEntitySystem       = 0x2710038;
    constexpr std::uintptr_t dwViewAngles             = 0x2571108;
    constexpr std::uintptr_t dwViewMatrix             = 0x25608E0;
    constexpr std::uintptr_t dwHighestEntityIndex     = 0x2120;
    constexpr std::uintptr_t dwChunkPointers          = 0x10;
    constexpr std::uintptr_t dwSlotStride             = 0x70;
}

namespace ENGINE2 {
    constexpr std::uintptr_t dwWindowWidth            = 0x91E4D8;
    constexpr std::uintptr_t dwWindowHeight           = 0x91E4DC;
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
    constexpr std::uintptr_t m_pMovementServices      = 0x1330;
    constexpr std::uintptr_t m_hOriginalController    = 0x1560;
    constexpr std::uintptr_t m_entitySpottedState     = 0x1E88;
    constexpr std::uintptr_t m_bSpotted               = 0x8;
    constexpr std::uintptr_t m_hPlayerPawn            = 0x92C;
    constexpr std::uintptr_t m_sSanitizedPlayerName   = 0x878;
    constexpr std::uintptr_t m_pWeaponServices        = 0x12F0;
    constexpr std::uintptr_t m_hActiveWeapon          = 0x60;
    constexpr std::uintptr_t m_AttributeManager       = 0x1290;
    constexpr std::uintptr_t m_Item                   = 0x50;
    constexpr std::uintptr_t m_iItemDefinitionIndex   = 0x1BA;
    constexpr std::uintptr_t m_modelState             = 0x140;
    constexpr std::uintptr_t m_iPing                  = 0x838;
    constexpr std::uintptr_t m_pCameraServices        = 0x1328;
    constexpr std::uintptr_t m_vecCsViewPunchAngle    = 0x48;
    constexpr std::uintptr_t m_nButtons               = 0x50;
}
