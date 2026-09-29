#ifndef OCEANTILE_H
#define OCEANTILE_H

// ============================================================================
// CannonCruise - Ocean Water Tile (OceanTile.h)
// Original path: D:\Projects\CannonCruisePC\code\OceanTile.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include <rwcore.h>
#include <rpworld.h>

class COceanTile
{
public:
    COceanTile();
    ~COceanTile();

    void Initialize(RwReal originX, RwReal originZ, RwReal size, RwInt32 subdivisions);
    void Update(RwReal waveTime);
    void Render();

private:
    RpAtomic* m_pAtomic;
    RwRGBA* m_paColors;
    RwTexCoords* m_paTexCoords;
    RwV3d* m_paVertices;
    RwUInt32 m_uNumVertices;
};

#endif // OCEANTILE_H
