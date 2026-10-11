#pragma once

// ============================================================================
// CannonCruise - Flotsam Environmental Entity (Flotsam.h)
// Original path: D:\Projects\CannonCruisePC\code\Flotsam.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include <rwcore.h>
#include <rpworld.h>
#include "framework/core/attributehandler/cattributehandler.h"
#include "framework/core/eventhandler/ceventhandler.h"
#include "CollisionTrigger.h"
#include "PhysicsEntity.h"

namespace RWS
{
    /**
     * @class CFlotsam
     * @brief Floating ocean wreckage and interactive pickup base entity.
     *
     * Serves as the base class for collectable floating items (health crates,
     * lifebuoys, ammo boxes, and floating explosive debris). Subscribes to engine
     * frame ticks (iMsgRunningTick), queries ocean wave elevation to float
     * realistically on the sea surface, and participates in collision triggering
     * through CollisionTrigger::CCollisionTrigger.
     */
    class CFlotsam : public CAttributeHandler, public CollisionTrigger::CCollisionTrigger
    {
    public:
        RWS_MAKENEWCLASS(CFlotsam);
        RWS_DECLARE_CLASS(CFlotsam);

        /**
         * @brief Constructs a flotsam object from attribute packet.
         * @param packet  Attribute initialization packet.
         */
        CFlotsam(const CAttributePacket& packet);

        /**
         * @brief Destructor – cleans up RpClump and validates active world counters.
         */
        virtual ~CFlotsam();

        /**
         * @brief Attribute dispatcher for level editor configuration.
         * @param packet  Command packet containing buoyancy and rotation settings.
         */
        virtual void HandleAttributes(const CAttributePacket& packet);

        /**
         * @brief Framework message dispatcher (processes iMsgRunningTick).
         * @param msg  Message descriptor.
         */
        virtual void HandleEvents(CMsg& msg);

        /**
         * @brief Callback invoked when a ship hull or player touches this flotsam.
         * @param info  Collision intersection parameters.
         */
        virtual void OnCollision(const CollisionTrigger::CollisionInfo& info);

        /**
         * @brief Collection handler – deactivates entity, removes from world, and triggers pickups.
         */
        virtual void OnCollect();

        /**
         * @brief Inserts the flotsam 3D clump into the active CScene RpWorld.
         */
        void AddToWorld();

        /**
         * @brief Removes the flotsam clump from the active CScene RpWorld.
         */
        void RemoveFromWorld();

        /** @brief Returns pointer to owned RenderWare 3D clump. */
        RpClump* GetClump() const { return m_pClump; }

        /** @brief Returns pointer to optional physics entity wrapper. */
        CPhysicsEntity* GetPhysicsEntity() const { return m_pPhysEntity; }

    protected:
        static RwUInt32 ms_nFlotsamsAddedToWorlds; ///< Global counter of active flotsam instances.

        CPhysicsEntity* m_pPhysEntity;       ///< Physics entity proxy (offset 0x54).
        RpClump* m_pClump;                   ///< 3D mesh model clump (offset 0x6c).
        RwReal m_buoyancy;                   ///< Buoyancy float coefficient (offset 0x88).
        RwReal m_rotationSpeed;              ///< Idle bobbing yaw rotation speed in rad/s (offset 0x8c).
        RwReal m_lifetime;                   ///< Despawn lifetime duration if temporary (offset 0x90).
        RwBool m_bAddedToWorld;              ///< Flag indicating world residency.
        RwBool m_bActive;                    ///< Active state flag; FALSE once collected.
    };
}
