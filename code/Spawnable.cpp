#include "Spawnable.h"
#include "framework/core/attributehandler/cclassfactory.h"
#include "framework/core/attributehandler/csystemcommands.h"

namespace RWS
{
    RWS_REGISTER_CLASS(CSpawnable);

    CSpawnable::CSpawnable(const CAttributePacket& packet)
        : CAttributeHandler(packet)
        , CEventHandler(0)
        , m_bActive(FALSE)
        , m_bSpawned(FALSE)
        , m_spawnDelay(0.0f)
        , m_spawnTimer(0.0f)
    {
        RegisterForMessage(iMsgRunningTick);
    }

    CSpawnable::~CSpawnable()
    {
        UnregisterForMessage(iMsgRunningTick);
    }

    void CSpawnable::HandleAttributes(const CAttributePacket& packet)
    {
        CAttributeHandler::HandleAttributes(packet);

        CAttributeCommandIterator it(packet, RWS_CLASS_INDEX(CSpawnable));
        while (!it.IsFinished())
        {
            switch (it->GetCommandId())
            {
            case 0:
                m_spawnDelay = *reinterpret_cast<const RwReal*>(it->GetCommandData());
                break;
            default:
                break;
            }
            ++it;
        }
    }

    void CSpawnable::HandleEvents(CMsg& msg)
    {
        if (msg.Id == iMsgRunningTick)
        {
            if (m_bActive && !m_bSpawned)
            {
                m_spawnTimer += 0.0166667f;
                if (m_spawnTimer >= m_spawnDelay)
                {
                    Spawn();
                }
            }
        }
    }

    void CSpawnable::Activate()
    {
        RWS_ASSERT(!m_bActive, "Spawnable already active");
        m_bActive = TRUE;
        m_spawnTimer = 0.0f;
    }

    void CSpawnable::Deactivate()
    {
        RWS_ASSERT(m_bActive, "Spawnable not active");
        m_bActive = FALSE;
        if (m_bSpawned)
        {
            Despawn();
        }
    }

    void CSpawnable::Spawn()
    {
        RWS_ASSERT(!m_bSpawned, "Spawnable already spawned");
        m_bSpawned = TRUE;
    }

    void CSpawnable::Despawn()
    {
        RWS_ASSERT(m_bSpawned, "Spawnable not spawned");
        m_bSpawned = FALSE;
    }

    RwBool CSpawnable::IsAlive() const
    {
        return m_bActive && m_bSpawned;
    }
}
