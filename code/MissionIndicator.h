#ifndef MISSIONINDICATOR_H
#define MISSIONINDICATOR_H

// ============================================================================
// CannonCruise - Mission Objective Indicator (MissionIndicator.h)
// Original path: D:\Projects\CannonCruisePC\code\MissionIndicator.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include "framework/core/eventhandler/ceventhandler.h"
#include <rwcore.h>
#include <rpworld.h>

/**
 * @class CMissionIndicator
 * @brief In-world 3D floating objective beacon / directional waypoint marker.
 *
 * Renders an animated hovering 3D marker above active mission targets
 * (ports, enemy flagship, flotsam caches) and tracks target coordinates.
 */
class CMissionIndicator : public RWS::CEventHandler
{
public:
    /**
     * @brief Constructs a new CMissionIndicator instance.
     * @see FUN_00484eb0
     */
    CMissionIndicator();

    /**
     * @brief Destructor. Destroys allocated 3D indicator clump.
     */
    virtual ~CMissionIndicator();

    /**
     * @brief Handles incoming event messages.
     * @param msg Event message container.
     */
    virtual void HandleEvents(const RWS::CMsg& msg);

    /**
     * @brief Sets target world position to hover above.
     * @param pPos 3D world position coordinates.
     */
    void SetTargetPosition(const RwV3d* pPos);

    /**
     * @brief Ticks bobbing and rotational hovering animation.
     * @param deltaTime Elapsed frame time in seconds.
     * @see FUN_00485320
     */
    void Update(RwReal deltaTime);

    /**
     * @brief Renders the 3D marker clump in the world.
     */
    void Render();

    /**
     * @brief Sets visibility state of the beacon.
     * @param bActive TRUE to show indicator, FALSE to hide.
     */
    void SetActive(RwBool bActive) { m_bActive = bActive; }

    /**
     * @brief Checks if indicator is active.
     * @return TRUE if active, FALSE otherwise.
     */
    RwBool IsActive() const { return m_bActive; }

private:
    RpClump* m_pClump;        ///< 3D beacon arrow clump geometry
    RwV3d    m_TargetPos;     ///< Target location to indicate
    RwReal   m_fHoverAngle;   ///< Oscillation angle for bobbing animation
    RwReal   m_fRotationYaw;  ///< Current rotation yaw angle
    RwBool   m_bActive;       ///< Visibility active flag
};

#endif // MISSIONINDICATOR_H
