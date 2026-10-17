#pragma once

// ============================================================================
// CannonCruise - Ambient Bird & Seagull Entity (Bird.h)
// Original path: D:\Projects\CannonCruisePC\code\Bird.h
// Reverse-engineered via Ghidra and CC.asm analysis.
// ============================================================================

#include <rwcore.h>
#include <rpworld.h>
#include "framework/core/attributehandler/cattributehandler.h"
#include "framework/core/eventhandler/ceventhandler.h"
#include "PhysicsEntity.h"
#include "AnimController.h"

namespace RWS
{
    /**
     * @class CBird
     * @brief Base ambient flying entity simulating avian circular flight patterns.
     *
     * Integrates with RenderWare Studio attribute handling (CAttributeHandler)
     * and event notification (CEventHandler). Animates 3D bird meshes (clumps)
     * using CAnimController, calculates trigonometric orbital flight trajectories
     * around an origin anchor point, and maintains active instance tracking.
     */
    class CBird : public CAttributeHandler, public CEventHandler
    {
    public:
        RWS_MAKENEWCLASS(CBird);
        RWS_DECLARE_CLASS(CBird);

        /**
         * @brief Constructs a bird entity configured from level design attributes.
         * @param packet  Attribute packet containing clump, speeds, and flight radii.
         */
        CBird(const CAttributePacket& packet);

        /**
         * @brief Destructor – removes bird from scene world and releases resources.
         */
        virtual ~CBird();

        /**
         * @brief Processes engine attribute commands to tune flight parameters dynamically.
         * @param packet  Packet containing command IDs for height, speed, radius, turn rate.
         */
        virtual void HandleAttributes(const CAttributePacket& packet);

        /**
         * @brief Framework message dispatcher (listens for iMsgRunningTick).
         * @param msg  Message descriptor.
         */
        virtual void HandleEvents(CMsg& msg);

        /**
         * @brief Inserts the bird's clump into the RenderWare scene world.
         */
        void AddToWorld();

        /**
         * @brief Removes the bird's clump from the RenderWare scene world.
         */
        void RemoveFromWorld();

        /**
         * @brief Top-level per-frame update for flight motion and animation playback.
         * @param deltaTime  Frame delta time in seconds.
         */
        void Update(RwReal deltaTime);

        /**
         * @brief Calculates updated circular coordinates and updates frame transform.
         * @param deltaTime  Frame delta time in seconds.
         */
        void UpdateFlight(RwReal deltaTime);

        /** @brief Returns pointer to optional physics entity wrapper. */
        CPhysicsEntity* GetPhysicsEntity() const { return m_pPhysEntity; }

        /** @brief Returns pointer to the 3D RenderWare model clump. */
        RpClump* GetClump() const { return m_pClump; }

        /** @brief Returns pointer to the hierarchical skeletal animation controller. */
        CAnimController* GetAnimController() const { return m_pAnimController; }

    protected:
        static RwUInt32 ms_nBirdsAddedToWorlds; ///< Global counter of active birds in simulation.

        CPhysicsEntity* m_pPhysEntity;       ///< Optional physics collision proxy (offset 0x54/0x68).
        RpClump* m_pClump;                   ///< Owned RenderWare clump model (offset 0x6c).
        CAnimController* m_pAnimController;  ///< Skeletal animation controller (offset 0x70).

        RwV3d m_originPos;                   ///< Center anchor position of circling orbit (offset 0xb0-0xb8).
        RwReal m_flightSpeed;                ///< Flight cruising velocity (offset 0x94).
        RwReal m_flightHeight;               ///< Altitude offset above origin (offset 0x90).
        RwReal m_turnRate;                   ///< Angular orbital velocity in radians/sec (offset 0x98).
        RwReal m_circleRadius;               ///< Radius of circling trajectory in world units (offset 0x9c).
        RwReal m_angle;                      ///< Current orbital angle in radians [0..2pi] (offset 0xa0).
        RwInt32 m_flyingState;               ///< Flight state flags/mode (offset 0xa4).
        RwBool m_bAddedToWorld;              ///< World residency flag.
    };

    /**
     * @class CBirdSeaGull
     * @brief Specialized coastal seagull entity with custom visual and audio behaviors.
     */
    class CBirdSeaGull : public CBird
    {
    public:
        RWS_MAKENEWCLASS(CBirdSeaGull);
        RWS_DECLARE_CLASS(CBirdSeaGull);

        /**
         * @brief Constructs a seagull entity instance from level attributes.
         * @param packet  Attribute initialization packet.
         */
        CBirdSeaGull(const CAttributePacket& packet);

        /**
         * @brief Virtual destructor.
         */
        virtual ~CBirdSeaGull();

        /**
         * @brief Attribute dispatcher for seagull-specific properties.
         * @param packet  Attribute packet.
         */
        virtual void HandleAttributes(const CAttributePacket& packet);

        /**
         * @brief Event message dispatcher for seagull entity.
         * @param msg  Incoming message.
         */
        virtual void HandleEvents(CMsg& msg);
    };
}
