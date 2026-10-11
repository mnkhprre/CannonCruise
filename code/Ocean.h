#ifndef OCEAN_H
#define OCEAN_H

// ============================================================================
// CannonCruise - Ocean Water Simulation (Ocean.h)
// Original path: D:\Projects\CannonCruisePC\code\Ocean.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include "framework/core/eventhandler/ceventhandler.h"
#include <rwcore.h>
#include <rpworld.h>
#include <vector>

#define OCEAN_MAX_SKY_DOMES 4

class COceanTile;

/**
 * @class COcean
 * @brief Dynamic ocean water simulation and surface mesh system.
 *
 * Implements a tiled water surface with analytical wave height calculation
 * (Gerstner / harmonic sine waves) used both for visual rendering and
 * physical ship buoyancy / projectile splash queries. Also manages sky dome
 * environmental clumps and listens to engine lifecycle messages (iMsgOceanUpdate,
 * iMsgOceanDestroy, MSG_THUNDER0_RENDER).
 */
class COcean : public RWS::CEventHandler
{
public:
    /**
     * @brief Constructs an ocean simulation instance.
     *
     * Clears sky dome pointers, sets default high-detail rendering, and resets wave time.
     */
    COcean();

    /**
     * @brief Destructor – deallocates all child water tiles and destroys sky dome clumps.
     */
    virtual ~COcean();

    /**
     * @brief Framework message dispatcher.
     * @param msg  Incoming message descriptor (iMsgOceanUpdate, iMsgOceanDestroy, etc.).
     */
    virtual void HandleEvents(const RWS::CMsg& msg);

    /**
     * @brief Creates a grid of procedural water tiles spanning the active play area.
     * @param numTilesX   Number of tile partitions along the X axis.
     * @param numTilesZ   Number of tile partitions along the Z axis.
     * @param tileSize    Dimension of each individual square tile in world units.
     * @param originX     Grid köşe X (dünya koordinatı — varsayılan 0).
     * @param originZ     Grid köşe Z (dünya koordinatı — varsayılan 0).
     */
    void Initialize(RwInt32 numTilesX, RwInt32 numTilesZ, RwReal tileSize,
                    RwReal originX = 0.0f, RwReal originZ = 0.0f);

    /**
     * @brief Per-frame simulation tick advancing wave phases and tessellation offsets.
     * @param deltaTime  Elapsed frame duration in seconds.
     */
    void Update(RwReal deltaTime);

    /**
     * @brief Renders all visible ocean surface tiles using Direct3D / RenderWare Im3D.
     */
    void Render();

    /**
     * @brief Queries instantaneous wave elevation and surface normal at specified world coordinates.
     * @param worldX        X coordinate in world space.
     * @param worldZ        Z coordinate in world space.
     * @param[out] pNormal  Optional pointer to receive unit surface normal vector.
     * @return Wave elevation Y in world units.
     */
    RwReal CalculateWaterHeight(RwReal worldX, RwReal worldZ, RwV3d* pNormal = 0) const;

    static COcean* GetInstance() { return sm_pInstance; }
    static RwReal GetWaterHeight(RwReal worldX, RwReal worldZ, RwV3d* pNormal = 0)
    {
        if (sm_pInstance) return sm_pInstance->CalculateWaterHeight(worldX, worldZ, pNormal);
        return 0.0f;
    }

private:
    static COcean* sm_pInstance;                        ///< Active ocean instance pointer.
    std::vector<COceanTile*> m_Tiles;                   ///< Collection of subdivided water surface grid tiles.
    RpClump* m_apSkyDomes[OCEAN_MAX_SKY_DOMES];         ///< Environmental sky hemisphere clumps.
    RwUInt32 m_uSkyDomeCount;                           ///< Number of active sky dome models.
    RwReal m_fWaveTime;                                 ///< Accumulated wave animation time in seconds.
    RwBool m_bHiDetail;                                 ///< Flag enabling high-resolution water mesh geometry.
};

#endif // OCEAN_H
