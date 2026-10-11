#pragma once

// ============================================================================
// CannonCruise - Soldier NPC & Soldier AI (Soldier.h)
// Original path: D:\Projects\CannonCruisePC\code\Soldier.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

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

    /**
     * @class CSoldier
     * @brief Ground-based garrison troop / NPC soldier defending islands and fortifications.
     *
     * Integrates with RenderWare Studio attribute handling (CAttributeHandler)
     * and event notification (CEventHandler). Employs skeletal animation
     * via CAnimController (controlling meshes such as "Bip_Beaver_Soldier" and "Viking"),
     * delegates behavior to an attached CSoldierAi state machine, and handles combat damage
     * and sound playback (Audio_HitGuard, Audio_Step).
     */
    class CSoldier : public CAttributeHandler, public CEventHandler
    {
    public:
        RWS_MAKENEWCLASS(CSoldier);
        RWS_DECLARE_CLASS(CSoldier);

        /**
         * @brief Constructs a soldier instance from level attribute packet.
         * @param packet  Attribute initialization packet.
         */
        CSoldier(const CAttributePacket& packet);

        /**
         * @brief Destructor – deregisters events, decrements global world counter,
         * releases AI controller, anim controller, and 3D clump.
         */
        virtual ~CSoldier();

        /**
         * @brief Attribute dispatcher for level editor configuration.
         * @param packet  Command packet configuring health, speeds, type.
         */
        virtual void HandleAttributes(const CAttributePacket& packet);

        /**
         * @brief Framework message dispatcher (handles iMsgRunningTick).
         * @param msg  Message descriptor.
         */
        virtual void HandleEvents(CMsg& msg);

        /**
         * @brief Adds the soldier clump to the active CScene RpWorld.
         */
        void AddToWorld();

        /**
         * @brief Removes the soldier clump from the active CScene RpWorld.
         */
        void RemoveFromWorld();

        /**
         * @enum SoldierType
         * @brief Character role and strength archetype.
         */
        enum SoldierType
        {
            TYPE_NORMAL = 0,    ///< Standard foot infantry.
            TYPE_GUARD,         ///< Fortification sentinel / sentry.
            TYPE_CAPTAIN        ///< Squad leader with heightened durability.
        };

        /**
         * @brief Applies damage to soldier health, plays impact sound, and transitions to DEAD if zero.
         * @param damage  Health points to deduct.
         */
        void TakeDamage(RwReal damage);

        /**
         * @brief Frame tick updating animation controller and AI state machine.
         * @param deltaTime  Elapsed frame time in seconds.
         */
        void Update(RwReal deltaTime);

        /** @brief Returns pointer to optional physics entity wrapper. */
        CPhysicsEntity* GetPhysicsEntity() const { return m_pPhysEntity; }

        /** @brief Returns pointer to the 3D RenderWare model clump. */
        RpClump* GetClump() const { return m_pClump; }

        /** @brief Returns pointer to the skeletal animation controller. */
        CAnimController* GetAnimController() const { return m_pAnimController; }

    private:
        static RwUInt32 ms_nSoldiersAddedToWorlds; ///< Global counter of active soldiers in simulation.

        CPhysicsEntity* m_pPhysEntity;       ///< Physics entity proxy (offset 0x54 / 0x6c).
        RpClump* m_pClump;                   ///< 3D character clump (offset 0x70).
        CAnimController* m_pAnimController;  ///< Skeletal animation controller (offset 0x74).
        CSoldierAi* m_pAi;                   ///< Behavioral AI controller instance (offset 0x78).

        RwReal m_health;                     ///< Current hit points (offset 0x88).
        RwReal m_speed;                      ///< Ground movement velocity (offset 0x8c).
        RwReal m_turnRate;                   ///< Turning angular rate (offset 0x9c).
        SoldierType m_soldierType;           ///< Archetype classification (offset 0xf0).
        RwBool m_bAddedToWorld;              ///< Scene world presence flag.
    };

    /**
     * @class CSoldierAi
     * @brief Autonomous tactical finite state machine governing infantry NPCs.
     */
    class CSoldierAi
    {
    public:
        /**
         * @brief Constructs an AI controller bound to a soldier entity.
         * @param pSoldier  Host soldier instance.
         */
        CSoldierAi(CSoldier* pSoldier);

        /**
         * @brief Destructor.
         */
        virtual ~CSoldierAi();

        /**
         * @enum State
         * @brief Operational AI behaviors.
         */
        enum State
        {
            STATE_IDLE = 0,     ///< Standing guard / idle animation.
            STATE_PATROL,       ///< Moving along designated perimeter path.
            STATE_CHASE,        ///< Advancing toward hostile target.
            STATE_ATTACK,       ///< Performing melee or ranged attack action.
            STATE_DEAD          ///< Incapacitated / death animation state.
        };

        /**
         * @brief Advances AI timers and evaluates state transitions.
         * @param deltaTime  Elapsed frame duration in seconds.
         */
        void Update(RwReal deltaTime);

        /**
         * @brief Changes current state and resets state duration timer.
         * @param state  New behavioral state.
         */
        void SetState(State state);

        /**
         * @brief Queries current AI state.
         * @return Active State enum value.
         */
        State GetState() const { return m_state; }

    private:
        CSoldier* m_pSoldier;   ///< Back-reference to the parent soldier entity.
        State m_state;          ///< Active behavioral state.
        RwReal m_stateTimer;    ///< Timer tracking time spent in the current state.
        RwV3d m_targetPos;      ///< Current destination or target world coordinates.
    };
}
