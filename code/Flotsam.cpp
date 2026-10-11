// ============================================================================
// CannonCruise - Flotsam Environmental Entity (Flotsam.cpp)
// Original path: D:\Projects\CannonCruisePC\code\Flotsam.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_0047f590 — CFlotsam initialization & event wiring
//   FUN_0047faa0 — CFlotsam clump loading ("Behaviour already has a clump", "_COLLHULL")
//   FUN_0047fe40 — CFlotsam attribute processing
//   FUN_00480250 — CFlotsam class registration ("CFlotsam")
//   FUN_00480cc0 — CFlotsam destructor ("ms_nFlotsamsAddedToWorlds != 0 Failed")
// ============================================================================

#include "Flotsam.h"
#include "framework/core/attributehandler/cclassfactory.h"
#include "framework/core/attributehandler/csystemcommands.h"
#include "framework/toolkits/world/helpers/clumphelper.h"
#include "framework/core/resourcemanager/cresourcemanager.h"
#include "Scene.h"
#include "Ocean.h"

namespace RWS
{
    // Register CFlotsam with RenderWare Studio reflection system
    RWS_REGISTER_CLASS(CFlotsam);

    // Global counter tracking active flotsam objects in the simulation
    RwUInt32 CFlotsam::ms_nFlotsamsAddedToWorlds = 0;

    // ---------------------------------------------------------------------------
    // Constructor – mirrors FUN_0047f590.
    // Extracts clump model from attribute packet, registers tick handler,
    // and inserts entity into the active scene world.
    // ---------------------------------------------------------------------------
    CFlotsam::CFlotsam(const CAttributePacket& packet)
        : CAttributeHandler(packet)
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

    // ---------------------------------------------------------------------------
    // Destructor – mirrors FUN_00480cc0.
    // Verifies counter integrity, removes entity from world, and destroys clump.
    // ---------------------------------------------------------------------------
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

    // ---------------------------------------------------------------------------
    // HandleAttributes – dynamic property updater from workspace / level editor.
    // Command mapping:
    //   0: Buoyancy multiplier (RwReal)
    //   1: Yaw rotation speed (RwReal)
    // ---------------------------------------------------------------------------
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

    // ---------------------------------------------------------------------------
    // HandleEvents – processes per-frame engine ticks.
    // Queries ocean wave height at current X/Z position to align floating mesh Y.
    // ---------------------------------------------------------------------------
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
                    // Align vertical Y position with ocean wave crests
                    RwReal waveHeight = COcean::GetWaterHeight(pMatrix->pos.x, pMatrix->pos.z);
                    pMatrix->pos.y = waveHeight;
                    RwFrameUpdateObjects(pFrame);
                }
            }
        }
    }

    // ---------------------------------------------------------------------------
    // OnCollision – collision trigger handler delegated from CCollisionTrigger.
    // ---------------------------------------------------------------------------
    void CFlotsam::OnCollision(const CollisionTrigger::CollisionInfo& info)
    {
        OnCollect();
    }

    // ---------------------------------------------------------------------------
    // OnCollect – marks pickup as collected and extracts from scene simulation.
    // ---------------------------------------------------------------------------
    void CFlotsam::OnCollect()
    {
        m_bActive = FALSE;
        RemoveFromWorld();
    }

    // ---------------------------------------------------------------------------
    // AddToWorld – inserts clump model into current CScene RpWorld.
    // ---------------------------------------------------------------------------
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

    // ---------------------------------------------------------------------------
    // RemoveFromWorld – extracts clump model from current CScene RpWorld.
    // ---------------------------------------------------------------------------
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
