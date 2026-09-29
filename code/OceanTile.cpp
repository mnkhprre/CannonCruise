// ============================================================================
// CannonCruise - Ocean Water Tile (OceanTile.cpp)
// Original path: D:\Projects\CannonCruisePC\code\OceanTile.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_00432e00 — COceanTile::Initialize
//   FUN_004331e0 — COceanTile::Update
//   FUN_00433630 — COceanTile::Render
// ============================================================================

#include "OceanTile.h"
#include <cassert>
#include <cmath>
#include <cstdlib>

COceanTile::COceanTile()
    : m_pAtomic(0),
      m_paColors(0),
      m_paTexCoords(0),
      m_paVertices(0),
      m_uNumVertices(0)
{
}

COceanTile::~COceanTile()
{
    if (m_paColors) free(m_paColors);
    if (m_paTexCoords) free(m_paTexCoords);
    if (m_paVertices) free(m_paVertices);
    if (m_pAtomic) RpAtomicDestroy(m_pAtomic);
}

void COceanTile::Initialize(RwReal originX, RwReal originZ, RwReal size, RwInt32 subdivisions)
{
    m_uNumVertices = (subdivisions + 1) * (subdivisions + 1);

    m_paColors = (RwRGBA*)malloc(m_uNumVertices * sizeof(RwRGBA));
    assert(m_paColors != 0 && "paColors != NULL Failed");

    m_paTexCoords = (RwTexCoords*)malloc(m_uNumVertices * sizeof(RwTexCoords));
    assert(m_paTexCoords != 0 && "paTexCoords != NULL Failed");

    m_paVertices = (RwV3d*)malloc(m_uNumVertices * sizeof(RwV3d));
}

void COceanTile::Update(RwReal waveTime)
{
    if (!m_paVertices) return;

    // Dalga tepe noktalarını güncelle
    for (RwUInt32 i = 0; i < m_uNumVertices; ++i)
    {
        m_paVertices[i].y = std::sin(m_paVertices[i].x * 0.05f + waveTime * 2.0f) * 1.5f;
    }
}

void COceanTile::Render()
{
}
