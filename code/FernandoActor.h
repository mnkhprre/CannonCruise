#pragma once
#include "Actor.h"

namespace RWS
{
    class CFernandoActor : public CActor
    {
    public:
        RWS_MAKENEWCLASS(CFernandoActor);
        RWS_DECLARE_CLASS(CFernandoActor);

        CFernandoActor(const CAttributePacket& packet);
        virtual ~CFernandoActor();

        virtual void HandleAttributes(const CAttributePacket& packet);
        virtual void HandleEvents(CMsg& msg);

        enum AnimState
        {
            ANIM_IDLE_01 = 0,
            ANIM_IDLE_02,
            ANIM_HIT,
            ANIM_JUMP_L,
            ANIM_JUMP_R,
            ANIM_FLY_LOOP,
            ANIM_TURN,
            ANIM_BINOCULAR_UP,
            ANIM_BINOCULAR_SEARCH,
            ANIM_BINOCULAR_DOWN,
            ANIM_COUNT
        };

        void PlayAnimation(AnimState state, RwReal speed = 1.0f);
        void SetMastVisibility(RwBool visible);

    private:
        CEventId m_eventChangeMastVisibility;       // 0x30
        CEventId m_eventPlayerShipDeath;            // 0x38
        CEventId m_eventPlayerShipActorTakeDamage;  // 0x40
        CEventId m_eventInqPlayerShipTurnDamper;    // 0x48
        CEventId m_eventPlayerShipDetachAllFrames;  // 0x50
        CEventId m_eventPlayerShipAddFernandoFrame; // 0x58
        CEventId m_eventCustom;                     // 0x60

        RwReal m_animSpeeds[ANIM_COUNT];            // 0x94 - 0xb0
        RwInt32 m_animIndices[ANIM_COUNT];          // 0xc8 - 0xe4
        RwInt32 m_currentState;                     // 0xb4
        RwInt32 m_mastVisible;                      // 0xbc
        RwReal m_idleTimer;                         // 0x90
    };
}
