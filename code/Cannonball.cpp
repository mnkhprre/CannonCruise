// ============================================================================
// CannonCruise - Cannonball Projectile (Cannonball.cpp)
// Original path: D:\Projects\CannonCruisePC\code\Cannonball.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_0046c2a0 — CCannonball constructor & registration
//   FUN_0046d1e0 — CCannonball::Spawn
//   FUN_0046d520 — CCannonball::OnHit
//   FUN_0046d950 — CCannonball destructor & world counter
// ============================================================================

#include "Cannonball.h"
#include "framework/core/attributehandler/cclassfactory.h"
#include "AudioManager.h"
#include <cassert>

// Register CCannonball with RenderWare Studio's reflection class factory
RWS_REGISTER_CLASS(CCannonball, []() -> void* { return new CCannonball(); }, sizeof(CCannonball));

// Global counter tracking all live cannonballs across active scenes/worlds
RwUInt32 CCannonball::sm_nCannonBallsAddedToWorlds = 0;

// ---------------------------------------------------------------------------
// Constructor – mirrors FUN_0046c2a0.
// Initializes state variables and increments the world-wide cannonball counter.
// ---------------------------------------------------------------------------
CCannonball::CCannonball()
    : m_pClump(0),
      m_eType(CANNONBALL_STANDARD),
      m_fLifeTimer(0.0f),
      m_uOwnerID(0)
{
    sm_nCannonBallsAddedToWorlds++;
}

// ---------------------------------------------------------------------------
// Destructor – mirrors FUN_0046d950.
// Verifies counter consistency and destroys the associated RenderWare 3D clump.
// ---------------------------------------------------------------------------
CCannonball::~CCannonball()
{
    assert(sm_nCannonBallsAddedToWorlds != 0 && "ms_nCannonBallsAddedToWorlds != 0 Failed");
    if (sm_nCannonBallsAddedToWorlds > 0)
    {
        sm_nCannonBallsAddedToWorlds--;
    }

    if (m_pClump)
    {
        RpClumpDestroy(m_pClump);
        m_pClump = 0;
    }
}

// ---------------------------------------------------------------------------
// HandleEvents – framework message listener for simulation ticks or triggers.
// ---------------------------------------------------------------------------
void CCannonball::HandleEvents(const RWS::CMsg& msg)
{
    // Framework event dispatch (unused in minimal projectile implementation)
}

// ---------------------------------------------------------------------------
// Spawn – mirrors FUN_0046d1e0.
// Injects the cannonball into the physical simulation at the specified muzzle
// position, imparts launch velocity, activates a 5-second lifetime limit,
// adds it to the Havok/CPhysicsEntity world, and fires the launch sound effect.
// ---------------------------------------------------------------------------
void CCannonball::Spawn(const SCannonballSpawnData& spawnData)
{
    m_eType = spawnData.m_Type;
    m_uOwnerID = spawnData.m_uOwnerID;
    m_fLifeTimer = 5.0f; // Default 5.0 seconds maximum flight time

    SetPosition(&spawnData.m_Pos);
    SetLinearVelocity(&spawnData.m_Vel);
    AddToWorld();

    CAudioManager* pAudio = CAudioManager::GetSingletonPtr();
    if (pAudio)
    {
        pAudio->PlaySound("Audio_CannonBall", &spawnData.m_Pos);
    }
}

// ---------------------------------------------------------------------------
// Update – frame tick logic.
// Decrements the remaining life timer and automatically removes the projectile
// if it travels without hitting an obstacle before timing out.
// ---------------------------------------------------------------------------
void CCannonball::Update(RwReal deltaTime)
{
    if (m_fLifeTimer > 0.0f)
    {
        m_fLifeTimer -= deltaTime;
        if (m_fLifeTimer <= 0.0f)
        {
            RemoveFromWorld();
        }
    }
}

// ---------------------------------------------------------------------------
// OnHit – mirrors FUN_0046d520.
// Handles impact resolution based on collision surface:
//   hitType == 0 : Direct hit on target (enemy/player ship hull or fort)
//   hitType == 1 : Ocean water surface splashdown
//   hitType == * : Solid static world geometry (rock, cliff, building)
// Dispatches audio cue and cleans up projectile from physics simulation.
// ---------------------------------------------------------------------------
void CCannonball::OnHit(void* pTarget, RwUInt32 hitType)
{
    CAudioManager* pAudio = CAudioManager::GetSingletonPtr();
    if (pAudio)
    {
        if (hitType == 0)
        {
            pAudio->PlaySound("Audio_CannonHit");
        }
        else if (hitType == 1)
        {
            pAudio->PlaySound("Audio_WaterHit");
        }
        else
        {
            pAudio->PlaySound("Audio_StaticHit");
        }
    }

    RemoveFromWorld();
}
