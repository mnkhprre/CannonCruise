// ============================================================================
// CannonCruise - Soldier NPC & Soldier AI (Soldier.cpp)
// Original path: D:\Projects\CannonCruisePC\code\Soldier.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_004b7e40 — CSoldier attribute handling & setup
//   FUN_004b8160 — CSoldier destructor & cleanup ("ms_nSoldiersAddedToWorlds != 0 Failed")
//   FUN_004b8710 — CSoldier::HandleEvents ("CSoldier HE iMsgRunningTick")
//   FUN_004b8810 — CSoldier class registration & factory
//   FUN_004b92f0 — CSoldierAi class registration
// ============================================================================

#include "Soldier.h"
#include "framework/core/attributehandler/cclassfactory.h"
#include "framework/core/attributehandler/csystemcommands.h"
#include "framework/toolkits/world/helpers/clumphelper.h"
#include "framework/core/resourcemanager/cresourcemanager.h"
#include "Scene.h"

namespace RWS
{
    // Register soldier entity classes with the RenderWare Studio reflection system
    RWS_REGISTER_CLASS(CSoldier);

    // Global counter tracking active soldier instances in simulation
    RwUInt32 CSoldier::ms_nSoldiersAddedToWorlds = 0;

    // ---------------------------------------------------------------------------
    // Constructor – allocates animation controller and AI state machine, extracts
    // clump model from attribute packet, registers tick event, and enters world.
    // ---------------------------------------------------------------------------
    CSoldier::CSoldier(const CAttributePacket& packet)
        : CAttributeHandler(packet)
        , CEventHandler(0)
        , m_pPhysEntity(0)
        , m_pClump(0)
        , m_pAnimController(0)
        , m_pAi(0)
        , m_health(100.0f)
        , m_speed(2.0f)
        , m_turnRate(1.0f)
        , m_soldierType(TYPE_NORMAL)
        , m_bAddedToWorld(FALSE)
    {
        m_pAnimController = new CAnimController();
        m_pAi = new CSoldierAi(this);

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

    // ---------------------------------------------------------------------------
    // Destructor – mirrors FUN_004b8160.
    // Unregisters tick handler, decrements global soldier count with assert check,
    // deletes AI and anim controllers, removes clump from world, and frees assets.
    // ---------------------------------------------------------------------------
    CSoldier::~CSoldier()
    {
        UnregisterForMessage(iMsgRunningTick);

        if (m_bAddedToWorld)
        {
            RWS_ASSERT(ms_nSoldiersAddedToWorlds != 0, "ms_nSoldiersAddedToWorlds != 0 Failed");
            --ms_nSoldiersAddedToWorlds;
        }

        if (m_pAi)
        {
            delete m_pAi;
            m_pAi = 0;
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

    // ---------------------------------------------------------------------------
    // HandleAttributes – dynamic property updater from workspace / level editor.
    // Command mapping:
    //   0: Turning rate in rad/s (RwReal)
    //   1: Health capacity (RwReal)
    //   2: Movement speed (RwReal)
    //   4: SoldierType role archetype (RwInt32)
    // ---------------------------------------------------------------------------
    void CSoldier::HandleAttributes(const CAttributePacket& packet)
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

        CAttributeCommandIterator it(packet, RWS_CLASS_INDEX(CSoldier));
        while (!it.IsFinished())
        {
            switch (it->GetCommandId())
            {
            case 0:
                m_turnRate = *reinterpret_cast<const RwReal*>(it->GetCommandData());
                break;
            case 1:
                m_health = *reinterpret_cast<const RwReal*>(it->GetCommandData());
                break;
            case 2:
                m_speed = *reinterpret_cast<const RwReal*>(it->GetCommandData());
                break;
            case 4:
                m_soldierType = static_cast<SoldierType>(*reinterpret_cast<const RwInt32*>(it->GetCommandData()));
                break;
            default:
                break;
            }
            ++it;
        }
    }

    // ---------------------------------------------------------------------------
    // HandleEvents – mirrors "CSoldier HE iMsgRunningTick" (FUN_004b8710).
    // Receives engine ticks and advances simulation.
    // ---------------------------------------------------------------------------
    void CSoldier::HandleEvents(CMsg& msg)
    {
        if (msg.Id == iMsgRunningTick)
        {
            Update(0.0166667f);
        }
    }

    // ---------------------------------------------------------------------------
    // AddToWorld – attaches character clump to the active CScene RpWorld.
    // ---------------------------------------------------------------------------
    void CSoldier::AddToWorld()
    {
        if (!m_bAddedToWorld && m_pClump)
        {
            m_bAddedToWorld = TRUE;
            ++ms_nSoldiersAddedToWorlds;
            RpWorld* pWorld = CScene::GetWorld();
            if (pWorld)
            {
                RpWorldAddClump(pWorld, m_pClump);
            }
        }
    }

    // ---------------------------------------------------------------------------
    // RemoveFromWorld – detaches character clump from active CScene RpWorld.
    // ---------------------------------------------------------------------------
    void CSoldier::RemoveFromWorld()
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

    // ---------------------------------------------------------------------------
    // TakeDamage – applies hit-point deduction, plays guard impact sound
    // ("Audio_HitGuard"), and switches AI state to STATE_DEAD upon zero health.
    // ---------------------------------------------------------------------------
    void CSoldier::TakeDamage(RwReal damage)
    {
        m_health -= damage;
        CAudioManager* pAudio = CAudioManager::GetSingletonPtr();
        if (pAudio)
        {
            pAudio->PlaySound("Audio_HitGuard");
        }
        if (m_health <= 0.0f && m_pAi)
        {
            m_pAi->SetState(CSoldierAi::STATE_DEAD);
        }
    }

    // ---------------------------------------------------------------------------
    // Update – ticks skeletal animation controller and evaluates AI behaviors.
    // ---------------------------------------------------------------------------
    void CSoldier::Update(RwReal deltaTime)
    {
        if (m_pAnimController)
        {
            m_pAnimController->Update(deltaTime);
        }
        if (m_pAi)
        {
            m_pAi->Update(deltaTime);
        }
    }

    // ===========================================================================
    // CSoldierAi Implementation
    // ===========================================================================

    CSoldierAi::CSoldierAi(CSoldier* pSoldier)
        : m_pSoldier(pSoldier)
        , m_state(STATE_IDLE)
        , m_stateTimer(0.0f)
    {
        m_targetPos.x = 0.0f;
        m_targetPos.y = 0.0f;
        m_targetPos.z = 0.0f;
    }

    CSoldierAi::~CSoldierAi()
    {
    }

    void CSoldierAi::SetState(State state)
    {
        m_state = state;
        m_stateTimer = 0.0f;
    }

    void CSoldierAi::Update(RwReal deltaTime)
    {
        m_stateTimer += deltaTime;
        switch (m_state)
        {
        case STATE_IDLE:
            if (m_stateTimer > 3.0f)
            {
                SetState(STATE_PATROL);
            }
            break;
        case STATE_PATROL:
            // Patrol logic
            break;
        case STATE_CHASE:
            // Chase logic
            break;
        case STATE_ATTACK:
            // Attack logic
            break;
        case STATE_DEAD:
            break;
        }
    }
}
