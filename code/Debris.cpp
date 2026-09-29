#include "Debris.h"
#include "framework/core/attributehandler/cclassfactory.h"
#include "framework/core/attributehandler/csystemcommands.h"
#include "ParticleEmitter.h"
#include "AudioManager.h"

namespace RWS
{
    RWS_REGISTER_CLASS(CDebris);

    CDebris::CDebris(const CAttributePacket& packet)
        : CFlotsam(packet)
        , m_debrisPieces(4)
        , m_explosionParticleId(-1)
    {
    }

    CDebris::~CDebris()
    {
    }

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

    void CDebris::HandleEvents(CMsg& msg)
    {
        CFlotsam::HandleEvents(msg);
    }

    void CDebris::Explode()
    {
        AudioManager::PlaySound("Audio_CannonHit");
        OnCollect();
    }
}
