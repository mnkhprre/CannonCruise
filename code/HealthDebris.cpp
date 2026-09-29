#include "HealthDebris.h"
#include "framework/core/attributehandler/cclassfactory.h"
#include "framework/core/attributehandler/csystemcommands.h"
#include "AudioManager.h"

namespace RWS
{
    RWS_REGISTER_CLASS(CHealthDebris);

    CHealthDebris::CHealthDebris(const CAttributePacket& packet)
        : CDebris(packet)
        , m_healthAmount(25.0f)
    {
        m_eventHealthRestored.SetId("EVENT_HEALTH_RESTORED");
    }

    CHealthDebris::~CHealthDebris()
    {
    }

    void CHealthDebris::HandleAttributes(const CAttributePacket& packet)
    {
        CDebris::HandleAttributes(packet);

        CAttributeCommandIterator it(packet, RWS_CLASS_INDEX(CHealthDebris));
        while (!it.IsFinished())
        {
            switch (it->GetCommandId())
            {
            case 0:
                m_healthAmount = *reinterpret_cast<const RwReal*>(it->GetCommandData());
                break;
            default:
                break;
            }
            ++it;
        }
    }

    void CHealthDebris::HandleEvents(CMsg& msg)
    {
        CDebris::HandleEvents(msg);
    }

    void CHealthDebris::OnCollect()
    {
        AudioManager::PlaySound("Audio_HealthPickup");
        CMsg msg(m_eventHealthRestored, &m_healthAmount);
        SendMsg(msg);
        CFlotsam::OnCollect();
    }
}
