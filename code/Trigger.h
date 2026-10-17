#pragma once

// ============================================================================
// CannonCruise - Area Trigger (Trigger.h)
// Original path: D:\Projects\CannonCruisePC\code\Trigger.h
// Reverse-engineered via Ghidra and CC.asm analysis.
// ============================================================================
// Binary ref: FUN_004bf390 - FUN_004bfe50
// ============================================================================

#include <rwcore.h>
#include <rpworld.h>
#include "framework/core/attributehandler/cattributehandler.h"
#include "framework/core/eventhandler/ceventhandler.h"
#include "CollisionTrigger.h"
#include "SpeakManager.h"

namespace RWS
{

/**
 * @class CTrigger
 * @brief Area trigger entity that fires events when entities enter or exit a region.
 *
 * CTrigger defines a position and radius in world coordinates; detects
 * entity entry/exit via the physics system and fires predefined RWS events.
 * Also supports voice/dialog triggering via CSpeakManager integration.
 *
 * **Features:**
 * - Spherical trigger area (position + radius)
 * - Entry (OnEnter) and exit (OnExit) events
 * - One-shot or repeating trigger mode (m_bOneShot)
 * - Voice dialog triggering via CSpeakManager
 *
 * **Binary references:**
 * - String table: `0x0061947c`
 * - Ctor: `FUN_004bf390`
 * - HandleEvents: `FUN_004bfe50`
 */
class CTrigger : public CAttributeHandler, public CEventHandler
{
public:
    RWS_MAKENEWCLASS(CTrigger);
    RWS_DECLARE_CLASS(CTrigger);

    /**
     * @brief Constructor — initializes trigger with attribute packet.
     * @param packet Packet containing position, radius and event ID data.
     */
    CTrigger(const CAttributePacket& packet);

    /**
     * @brief Destructor — clears event registrations.
     */
    virtual ~CTrigger();

    /**
     * @brief Reads position, radius and event IDs from attribute packet.
     * @param packet Packet containing current settings.
     */
    virtual void HandleAttributes(const CAttributePacket& packet);

    /**
     * @brief Processes incoming events; evaluates physics collision notifications.
     * @param msg CMsg event to process.
     */
    virtual void HandleEvents(CMsg& msg);

    /**
     * @brief Called when an entity enters the trigger area.
     *
     * Fires `m_eventOnEnter` event and if `m_speakId` is valid,
     * starts speech via CSpeakManager.
     */
    void OnEnter();

    /**
     * @brief Called when an entity exits the trigger area.
     *
     * Fires `m_eventOnExit` event.
     */
    void OnExit();

private:
    CEventId m_eventOnEnter;   ///< Entry event ID
    CEventId m_eventOnExit;    ///< Exit event ID
    CEventId m_eventOnStay;    ///< Stay event ID (periodic)

    RwV3d  m_triggerPos;       ///< Trigger area world position
    RwReal m_triggerRadius;    ///< Trigger area radius (meters)
    RwBool m_bOneShot;         ///< TRUE if triggered only once
    RwBool m_bTriggered;       ///< One-shot trigger occurred?
    RwBool m_bInsideTrigger;   ///< Entity currently inside trigger?

    RwInt32 m_speakId;         ///< Speech ID to play when triggered
};

} // namespace RWS
