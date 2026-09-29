#pragma once
#include <rwcore.h>
#include <rpworld.h>
#include "framework/core/attributehandler/cattributehandler.h"
#include "framework/core/eventhandler/ceventhandler.h"
#include "PhysicsEntity.h"
#include "AnimController.h"
#include "AudioManager.h"

namespace RWS
{
    class CSoldierAi;

    class CSoldier : public CAttributeHandler, public CEventHandler
    {
    public:
        RWS_MAKENEWCLASS(CSoldier);
        RWS_DECLARE_CLASS(CSoldier);

        CSoldier(const CAttributePacket& packet);
        virtual ~CSoldier();

        virtual void HandleAttributes(const CAttributePacket& packet);
        virtual void HandleEvents(CMsg& msg);

        void AddToWorld();
        void RemoveFromWorld();

        enum SoldierType
        {
            TYPE_NORMAL = 0,
            TYPE_GUARD,
            TYPE_CAPTAIN
        };

        void TakeDamage(RwReal damage);
        void Update(RwReal deltaTime);

        CPhysicsEntity* GetPhysicsEntity() const { return m_pPhysEntity; }
        RpClump* GetClump() const { return m_pClump; }
        CAnimController* GetAnimController() const { return m_pAnimController; }

    private:
        static RwUInt32 ms_nSoldiersAddedToWorlds;

        CPhysicsEntity* m_pPhysEntity;       // 0x54 / 0x6c
        RpClump* m_pClump;                   // 0x70
        CAnimController* m_pAnimController;  // 0x74
        CSoldierAi* m_pAi;                   // 0x78

        RwReal m_health;                     // 0x88
        RwReal m_speed;                      // 0x8c
        RwReal m_turnRate;                   // 0x9c
        SoldierType m_soldierType;           // 0xf0
        RwBool m_bAddedToWorld;
    };

    class CSoldierAi
    {
    public:
        CSoldierAi(CSoldier* pSoldier);
        virtual ~CSoldierAi();

        enum State
        {
            STATE_IDLE = 0,
            STATE_PATROL,
            STATE_CHASE,
            STATE_ATTACK,
            STATE_DEAD
        };

        void Update(RwReal deltaTime);
        void SetState(State state);
        State GetState() const { return m_state; }

    private:
        CSoldier* m_pSoldier;
        State m_state;
        RwReal m_stateTimer;
        RwV3d m_targetPos;
    };
}
