#pragma once
#include <rwcore.h>
#include <rpworld.h>
#include "framework/core/attributehandler/cattributehandler.h"
#include "framework/core/eventhandler/ceventhandler.h"
#include "PhysicsEntity.h"
#include "AnimController.h"

namespace RWS
{
    class CBird : public CAttributeHandler, public CEventHandler
    {
    public:
        RWS_MAKENEWCLASS(CBird);
        RWS_DECLARE_CLASS(CBird);

        CBird(const CAttributePacket& packet);
        virtual ~CBird();

        virtual void HandleAttributes(const CAttributePacket& packet);
        virtual void HandleEvents(CMsg& msg);

        void AddToWorld();
        void RemoveFromWorld();

        void Update(RwReal deltaTime);
        void UpdateFlight(RwReal deltaTime);

        CPhysicsEntity* GetPhysicsEntity() const { return m_pPhysEntity; }
        RpClump* GetClump() const { return m_pClump; }
        CAnimController* GetAnimController() const { return m_pAnimController; }

    protected:
        static RwUInt32 ms_nBirdsAddedToWorlds;

        CPhysicsEntity* m_pPhysEntity;       // 0x54 / 0x68
        RpClump* m_pClump;                   // 0x6c
        CAnimController* m_pAnimController;  // 0x70

        RwV3d m_originPos;                   // 0xb0 - 0xb8
        RwReal m_flightSpeed;                // 0x94
        RwReal m_flightHeight;               // 0x90
        RwReal m_turnRate;                   // 0x98
        RwReal m_circleRadius;               // 0x9c
        RwReal m_angle;                      // 0xa0
        RwInt32 m_flyingState;               // 0xa4
        RwBool m_bAddedToWorld;
    };

    class CBirdSeaGull : public CBird
    {
    public:
        RWS_MAKENEWCLASS(CBirdSeaGull);
        RWS_DECLARE_CLASS(CBirdSeaGull);

        CBirdSeaGull(const CAttributePacket& packet);
        virtual ~CBirdSeaGull();

        virtual void HandleAttributes(const CAttributePacket& packet);
        virtual void HandleEvents(CMsg& msg);
    };
}
