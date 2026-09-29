#pragma once
#include <rwcore.h>
#include <rpworld.h>
#include "framework/core/attributehandler/cattributehandler.h"
#include "framework/core/eventhandler/ceventhandler.h"
#include "CollisionTrigger.h"
#include "SpeakManager.h"

namespace RWS
{
    /**
     * CTrigger - Area trigger that fires events when entities enter/exit.
     * Integrates with CSpeakManager for dialogue triggers.
     * Binary ref: strings at 0x0061947c, xrefs FUN_004bf390 - FUN_004bfe50
     */
    class CTrigger : public CAttributeHandler, public CEventHandler
    {
    public:
        RWS_MAKENEWCLASS(CTrigger);
        RWS_DECLARE_CLASS(CTrigger);

        CTrigger(const CAttributePacket& packet);
        virtual ~CTrigger();

        virtual void HandleAttributes(const CAttributePacket& packet);
        virtual void HandleEvents(CMsg& msg);

        void OnEnter();
        void OnExit();

    private:
        CEventId m_eventOnEnter;
        CEventId m_eventOnExit;
        CEventId m_eventOnStay;

        RwV3d m_triggerPos;
        RwReal m_triggerRadius;
        RwBool m_bOneShot;
        RwBool m_bTriggered;
        RwBool m_bInsideTrigger;

        RwInt32 m_speakId;       // Speech ID to play on trigger
    };
}
