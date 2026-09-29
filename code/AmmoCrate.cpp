#include "AmmoCrate.h"
#include "framework/core/attributehandler/cclassfactory.h"
#include "framework/core/attributehandler/csystemcommands.h"
#include "AudioManager.h"

namespace RWS
{
    RWS_REGISTER_CLASS(CAmmoCrate);

    CAmmoCrate::CAmmoCrate(const CAttributePacket& packet)
        : CFlotsam(packet)
        , m_ammoType(0)
        , m_ammoCount(5)
    {
        m_eventAmmoCollected.SetId("EVENT_AMMO_COLLECTED");
    }

    CAmmoCrate::~CAmmoCrate()
    {
    }

    void CAmmoCrate::HandleAttributes(const CAttributePacket& packet)
    {
        CFlotsam::HandleAttributes(packet);

        CAttributeCommandIterator it(packet, RWS_CLASS_INDEX(CAmmoCrate));
        while (!it.IsFinished())
        {
            switch (it->GetCommandId())
            {
            case 0:
                m_ammoType = *reinterpret_cast<const RwInt32*>(it->GetCommandData());
                break;
            case 1:
                m_ammoCount = *reinterpret_cast<const RwInt32*>(it->GetCommandData());
                break;
            default:
                break;
            }
            ++it;
        }
    }

    void CAmmoCrate::HandleEvents(CMsg& msg)
    {
        CFlotsam::HandleEvents(msg);
    }

    void CAmmoCrate::OnCollect()
    {
        AudioManager::PlaySound("Audio_HealthPickup");
        CMsg msg(m_eventAmmoCollected, &m_ammoType);
        SendMsg(msg);
        CFlotsam::OnCollect();
    }
}
