#pragma once
#include <rwcore.h>
#include <rpworld.h>
#include "framework/core/attributehandler/cattributehandler.h"
#include "framework/core/eventhandler/ceventhandler.h"
#include "CollisionTrigger.h"
#include "PhysicsEntity.h"

namespace RWS
{
    class CFlotsam : public CAttributeHandler, public CEventHandler, public CollisionTrigger::CCollisionTrigger
    {
    public:
        RWS_MAKENEWCLASS(CFlotsam);
        RWS_DECLARE_CLASS(CFlotsam);

        CFlotsam(const CAttributePacket& packet);
        virtual ~CFlotsam();

        virtual void HandleAttributes(const CAttributePacket& packet);
        virtual void HandleEvents(CMsg& msg);

        virtual void OnCollision(const CollisionTrigger::CollisionInfo& info);
        virtual void OnCollect();

        void AddToWorld();
        void RemoveFromWorld();

        RpClump* GetClump() const { return m_pClump; }
        CPhysicsEntity* GetPhysicsEntity() const { return m_pPhysEntity; }

    protected:
        static RwUInt32 ms_nFlotsamsAddedToWorlds;

        CPhysicsEntity* m_pPhysEntity;       // 0x54
        RpClump* m_pClump;                   // 0x6c
        RwReal m_buoyancy;                   // 0x88
        RwReal m_rotationSpeed;              // 0x8c
        RwReal m_lifetime;                   // 0x90
        RwBool m_bAddedToWorld;
        RwBool m_bActive;
    };
}
