// ============================================================================
// CannonCruise - Ocean Water Simulation (Ocean.cpp)
// Original path: D:\Projects\CannonCruisePC\code\Ocean.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_0042d870 — COcean constructor & registration
//   FUN_0042d9b0 — COcean::Initialize
//   FUN_0042e800 — COcean::Update
//   FUN_0042f500 — COcean::GetWaterHeight
// ============================================================================

#include "Ocean.h"
#include "OceanTile.h"
#include "framework/core/attributehandler/cclassfactory.h"
#include <cassert>
#include <cmath>
#include <cstring>

RWS_REGISTER_CLASS(COcean, []() -> void* { return new COcean(); }, sizeof(COcean));

COcean::COcean()
    : m_uSkyDomeCount(0),
      m_fWaveTime(0.0f),
      m_bHiDetail(TRUE)
{
    memset(m_apSkyDomes, 0, sizeof(m_apSkyDomes));
}

COcean::~COcean()
{
    for (size_t i = 0; i < m_Tiles.size(); ++i)
    {
        delete m_Tiles[i];
    }
    m_Tiles.clear();

    for (RwUInt32 i = 0; i < m_uSkyDomeCount; ++i)
    {
        if (m_apSkyDomes[i])
        {
            RpClumpDestroy(m_apSkyDomes[i]);
            m_apSkyDomes[i] = 0;
        }
    }
}

void COcean::HandleEvents(const RWS::CMsg& msg)
{
    // iMsgOceanUpdate, iMsgOceanDestroy
}

void COcean::Initialize(RwInt32 numTilesX, RwInt32 numTilesZ, RwReal tileSize)
{
    // Okyanus ızgara karolarını oluştur
}

void COcean::Update(RwReal deltaTime)
{
    m_fWaveTime += deltaTime;

    for (size_t i = 0; i < m_Tiles.size(); ++i)
    {
        if (m_Tiles[i])
        {
            m_Tiles[i]->Update(m_fWaveTime);
        }
    }
}

void COcean::Render()
{
    for (size_t i = 0; i < m_Tiles.size(); ++i)
    {
        if (m_Tiles[i])
        {
            m_Tiles[i]->Render();
        }
    }
}

// ============================================================================
// COcean::GetWaterHeight (FUN_0042f500)
// ASM: 0042f500 — Verilen X/Z koordinatındaki dalga yüksekliğini ve normalini hesapla
// ============================================================================
RwReal COcean::GetWaterHeight(RwReal worldX, RwReal worldZ, RwV3d* pNormal) const
{
    // Gerstner / Sinüs dalga modeli
    RwReal wave1 = std::sin(worldX * 0.05f + m_fWaveTime * 2.0f) * 1.5f;
    RwReal wave2 = std::cos(worldZ * 0.08f + m_fWaveTime * 1.5f) * 1.0f;
    RwReal height = wave1 + wave2;

    if (pNormal)
    {
        pNormal->x = -std::cos(worldX * 0.05f + m_fWaveTime * 2.0f) * 0.075f;
        pNormal->y = 1.0f;
        pNormal->z = std::sin(worldZ * 0.08f + m_fWaveTime * 1.5f) * 0.08f;
        // Normalize normal
        RwReal len = std::sqrt(pNormal->x * pNormal->x + pNormal->y * pNormal->y + pNormal->z * pNormal->z);
        if (len > 0.0001f)
        {
            pNormal->x /= len;
            pNormal->y /= len;
            pNormal->z /= len;
        }
    }

    return height;
}
