#pragma once

// ============================================================================
// CannonCruise - Debris Obstacle & Explosive Flotsam (Debris.h)
// Original path: D:\Projects\CannonCruisePC\code\Debris.h
// Reverse-engineered via Ghidra and CC.asm analysis.
// ============================================================================

#include "Flotsam.h"

namespace RWS
{
    /**
     * @class CDebris
     * @brief Floating obstacle that shatters or explodes upon weapon impact or ramming.
     *
     * Inherits flotation behavior from CFlotsam. Upon collision or projectile impact,
     * triggers Explode() to play detonation audio (Audio_CannonHit), spawns particle
     * debris effects, and cleans itself up.
     */
    class CDebris : public CFlotsam
    {
    public:
        RWS_MAKENEWCLASS(CDebris);
        RWS_DECLARE_CLASS(CDebris);

        /**
         * @brief Constructs debris instance from attribute packet.
         * @param packet  Attribute initialization packet.
         */
        CDebris(const CAttributePacket& packet);

        /**
         * @brief Destructor.
         */
        virtual ~CDebris();

        /**
         * @brief Attribute dispatcher for level configuration.
         * @param packet  Contains debris piece count and particle IDs.
         */
        virtual void HandleAttributes(const CAttributePacket& packet);

        /**
         * @brief Message dispatcher (forwards to CFlotsam).
         * @param msg  Message descriptor.
         */
        virtual void HandleEvents(CMsg& msg);

        /**
         * @brief Triggers explosive fragmentation audio and particle FX.
         */
        virtual void Explode();

    protected:
        RwInt32 m_debrisPieces;             ///< Number of shattered splinter meshes to spawn (offset 0xac).
        RwInt32 m_explosionParticleId;      ///< Associated particle effect template ID (offset 0xb0).
    };
}
