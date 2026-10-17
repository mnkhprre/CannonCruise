#pragma once

// ============================================================================
// CannonCruise - Ammo Crate Pickup (AmmoCrate.h)
// Original path: D:\Projects\CannonCruisePC\code\AmmoCrate.h
// Reverse-engineered via Ghidra and CC.asm analysis.
// ============================================================================

#include "Flotsam.h"

namespace RWS
{
    /**
     * @class CAmmoCrate
     * @brief Floating munitions crate replenishing ship special ammunition.
     *
     * Derives from CFlotsam. Upon collision with player ship, dispatches
     * `EVENT_AMMO_COLLECTED` supplying projectile type and quantity,
     * triggers audio cue, and removes the crate from the world.
     */
    class CAmmoCrate : public CFlotsam
    {
    public:
        RWS_MAKENEWCLASS(CAmmoCrate);
        RWS_DECLARE_CLASS(CAmmoCrate);

        /**
         * @brief Constructs ammo crate instance from attribute packet.
         * @param packet  Attribute initialization packet.
         */
        CAmmoCrate(const CAttributePacket& packet);

        /**
         * @brief Destructor.
         */
        virtual ~CAmmoCrate();

        /**
         * @brief Attribute dispatcher for ammo type and count configuration.
         * @param packet  Contains ammo parameters.
         */
        virtual void HandleAttributes(const CAttributePacket& packet);

        /**
         * @brief Message dispatcher (forwards to CFlotsam).
         * @param msg  Message descriptor.
         */
        virtual void HandleEvents(CMsg& msg);

        /**
         * @brief Collection handler – delivers ammo payload, fires event, and despawns.
         */
        virtual void OnCollect();

    private:
        RwInt32 m_ammoType;                 ///< Munition classification index (offset 0xac).
        RwInt32 m_ammoCount;                ///< Quantity of ammunition rounds granted (offset 0xb0).
        CEventId m_eventAmmoCollected;      ///< Event ID ("EVENT_AMMO_COLLECTED", offset 0xb4).
    };
}
