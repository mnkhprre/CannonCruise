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

class COcean : public RWS::CEventHandler
{
public:
    COcean();
    virtual ~COcean();

    virtual void HandleEvents(const RWS::CMsg& msg);

    void Initialize(RwInt32 numTilesX, RwInt32 numTilesZ, RwReal tileSize);
    void Update(RwReal deltaTime);
    void Render();

    RwReal GetWaterHeight(RwReal worldX, RwReal worldZ, RwV3d* pNormal = 0) const;

private:
    std::vector<COceanTile*> m_Tiles;
    RpClump* m_apSkyDomes[OCEAN_MAX_SKY_DOMES];
    RwUInt32 m_uSkyDomeCount;
    RwReal m_fWaveTime;
    RwBool m_bHiDetail;
};

#endif // OCEAN_H
