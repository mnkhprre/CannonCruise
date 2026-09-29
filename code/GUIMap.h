#ifndef GUIMAP_H
#define GUIMAP_H

// ============================================================================
// CannonCruise - GUI Minimap (GUIMap.h)
// Original path: D:\Projects\CannonCruisePC\code\GUIMap.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include "framework/core/eventhandler/ceventhandler.h"
#include <rwcore.h>
#include <vector>

enum EMissionTextureType
{
    MISSION_TEXTURE_PIRATE = 0,
    MISSION_TEXTURE_MISSION,
    MISSION_TEXTURE_ENEMY,
    MISSION_TEXTURE_CHEST,
    MISSION_TEXTURE_COUNT
};

struct SMissionIndicatorData
{
    RwV3d m_WorldPos;
    EMissionTextureType m_Type;
    RwBool m_bActive;
};

class CGUIMap : public RWS::CEventHandler
{
public:
    CGUIMap();
    virtual ~CGUIMap();

    virtual void HandleEvents(const RWS::CMsg& msg);

    void AddSpot(const RwV3d* pPos, EMissionTextureType type);
    void ClearSpots();
    void SetPlayerHeading(RwReal angle);
    void Render();

private:
    std::vector<SMissionIndicatorData> m_Spots;
    RwTexture* m_apTextures[MISSION_TEXTURE_COUNT];
    RwReal m_fPlayerHeading;
};

#endif // GUIMAP_H
