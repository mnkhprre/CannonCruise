#ifndef OCEANTILE_H
#define OCEANTILE_H

// ============================================================================
// CannonCruise - Ocean Water Tile (OceanTile.h)
// Original path: D:\Projects\CannonCruisePC\code\OceanTile.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include <rwcore.h>
#include <rpworld.h>

/**
 * @class COceanTile
 * @brief Represents a tessellated grid partition of the ocean surface.
 *
 * Owns vertex arrays (RwV3d), texture UVs (RwTexCoords), and vertex color
 * buffers (RwRGBA) representing dynamic water wave geometry. Per frame,
 * vertices are animated along the Y axis before being submitted for drawing.
 */
class COceanTile
{
public:
    /**
     * @brief Constructs an uninitialized ocean tile with zeroed pointers.
     */
    COceanTile();

    /**
     * @brief Destructor – releases vertex, color, and UV arrays and destroys atomic.
     */
    ~COceanTile();

    /**
     * @brief Allocates dynamic mesh buffers and sets initial planar coordinates.
     * @param originX       Top-left X origin in world coordinates.
     * @param originZ       Top-left Z origin in world coordinates.
     * @param size          Width/depth span of the tile in world units.
     * @param subdivisions  Grid division frequency along each axis.
     */
    void Initialize(RwReal originX, RwReal originZ, RwReal size, RwInt32 subdivisions);

    /**
     * @brief Perturbs vertex Y values based on simulation wave time.
     * @param waveTime  Elapsed wave phase time in seconds.
     */
    void Update(RwReal waveTime);

    /**
     * @brief Submits tile geometry to the rendering pipeline (COcean_W32).
     */
    void Render();

private:
    RpAtomic* m_pAtomic;            ///< Optional RenderWare atomic wrapper for tile mesh.
    RwRGBA* m_paColors;             ///< Per-vertex color buffer for water transparency/shading.
    RwTexCoords* m_paTexCoords;     ///< UV coordinates buffer for water ripple textures.
    RwV3d* m_paVertices;            ///< Deformable 3D vertex position buffer.
    RwUInt32 m_uNumVertices;        ///< Total vertex count ((subdivisions + 1)^2).
};

#endif // OCEANTILE_H
