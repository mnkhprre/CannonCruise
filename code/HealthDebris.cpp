// ============================================================================
// CannonCruise - Health Debris Pickup (HealthDebris.cpp)
// Original path: D:\Projects\CannonCruisePC\code\HealthDebris.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include "HealthDebris.h"
#include "framework/core/attributehandler/cclassfactory.h"
#include "framework/core/attributehandler/csystemcommands.h"
#include "AudioManager.h"

namespace RWS
{
    // Register CHealthDebris with RenderWare Studio reflection system
    RWS_REGISTER_CLASS(CHealthDebris);

    // ---------------------------------------------------------------------------
    // Constructor – initializes restorative health value and configures event ID.
    // ---------------------------------------------------------------------------
    CHealthDebris::CHealthDebris(const CAttributePacket& packet)
        : CDebris(packet)
        , m_healthAmount(25.0f)
        , m_eventHealthRestored(0)
    {
    }

    // ---------------------------------------------------------------------------
    // Destructor.
    // ---------------------------------------------------------------------------
    CHealthDebris::~CHealthDebris()
    {
    }

    // ---------------------------------------------------------------------------
    // HandleAttributes – dynamic property updater from workspace / level editor.
    // Command mapping:
    //   0: Health restore amount (RwReal)
    // ---------------------------------------------------------------------------
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

    // ---------------------------------------------------------------------------
    // HandleEvents – forwards framework events to base CDebris.
    // ---------------------------------------------------------------------------
    void CHealthDebris::HandleEvents(CMsg& msg)
    {
        CDebris::HandleEvents(msg);
    }

    // ---------------------------------------------------------------------------
    // OnCollect – plays "Audio_HealthPickup", dispatches EVENT_HEALTH_RESTORED
    // with restorative amount to listeners, and completes collection cleanup.
    // ---------------------------------------------------------------------------
    void CHealthDebris::OnCollect()
    {
        CAudioManager* pAudio = CAudioManager::GetSingletonPtr();
        if (pAudio)
        {
            pAudio->PlaySound("Audio_HealthPickup");
        }
        CMsg msg(m_eventHealthRestored, &m_healthAmount);
        CEventHandler::SendMsg(msg);
        CFlotsam::OnCollect();
    }
}

