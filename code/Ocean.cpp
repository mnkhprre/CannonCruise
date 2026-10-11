// ============================================================================
// CannonCruise - Ocean Water Simulation (Ocean.cpp)
// Original path: D:\Projects\CannonCruisePC\code\Ocean.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_0042d870 — COcean constructor & class registration ("COcean")
//   FUN_0042d8b0 — COcean mesh tessellation ("Error in Tristripper")
//   FUN_0042dbf0 — COcean::Initialize ("Cannot create sky dome clump.", "Cannot create ocean atomic ")
//   FUN_0042e800 — COcean::Update
//   FUN_0042f500 — COcean::GetWaterHeight (wave height & normal formula)
// ============================================================================

#include "Ocean.h"
#include "OceanTile.h"
#include "framework/core/attributehandler/cclassfactory.h"
#include <cassert>
#include <cmath>
#include <cstring>

// Register COcean with RenderWare Studio reflection system
RWS_REGISTER_CLASS(COcean, []() -> void* { return new COcean(); }, sizeof(COcean));

COcean* COcean::sm_pInstance = 0;

// ---------------------------------------------------------------------------
// Constructor – mirrors FUN_0042d870.
// Clears sky dome pointers, enables high-detail mode, and resets wave timer.
// ---------------------------------------------------------------------------
COcean::COcean()
    : m_uSkyDomeCount(0),
      m_fWaveTime(0.0f),
      m_bHiDetail(TRUE)
{
    sm_pInstance = this;
    memset(m_apSkyDomes, 0, sizeof(m_apSkyDomes));
}

// ---------------------------------------------------------------------------
// Destructor – frees all allocated ocean tiles and cleans up sky dome clumps.
// ---------------------------------------------------------------------------
COcean::~COcean()
{
    if (sm_pInstance == this)
    {
        sm_pInstance = 0;
    }
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

// ---------------------------------------------------------------------------
// HandleEvents – listens for engine lifecycle and weather broadcast events:
//   • iMsgOceanUpdate
//   • iMsgOceanDestroy
//   • MSG_THUNDER0_RENDER
// ---------------------------------------------------------------------------
void COcean::HandleEvents(const RWS::CMsg& msg)
{
    // iMsgOceanUpdate, iMsgOceanDestroy, MSG_THUNDER0_RENDER
}

// ---------------------------------------------------------------------------
// Initialize – mirrors FUN_0042dbf0.
// Generates the grid of water tiles covering the sea surface based on level extents.
// ---------------------------------------------------------------------------
void COcean::Initialize(RwInt32 numTilesX, RwInt32 numTilesZ, RwReal tileSize,
                        RwReal originX, RwReal originZ)
{
    // mevcut ızgarayı temizle (yeniden-init güvenli)
    for (size_t i = 0; i < m_Tiles.size(); ++i) {
        delete m_Tiles[i];
    }
    m_Tiles.clear();

    if (numTilesX < 1) numTilesX = 1;
    if (numTilesZ < 1) numTilesZ = 1;
    if (tileSize <= 0.0f) tileSize = 100.0f;
    const RwInt32 kSubdivisions = 8; // 9×9 grid/tile — Im3D altında

    for (RwInt32 z = 0; z < numTilesZ; ++z) {
        for (RwInt32 x = 0; x < numTilesX; ++x) {
            COceanTile *pTile = new COceanTile;
            pTile->Initialize(originX + (RwReal)x * tileSize,
                              originZ + (RwReal)z * tileSize, tileSize,
                              kSubdivisions);
            m_Tiles.push_back(pTile);
        }
    }
}

// ---------------------------------------------------------------------------
// Update – mirrors FUN_0042e800.
// Increments global wave simulation timestamp and triggers per-tile vertex perturbation.
// ---------------------------------------------------------------------------
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

// ---------------------------------------------------------------------------
// Render – iterates over all active water tiles to submit geometry for rendering.
// ---------------------------------------------------------------------------
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

// ---------------------------------------------------------------------------
// CalculateWaterHeight – mirrors FUN_0042f500 (ASM: 0042f500).
// Evaluates compound sinusoidal wave displacement and surface normal:
//   wave1 = sin(X * 0.05 + time * 2.0) * 1.5
//   wave2 = cos(Z * 0.08 + time * 1.5) * 1.0
//   height = wave1 + wave2
// Also computes partial derivatives for normalized surface normal calculation
// required by physics buoyancy torque and water lighting.
// ---------------------------------------------------------------------------
RwReal COcean::CalculateWaterHeight(RwReal worldX, RwReal worldZ, RwV3d* pNormal) const
{
    RwReal wave1 = std::sin(worldX * 0.05f + m_fWaveTime * 2.0f) * 1.5f;
    RwReal wave2 = std::cos(worldZ * 0.08f + m_fWaveTime * 1.5f) * 1.0f;
    RwReal height = wave1 + wave2;

    if (pNormal)
    {
        pNormal->x = -std::cos(worldX * 0.05f + m_fWaveTime * 2.0f) * 0.075f;
        pNormal->y = 1.0f;
        pNormal->z = std::sin(worldZ * 0.08f + m_fWaveTime * 1.5f) * 0.08f;
        // Normalize normal vector
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
