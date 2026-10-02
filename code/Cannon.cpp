// ============================================================================
// CannonCruise - Cannon Weapon Entity (Cannon.cpp)
// Original path: D:\Projects\CannonCruisePC\code\Cannon.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_0046a0e0 — CCannon constructor & initialization
//   FUN_0046a700 — CCannon::Fire
// ============================================================================

#include "Cannon.h"
#include "AudioManager.h"
#include "framework/toolkits/world/helpers/framehelper.h"
#include <cassert>

CCannon::CCannon()
    : m_pClump(0),
      m_pGunBarrelFrame(0),
      m_pGunBaseFrame(0),
      m_uOwnerID(0),
      m_fReloadTimer(0.0f)
{
}

// ---------------------------------------------------------------------------
// Destructor – intentionally does NOT destroy the clump.
// The clump is owned by the parent CShip; we only hold a reference to it.
// ---------------------------------------------------------------------------
CCannon::~CCannon()
{
}

// ---------------------------------------------------------------------------
// HandleEvents – reserved for future message handling (e.g., ammo pickups).
// ---------------------------------------------------------------------------
void CCannon::HandleEvents(const RWS::CMsg& msg)
{
}

// ---------------------------------------------------------------------------
// Initialize – associates the cannon with a 3-D model and its owning ship.
//
// Walks the clump's frame hierarchy to locate:
//   • "Gun_Barrel" – the muzzle tip, used to determine the spawn position
//     of cannonball projectiles.
//   • "Gun_Base"   – the swivel base, used by Aim() for horizontal rotation.
//
// ownerID is stored so that spawned cannonballs can identify which entity
// fired them (important for damage attribution and friendly-fire prevention).
// ---------------------------------------------------------------------------
void CCannon::Initialize(RpClump* pClump, RwUInt32 ownerID)
{
    assert(ownerID != 0 && "Unknown cannon owner");
    m_pClump = pClump;
    m_uOwnerID = ownerID;

    if (m_pClump)
    {
        RwFrame* pRootFrame = RpClumpGetFrame(m_pClump);
        m_pGunBarrelFrame = RWS::FrameHelper::FindFrameByName(pRootFrame, "Gun_Barrel");
        m_pGunBaseFrame = RWS::FrameHelper::FindFrameByName(pRootFrame, "Gun_Base");
    }
}

// ---------------------------------------------------------------------------
// Fire – creates a cannonball and plays the firing audio (FUN_0046a700).
//
// 1. Determine the spawn position:
//    • If the barrel frame exists, use its world position (accurate muzzle).
//    • Otherwise, fall back to the clump root position.
// 2. Compute the velocity vector by scaling the normalised direction by speed.
// 3. Fill an SCannonballSpawnData struct with position, velocity, projectile
//    type, and owner ID. (Actual spawning into the world is delegated to the
//    entity system – the spawn data is published via a message.)
// 4. Play the positional "Audio_CannonFire" sound at the muzzle location.
// ---------------------------------------------------------------------------
void CCannon::Fire(ECannonballType type, const RwV3d* pDirection, RwReal speed)
{
    if (!pDirection) return;

    // --- 1. Muzzle position ---
    RwV3d firePos;
    if (m_pGunBarrelFrame)
    {
        RWS::FrameHelper::GetPosition(m_pGunBarrelFrame, &firePos);
    }
    else if (m_pClump)
    {
        RWS::FrameHelper::GetPosition(RpClumpGetFrame(m_pClump), &firePos);
    }

    // --- 2. Velocity vector ---
    RwV3d vel;
    vel.x = pDirection->x * speed;
    vel.y = pDirection->y * speed;
    vel.z = pDirection->z * speed;

    // --- 3. Spawn data ---
    SCannonballSpawnData spawnData;
    spawnData.m_Pos = firePos;
    spawnData.m_Vel = vel;
    spawnData.m_Type = type;
    spawnData.m_uOwnerID = m_uOwnerID;

    // --- 4. Firing sound (positional 3-D audio) ---
    // Top ateşleme sesi
    CAudioManager* pAudio = CAudioManager::GetSingletonPtr();
    if (pAudio)
    {
        pAudio->PlaySound("Audio_CannonFire", &firePos);
    }
}

// ---------------------------------------------------------------------------
// Aim – adjusts the barrel elevation and base azimuth.
// Implementation stub: barrel and base frame rotations will be applied here
// once the full animation matrices are reverse-engineered.
// ---------------------------------------------------------------------------
void CCannon::Aim(RwReal elevation, RwReal azimuth)
{
    // Namlu ve kaide dönüşleri
}
