#include "Flotsam.h"
#include "framework/core/attributehandler/cclassfactory.h"
#include "framework/core/attributehandler/csystemcommands.h"
#include "framework/toolkits/world/helpers/clumphelper.h"
#include "framework/core/resourcemanager/cresourcemanager.h"
#include "Scene.h"
#include "Ocean.h"

namespace RWS
{
    RWS_REGISTER_CLASS(CFlotsam);

    RwUInt32 CFlotsam::ms_nFlotsamsAddedToWorlds = 0;

    CFlotsam::CFlotsam(const CAttributePacket& packet)
        : CAttributeHandler(packet)
        , CEventHandler(0)
        , m_pPhysEntity(0)
        , m_pClump(0)
        , m_buoyancy(1.0f)
        , m_rotationSpeed(0.5f)
        , m_lifetime(0.0f)
        , m_bAddedToWorld(FALSE)
        , m_bActive(TRUE)
    {
        const void* pClumpData = CSystemCommands::ExtractClump(packet);
        if (pClumpData)
        {
            m_pClump = ClumpHelper::CreateClumpFromResource(pClumpData);
        }

        RegisterForMessage(iMsgRunningTick);
        AddToWorld();
    }

    CFlotsam::~CFlotsam()
    {
        UnregisterForMessage(iMsgRunningTick);

        if (m_bAddedToWorld)
        {
            RWS_ASSERT(ms_nFlotsamsAddedToWorlds != 0, "ms_nFlotsamsAddedToWorlds != 0 Failed");
            --ms_nFlotsamsAddedToWorlds;
        }

        RemoveFromWorld();

        if (m_pClump)
        {
            ClumpHelper::DestroyClump(m_pClump);
            m_pClump = 0;
        }
    }

    void CFlotsam::HandleAttributes(const CAttributePacket& packet)
    {
        CAttributeHandler::HandleAttributes(packet);

        if (m_pClump)
        {
            ClumpHelper::HandleAttributes(m_pClump, packet);
        }

        CAttributeCommandIterator it(packet, RWS_CLASS_INDEX(CFlotsam));
        while (!it.IsFinished())
        {
            switch (it->GetCommandId())
            {
            case 0:
                m_buoyancy = *reinterpret_cast<const RwReal*>(it->GetCommandData());
                break;
            case 1:
                m_rotationSpeed = *reinterpret_cast<const RwReal*>(it->GetCommandData());
                break;
            default:
                break;
            }
            ++it;
        }
    }

    void CFlotsam::HandleEvents(CMsg& msg)
    {
        if (msg.Id == iMsgRunningTick)
        {
            if (m_pClump && m_bActive)
            {
                RwFrame* pFrame = RpClumpGetFrame(m_pClump);
                if (pFrame)
                {
                    RwMatrix* pMatrix = RwFrameGetMatrix(pFrame);
                    RwReal waveHeight = COcean::GetWaveHeight(pMatrix->pos.x, pMatrix->pos.z);
                    pMatrix->pos.y = waveHeight;
                    RwFrameUpdateObjects(pFrame);
                }
            }
        }
    }

    void CFlotsam::OnCollision(const CollisionTrigger::CollisionInfo& info)
    {
        OnCollect();
    }

    void CFlotsam::OnCollect()
    {
        m_bActive = FALSE;
        RemoveFromWorld();
    }

    void CFlotsam::AddToWorld()
    {
        if (!m_bAddedToWorld && m_pClump)
        {
            m_bAddedToWorld = TRUE;
            ++ms_nFlotsamsAddedToWorlds;
            RpWorld* pWorld = CScene::GetWorld();
            if (pWorld)
            {
                RpWorldAddClump(pWorld, m_pClump);
            }
        }
    }

    void CFlotsam::RemoveFromWorld()
    {
        if (m_bAddedToWorld)
        {
            m_bAddedToWorld = FALSE;
            RpWorld* pWorld = CScene::GetWorld();
            if (pWorld && m_pClump)
            {
                RpWorldRemoveClump(pWorld, m_pClump);
            }
        }
    }
}
