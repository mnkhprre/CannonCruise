#include "Trigger.h"
#include "framework/core/attributehandler/cclassfactory.h"
#include "framework/core/attributehandler/csystemcommands.h"

namespace RWS
{
    RWS_REGISTER_CLASS(CTrigger);

    CTrigger::CTrigger(const CAttributePacket& packet)
        : CAttributeHandler(packet)
        , CEventHandler(0)
        , m_triggerRadius(5.0f)
        , m_bOneShot(FALSE)
        , m_bTriggered(FALSE)
        , m_bInsideTrigger(FALSE)
        , m_speakId(-1)
    {
        m_triggerPos.x = 0.0f;
        m_triggerPos.y = 0.0f;
        m_triggerPos.z = 0.0f;

        RegisterForMessage(iMsgRunningTick);
    }

    CTrigger::~CTrigger()
    {
        UnregisterForMessage(iMsgRunningTick);
    }

    void CTrigger::HandleAttributes(const CAttributePacket& packet)
    {
        CAttributeHandler::HandleAttributes(packet);

        CAttributeCommandIterator it(packet, RWS_CLASS_INDEX(CTrigger));
        while (!it.IsFinished())
        {
            switch (it->GetCommandId())
            {
            case 0:
                m_triggerRadius = *reinterpret_cast<const RwReal*>(it->GetCommandData());
                break;
            case 1:
                m_bOneShot = *reinterpret_cast<const RwInt32*>(it->GetCommandData()) != 0;
                break;
            case 2:
                m_speakId = *reinterpret_cast<const RwInt32*>(it->GetCommandData());
                break;
            default:
                break;
            }
            ++it;
        }
    }

    void CTrigger::HandleEvents(CMsg& msg)
    {
        if (msg.Id == iMsgRunningTick)
        {
            // Distance check against player would happen here
        }
    }

    void CTrigger::OnEnter()
    {
        if (m_bOneShot && m_bTriggered)
            return;

        m_bTriggered = TRUE;
        m_bInsideTrigger = TRUE;

        CMsg msg(m_eventOnEnter, 0);
        SendMsg(msg);

        if (m_speakId >= 0)
        {
            CSpeakManager* pSpeakMgr = CSpeakManager::GetSingleton();
            RWS_ASSERT(pSpeakMgr != 0, "CSpeakManager::GetSingleton()->IsValid()");
            if (pSpeakMgr)
            {
                pSpeakMgr->PlaySpeech(m_speakId);
            }
        }
    }

    void CTrigger::OnExit()
    {
        m_bInsideTrigger = FALSE;

        CMsg msg(m_eventOnExit, 0);
        SendMsg(msg);
    }
}
