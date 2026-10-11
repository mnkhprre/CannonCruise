// ============================================================================
// CannonCruise - Ocean Water Tile (OceanTile.cpp)
// Original path: D:\Projects\CannonCruisePC\code\OceanTile.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_00432d10 / FUN_00432e00 — COceanTile::Initialize ("paColors != NULL Failed", "Memory leak !")
//   FUN_004331e0 — COceanTile::Update
//   FUN_00433540 / FUN_00433630 — COceanTile::Render ("Can't access array with size 0 !", "Trying to access nonexisting array !")
// ============================================================================

#include "OceanTile.h"
#include "Ocean_W32.h"
#include <cassert>
#include <cmath>
#include <cstdlib>

// ---------------------------------------------------------------------------
// Constructor – clears buffer pointers and vertex counter.
// ---------------------------------------------------------------------------
COceanTile::COceanTile()
    : m_pAtomic(0),
      m_paColors(0),
      m_paTexCoords(0),
      m_paVertices(0),
      m_uNumVertices(0)
{
}

// ---------------------------------------------------------------------------
// Destructor – frees dynamic memory buffers and destroys atomic if allocated.
// ---------------------------------------------------------------------------
COceanTile::~COceanTile()
{
    if (m_paColors) free(m_paColors);
    if (m_paTexCoords) free(m_paTexCoords);
    if (m_paVertices) free(m_paVertices);
    if (m_pAtomic) RpAtomicDestroy(m_pAtomic);
}

// ---------------------------------------------------------------------------
// Initialize – mirrors FUN_00432e00.
// Allocates vertex, texture coordinate, and vertex color buffers for an N×N grid.
// Validates all allocations with binary asserts.
// ---------------------------------------------------------------------------
void COceanTile::Initialize(RwReal originX, RwReal originZ, RwReal size, RwInt32 subdivisions)
{
    if (subdivisions < 1) subdivisions = 1;
    const RwInt32 side = subdivisions + 1;
    m_uNumVertices = (RwUInt32)(side * side);

    m_paColors = (RwRGBA*)malloc(m_uNumVertices * sizeof(RwRGBA));
    assert(m_paColors != 0 && "paColors != NULL Failed");

    m_paTexCoords = (RwTexCoords*)malloc(m_uNumVertices * sizeof(RwTexCoords));
    assert(m_paTexCoords != 0 && "paTexCoords != NULL Failed");

    m_paVertices = (RwV3d*)malloc(m_uNumVertices * sizeof(RwV3d));
    assert(m_paVertices != 0 && "paVertices != NULL Failed");

    // Düzlemsel grid: [originX, originX+size] × [originZ, originZ+size],
    // Y=0 (Update dalgaları bindirir); UV 0..1; su mavisi.
    const RwReal step = size / (RwReal)subdivisions;
    for (RwInt32 iz = 0; iz < side; ++iz) {
        for (RwInt32 ix = 0; ix < side; ++ix) {
            const RwUInt32 v = (RwUInt32)(iz * side + ix);
            m_paVertices[v].x = originX + (RwReal)ix * step;
            m_paVertices[v].y = 0.0f;
            m_paVertices[v].z = originZ + (RwReal)iz * step;
            m_paTexCoords[v].u = (RwReal)ix / (RwReal)subdivisions;
            m_paTexCoords[v].v = (RwReal)iz / (RwReal)subdivisions;
            m_paColors[v].red = 60;
            m_paColors[v].green = 130;
            m_paColors[v].blue = 200;
            m_paColors[v].alpha = 255; // opak — alpha blend karanlıkleştiriyordu
        }
    }
}

// ---------------------------------------------------------------------------
// Update – mirrors FUN_004331e0.
// Perturbs the Y component of grid vertices according to sinusoidal wave functions.
// ---------------------------------------------------------------------------
void COceanTile::Update(RwReal waveTime)
{
    if (!m_paVertices) return;

    // COcean::CalculateWaterHeight ile aynı formül (FUN_0042f500):
    // y = sin(x*0.05 + t*2)*1.5 + cos(z*0.08 + t*1.5)*1.0
    for (RwUInt32 i = 0; i < m_uNumVertices; ++i)
    {
        m_paVertices[i].y =
            std::sin(m_paVertices[i].x * 0.05f + waveTime * 2.0f) * 1.5f +
            std::cos(m_paVertices[i].z * 0.08f + waveTime * 1.5f) * 1.0f;
    }
}

// ---------------------------------------------------------------------------
// Render – mirrors FUN_00433630.
// Passes dynamic vertex, UV, and color arrays to COcean_W32::RenderWaterMesh.
// ---------------------------------------------------------------------------
void COceanTile::Render()
{
    if (m_uNumVertices == 0)
    {
        assert(false && "Can't access array with size 0 !");
        return;
    }

    if (!m_paVertices || !m_paTexCoords || !m_paColors)
    {
        assert(false && "Trying to access nonexisting array !");
        return;
    }

    COcean_W32::RenderWaterMesh(m_paVertices, m_paTexCoords, m_paColors, m_uNumVertices);
}
