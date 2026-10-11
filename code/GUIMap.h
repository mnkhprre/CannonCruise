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

/**
 * @enum EMissionTextureType
 * @brief Icon / blip texture categories rendered on the radar minimap.
 */
enum EMissionTextureType
{
    MISSION_TEXTURE_PIRATE = 0, ///< Friendly/player or pirate point indicator
    MISSION_TEXTURE_MISSION,    ///< Primary mission objective marker
    MISSION_TEXTURE_ENEMY,      ///< Hostile vessel blip marker
    MISSION_TEXTURE_CHEST,      ///< Treasure chest / cargo flotsam marker
    MISSION_TEXTURE_COUNT       ///< Total number of icon types
};

/**
 * @struct SMissionIndicatorData
 * @brief Represents a tracked point of interest on the minimap.
 */
struct SMissionIndicatorData
{
    RwV3d               m_WorldPos; ///< 3D world coordinate of the tracked marker
    EMissionTextureType m_Type;     ///< Blip icon classification type
    RwBool              m_bActive;  ///< Whether marker is currently visible on HUD
};

/**
 * @class CGUIMap
 * @brief HUD radar minimap and compass display component.
 *
 * Tracks world positions of ships, objectives, chests, and compass heading,
 * projecting them onto a circular 2D HUD radar display.
 */
class CGUIMap : public RWS::CEventHandler
{
public:
    /**
     * @brief Constructs a CGUIMap instance and initializes texture pointers.
     * @see FUN_00485830
     */
    CGUIMap();

    /**
     * @brief Destructor. Clears tracked indicator spots.
     */
    virtual ~CGUIMap();

    /**
     * @brief Handles incoming minimap event notifications.
     * @param msg Event message container.
     */
    virtual void HandleEvents(const RWS::CMsg& msg);

    /**
     * @brief Registers an objective/entity marker on the minimap.
     * @param pPos 3D world position of the spot.
     * @param type Texture / marker category.
     * @see FUN_004860b0
     */
    void AddSpot(const RwV3d* pPos, EMissionTextureType type);

    /**
     * @brief Removes all currently registered minimap spots.
     */
    void ClearSpots();

    /**
     * @brief Updates the player's yaw heading angle for compass rotation.
     * @param angle Heading angle in radians.
     */
    void SetPlayerHeading(RwReal angle);

    /**
     * @brief Renders the circular minimap backdrop, player compass, and blips.
     * @see FUN_004867f0
     */
    void Render();

private:
    std::vector<SMissionIndicatorData> m_Spots;                                  ///< List of active minimap blips
    RwTexture*                         m_apTextures[MISSION_TEXTURE_COUNT];      ///< Texture handles for all blip types
    RwReal                             m_fPlayerHeading;                         ///< Player vessel compass yaw angle
};

#endif // GUIMAP_H
