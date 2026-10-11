// ============================================================================
// CannonCruise - RenderBehaviourWorld singleton (RenderBehaviourWorld.cpp)
// Tersine mühendislik ile yeniden yazılmıştır.
// Binary ref: FUN_004c4410 (ctor/default), FUN_004c3e40 (property apply).
// ============================================================================

#include "RenderBehaviourWorld.h"

#include <cstring>

CRenderBehaviourWorld *CRenderBehaviourWorld::sm_pInstance = 0;

CRenderBehaviourWorld *CRenderBehaviourWorld::Instance() {
    if (sm_pInstance == 0) {
        sm_pInstance = new CRenderBehaviourWorld();
        sm_pInstance->Init();
    }
    return sm_pInstance;
}

void CRenderBehaviourWorld::Destroy() {
    delete sm_pInstance;
    sm_pInstance = 0;
}

void CRenderBehaviourWorld::Init() {
    // FUN_004c4410 default'ları (çekirdek alan; pass/pool'lar port
    // gerektiğinde eklenir):
    //   [5]=+0x14 → 0, [6]=+0x18 → 125.0f, [7]=+0x1c → 1.0f,
    //   [8]=+0x20 → 0, [9]=+0x24 → 1, [0x14]=+0x50 → 1000.0f,
    //   [0x15]=+0x54 → 300.0f, [0x16]=+0x58 → 0
    std::memset(this, 0, sizeof(*this));
    m_Render.m_param0 = 0.0f;
    m_Render.m_param1 = 125.0f;
    m_param2 = 1.0f;
    m_pad24 = 1;
    m_param8 = 1000.0f;
    m_param9 = 300.0f;
}

RwBool CRenderBehaviourWorld::ApplyWbRenderAttribute(RwUInt32 id,
                                                     RwUInt32 value) {
    switch (id) {
    case 0x0:
        std::memcpy(&m_Render.m_param0, &value, 4);
        return TRUE;
    case 0x1:
        std::memcpy(&m_Render.m_param1, &value, 4);
        return TRUE;
    case 0x2:
        std::memcpy(&m_param2, &value, 4);
        return TRUE;
    case 0x3:
        m_param3 = value;
        return TRUE;
    case 0x4:
        m_param4 = value;
        return TRUE;
    case 0x5:
        std::memcpy(&m_param5, &value, 4);
        return TRUE;
    case 0x6:
        // ARGB → R,G,B baytları float olarak (binary: >>16, >>8, &0xff)
        m_color1R = (float)((value >> 16) & 0xFF);
        m_color1G = (float)((value >> 8) & 0xFF);
        m_color1B = (float)(value & 0xFF);
        return TRUE;
    case 0x7:
        std::memcpy(&m_param7, &value, 4);
        return TRUE;
    case 0x8:
        std::memcpy(&m_param8, &value, 4);
        return TRUE;
    case 0x9:
        std::memcpy(&m_param9, &value, 4);
        return TRUE;
    case 0xA:
        m_color2R = (float)((value >> 16) & 0xFF);
        m_color2G = (float)((value >> 8) & 0xFF);
        m_color2B = (float)(value & 0xFF);
        return TRUE;
    case 0xB:
        std::memcpy(&m_paramB, &value, 4);
        return TRUE;
    case 0xC:
        std::memcpy(&m_paramC, &value, 4);
        return TRUE;
    default:
        // binary: assert "Unhandled attribute in WbRender..." (satır 0xC0)
        return FALSE;
    }
}
