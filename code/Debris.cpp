// ============================================================================
// CannonCruise - Debris Obstacle & Explosive Flotsam (Debris.cpp)
// Original path: D:\Projects\CannonCruisePC\code\Debris.cpp
// Reverse-engineered via Ghidra and CC.asm analysis.
// ============================================================================

#include "Debris.h"
#include "framework/core/attributehandler/cclassfactory.h"
#include "framework/core/attributehandler/csystemcommands.h"
#include "ParticleEmitter.h"
#include "AudioManager.h"

namespace RWS
{
    // Register CDebris with RenderWare Studio reflection system
    RWS_REGISTER_CLASS(CDebris);

    // ---------------------------------------------------------------------------
    // Constructor – initializes default splinter count and particle template.
    // ---------------------------------------------------------------------------
    CDebris::CDebris(const CAttributePacket& packet)
        : CFlotsam(packet)
        , m_debrisPieces(4)
        , m_explosionParticleId(-1)
    {
    }

    // ---------------------------------------------------------------------------
    // Destructor.
    // ---------------------------------------------------------------------------
    CDebris::~CDebris()
    {
    }

    // ---------------------------------------------------------------------------
    // HandleAttributes – dynamic property updater from workspace / level editor.
    // Command mapping:
    //   0: Number of splinter pieces (RwInt32)
    //   1: Explosion particle effect template ID (RwInt32)
    // ---------------------------------------------------------------------------
    void CDebris::HandleAttributes(const CAttributePacket& packet)
    {
        CFlotsam::HandleAttributes(packet);

        CAttributeCommandIterator it(packet, RWS_CLASS_INDEX(CDebris));
        while (!it.IsFinished())
        {
            switch (it->GetCommandId())
            {
            case 0:
                m_debrisPieces = *reinterpret_cast<const RwInt32*>(it->GetCommandData());
                break;
            case 1:
                m_explosionParticleId = *reinterpret_cast<const RwInt32*>(it->GetCommandData());
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
    void CDebris::HandleEvents(CMsg& msg)
    {
        CFlotsam::HandleEvents(msg);
    }

    // ---------------------------------------------------------------------------
    // Explode – triggers impact sound "Audio_CannonHit" and cleans up entity.
    // ---------------------------------------------------------------------------
    void CDebris::Explode()
    {
        CAudioManager* pAudio = CAudioManager::GetSingletonPtr();
        if (pAudio)
        {
            pAudio->PlaySound("Audio_CannonHit");
        }
        OnCollect();
    }
}
