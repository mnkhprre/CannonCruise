#include "Bird.h"
#include "framework/core/attributehandler/cclassfactory.h"
#include "framework/core/attributehandler/csystemcommands.h"
#include "framework/toolkits/world/helpers/clumphelper.h"
#include "framework/core/resourcemanager/cresourcemanager.h"
#include "Scene.h"
#include <cmath>

namespace RWS
{
    RWS_REGISTER_CLASS(CBird);
    RWS_REGISTER_CLASS(CBirdSeaGull);

    RwUInt32 CBird::ms_nBirdsAddedToWorlds = 0;

    CBird::CBird(const CAttributePacket& packet)
        : CAttributeHandler(packet)
        , CEventHandler(0)
        , m_pPhysEntity(0)
        , m_pClump(0)
        , m_pAnimController(0)
        , m_flightSpeed(5.0f)
        , m_flightHeight(15.0f)
        , m_turnRate(1.0f)
        , m_circleRadius(20.0f)
        , m_angle(0.0f)
        , m_flyingState(0)
        , m_bAddedToWorld(FALSE)
    {
        m_originPos.x = 0.0f;
        m_originPos.y = 0.0f;
        m_originPos.z = 0.0f;

        m_pAnimController = new CAnimController();

        const void* pClumpData = CSystemCommands::ExtractClump(packet);
        if (pClumpData)
        {
            m_pClump = ClumpHelper::CreateClumpFromResource(pClumpData);
            if (m_pClump && m_pAnimController)
            {
                m_pAnimController->SetTarget(m_pClump);
            }
        }

        RegisterForMessage(iMsgRunningTick);
        AddToWorld();
    }

    CBird::~CBird()
    {
        UnregisterForMessage(iMsgRunningTick);

        if (m_bAddedToWorld)
        {
            RWS_ASSERT(ms_nBirdsAddedToWorlds != 0, "ms_nBirdsAddedToWorlds != 0 Failed");
            --ms_nBirdsAddedToWorlds;
        }

        if (m_pAnimController)
        {
            delete m_pAnimController;
            m_pAnimController = 0;
        }

        RemoveFromWorld();

        if (m_pClump)
        {
            ClumpHelper::DestroyClump(m_pClump);
            m_pClump = 0;
        }
    }

    void CBird::HandleAttributes(const CAttributePacket& packet)
    {
        CAttributeHandler::HandleAttributes(packet);

        if (m_pClump)
        {
            ClumpHelper::HandleAttributes(m_pClump, packet);
            if (m_pAnimController)
            {
                m_pAnimController->HandleAttributes(packet);
            }
        }

        CAttributeCommandIterator it(packet, RWS_CLASS_INDEX(CBird));
        while (!it.IsFinished())
        {
            switch (it->GetCommandId())
            {
            case 0:
                m_flightHeight = *reinterpret_cast<const RwReal*>(it->GetCommandData());
                break;
            case 1:
                m_flightSpeed = *reinterpret_cast<const RwReal*>(it->GetCommandData());
                break;
            case 2:
                m_circleRadius = *reinterpret_cast<const RwReal*>(it->GetCommandData());
                break;
            case 3:
                m_turnRate = *reinterpret_cast<const RwReal*>(it->GetCommandData());
                break;
            default:
                break;
            }
            ++it;
        }
    }

    void CBird::HandleEvents(CMsg& msg)
    {
        if (msg.Id == iMsgRunningTick)
        {
            Update(0.0166667f);
        }
    }

    void CBird::AddToWorld()
    {
        if (!m_bAddedToWorld && m_pClump)
        {
            m_bAddedToWorld = TRUE;
            ++ms_nBirdsAddedToWorlds;
            RpWorld* pWorld = CScene::GetWorld();
            if (pWorld)
            {
                RpWorldAddClump(pWorld, m_pClump);
            }
        }
    }

    void CBird::RemoveFromWorld()
    {
        if (m_bAddedToWorld)
        {
            m_bAddedToWorld = FALSE;
            RWS_ASSERT(m_pClump != 0, "m_pClump != NULL Failed");
            RpWorld* pWorld = CScene::GetWorld();
            if (pWorld && m_pClump)
            {
                RpWorldRemoveClump(pWorld, m_pClump);
            }
        }
    }

    void CBird::Update(RwReal deltaTime)
    {
        UpdateFlight(deltaTime);

        if (m_pAnimController)
        {
            m_pAnimController->Update(deltaTime);
        }
    }

    void CBird::UpdateFlight(RwReal deltaTime)
    {
        m_angle += m_turnRate * deltaTime;
        if (m_angle > 6.2831853f)
        {
            m_angle -= 6.2831853f;
        }

        if (m_pClump)
        {
            RwFrame* pFrame = RpClumpGetFrame(m_pClump);
            if (pFrame)
            {
                RwV3d pos;
                pos.x = m_originPos.x + std::cos(m_angle) * m_circleRadius;
                pos.y = m_originPos.y + m_flightHeight;
                pos.z = m_originPos.z + std::sin(m_angle) * m_circleRadius;

                RwMatrix* pMatrix = RwFrameGetMatrix(pFrame);
                pMatrix->pos = pos;
                RwFrameUpdateObjects(pFrame);
            }
        }
    }

    // CBirdSeaGull
    CBirdSeaGull::CBirdSeaGull(const CAttributePacket& packet)
        : CBird(packet)
    {
    }

    CBirdSeaGull::~CBirdSeaGull()
    {
    }

    void CBirdSeaGull::HandleAttributes(const CAttributePacket& packet)
    {
        CBird::HandleAttributes(packet);
    }

    void CBirdSeaGull::HandleEvents(CMsg& msg)
    {
        CBird::HandleEvents(msg);
    }
}
