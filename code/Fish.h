#pragma once
#include <rwcore.h>
#include <rpworld.h>
#include "framework/core/attributehandler/cattributehandler.h"
#include "framework/core/eventhandler/ceventhandler.h"
#include "PhysicsEntity.h"
#include "AnimController.h"

namespace RWS
{
    class CFish : public CAttributeHandler, public CEventHandler
    {
    public:
        RWS_MAKENEWCLASS(CFish);
        RWS_DECLARE_CLASS(CFish);

        CFish(const CAttributePacket& packet);
        virtual ~CFish();

        virtual void HandleAttributes(const CAttributePacket& packet);
        virtual void HandleEvents(CMsg& msg);

        void AddToWorld();
        void RemoveFromWorld();

        enum FishState
        {
            STATE_SWIM = 0,
            STATE_JUMP,
            STATE_FLEE,
            STATE_DEAD
        };

        void Update(RwReal deltaTime);
        void OnPickup();

        CPhysicsEntity* GetPhysicsEntity() const { return m_pPhysEntity; }
        RpClump* GetClump() const { return m_pClump; }
        CAnimController* GetAnimController() const { return m_pAnimController; }

    protected:
        static RwUInt32 ms_nFishAddedToWorlds;

        CPhysicsEntity* m_pPhysEntity;       // 0x44
        RpClump* m_pClump;                   // 0x48
        CAnimController* m_pAnimController;  // 0x50

        FishState m_state;                   // 0xa0
        RwReal m_swimSpeed;                  // 0xac
        RwReal m_turnRate;                   // 0xb0
        RwReal m_jumpForce;                  // 0x80
        RwReal m_healthValue;                // 0x84
        RwBool m_bAddedToWorld;
    };

    class CFishGreen : public CFish
    {
    public:
        RWS_MAKENEWCLASS(CFishGreen);
        RWS_DECLARE_CLASS(CFishGreen);

        CFishGreen(const CAttributePacket& packet);
        virtual ~CFishGreen();

        virtual void HandleAttributes(const CAttributePacket& packet);
        virtual void HandleEvents(CMsg& msg);
    };
}
