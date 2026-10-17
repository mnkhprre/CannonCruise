// ============================================================================
// CannonCruise - Ammo Crate Pickup (AmmoCrate.cpp)
// Original path: D:\Projects\CannonCruisePC\code\AmmoCrate.cpp
// Reverse-engineered via Ghidra and CC.asm analysis.
// ============================================================================

#include "AmmoCrate.h"
#include "framework/core/attributehandler/cclassfactory.h"
#include "framework/core/attributehandler/csystemcommands.h"
#include "AudioManager.h"

namespace RWS
{
    // Register CAmmoCrate with RenderWare Studio reflection system
    RWS_REGISTER_CLASS(CAmmoCrate);

    // ---------------------------------------------------------------------------
    // Constructor – initializes projectile ammunition bundle parameters.
    // ---------------------------------------------------------------------------
    CAmmoCrate::CAmmoCrate(const CAttributePacket& packet)
        : CFlotsam(packet)
        , m_ammoType(0)
        , m_ammoCount(5)
        , m_eventAmmoCollected(0)
    {
    }

    // ---------------------------------------------------------------------------
    // Destructor.
    // ---------------------------------------------------------------------------
    CAmmoCrate::~CAmmoCrate()
    {
    }

    // ---------------------------------------------------------------------------
    // HandleAttributes – dynamic property updater from workspace / level editor.
    // Command mapping:
    //   0: Projectile ammo type index (RwInt32)
    //   1: Ammunition count provided (RwInt32)
    // ---------------------------------------------------------------------------
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

    // ---------------------------------------------------------------------------
    // HandleEvents – forwards framework events to base CFlotsam.
    // ---------------------------------------------------------------------------
    void CAmmoCrate::HandleEvents(CMsg& msg)
    {
        CFlotsam::HandleEvents(msg);
    }

    // ---------------------------------------------------------------------------
    // OnCollect – plays sound effect, emits EVENT_AMMO_COLLECTED with ammo type,
    // and clears flotsam entity from simulation.
    // ---------------------------------------------------------------------------
    void CAmmoCrate::OnCollect()
    {
        CAudioManager* pAudio = CAudioManager::GetSingletonPtr();
        if (pAudio)
        {
            pAudio->PlaySound("Audio_HealthPickup");
        }
        CMsg msg(m_eventAmmoCollected, &m_ammoType);
        CEventHandler::SendMsg(msg);
        CFlotsam::OnCollect();
    }
}

