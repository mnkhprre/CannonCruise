#pragma once

// ============================================================================
// CannonCruise - Health Debris Pickup (HealthDebris.h)
// Original path: D:\Projects\CannonCruisePC\code\HealthDebris.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include "Debris.h"

namespace RWS
{
    /**
     * @class CHealthDebris
     * @brief Floating health crate / restorative pickup in the water.
     *
     * Derives from CDebris. When collected by the player vessel, broadcasts
     * `EVENT_HEALTH_RESTORED` carrying the restorative HP amount (default 25.0),
     * plays `Audio_HealthPickup`, and removes itself from the world.
     */
    class CHealthDebris : public CDebris
    {
    public:
        RWS_MAKENEWCLASS(CHealthDebris);
        RWS_DECLARE_CLASS(CHealthDebris);

        /**
         * @brief Constructs a health debris instance from attribute packet.
         * @param packet  Attribute initialization packet.
         */
        CHealthDebris(const CAttributePacket& packet);

        /**
         * @brief Destructor.
         */
        virtual ~CHealthDebris();

        /**
         * @brief Attribute dispatcher for configuring restoration amount.
         * @param packet  Contains health amount command data.
         */
        virtual void HandleAttributes(const CAttributePacket& packet);

        /**
         * @brief Message dispatcher (forwards to CDebris).
         * @param msg  Message descriptor.
         */
        virtual void HandleEvents(CMsg& msg);

        /**
         * @brief Collection handler – plays audio cue, fires event, and despawns.
         */
        virtual void OnCollect();

    private:
        RwReal m_healthAmount;              ///< Hit points restored to collector (offset 0xb4).
        CEventId m_eventHealthRestored;     ///< Framework event ID ("EVENT_HEALTH_RESTORED", offset 0xb8).
    };
}
