#pragma once

// ============================================================================
// CannonCruise - Event Proxy (eventproxy.h)
// Original path: D:\Projects\CannonCruisePC\code\eventproxy.h
// Reverse-engineered via Ghidra and CC.asm analysis.
// Binary ref: FUN_00473480 - FUN_00473600
// ============================================================================

#include <rwcore.h>
#include <rpworld.h>
#include "framework/core/attributehandler/cattributehandler.h"
#include "framework/core/eventhandler/ceventhandler.h"

namespace RWS
{

/**
 * @class CEventProxy
 * @brief Event router using named event IDs between entities.
 *
 * CEventProxy receives an input event and re-fires it by converting
 * it to an output event. This mechanism allows level designers to
 * connect entities unaware of each other from RenderWare Studio editor.
 * Also handles Fernando character-specific IDUNA01/02/03 dialog events.
 *
 * **Features:**
 * - Input -> Output event routing
 * - IDUNA01/02/03 — Fernando NPC dialog triggers
 * - Runtime enable/disable via `m_bEnabled`
 *
 * **Binary references:**
 * - String table: `0x00616e38` (IDUNA01/02/03 event names)
 * - Ctor: `FUN_00473480`
 * - HandleEvents: `FUN_00473580`
 */
class CEventProxy : public CAttributeHandler, public CEventHandler
{
public:
    RWS_MAKENEWCLASS(CEventProxy);
    RWS_DECLARE_CLASS(CEventProxy);

    /**
     * @brief Constructor — initializes event proxy with attribute packet.
     * @param packet Attribute packet containing input/output event names.
     */
    CEventProxy(const CAttributePacket& packet);

    /**
     * @brief Destructor — cancels event registrations.
     */
    virtual ~CEventProxy();

    /**
     * @brief Updates input/output event IDs and enabled state.
     * @param packet Packet containing current event ID settings.
     */
    virtual void HandleAttributes(const CAttributePacket& packet);

    /**
     * @brief Receives input event and fires output event.
     * @param msg Incoming CMsg event to process.
     *
     * IDUNA01/02/03 messages handled with special Fernando dialog logic.
     */
    virtual void HandleEvents(CMsg& msg);

private:
    CEventId m_eventInput;      ///< Input event ID to listen for
    CEventId m_eventOutput;     ///< Output event ID to route to
    CEventId m_eventIduna01;    ///< Fernando dialog event "IDUNA01"
    CEventId m_eventIduna02;    ///< Fernando dialog event "IDUNA02"
    CEventId m_eventIduna03;    ///< Fernando dialog event "IDUNA03"
    RwBool m_bEnabled;          ///< FALSE means events are not routed
};

} // namespace RWS
