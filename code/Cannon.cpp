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

CCannon::~CCannon()
{
}

void CCannon::HandleEvents(const RWS::CMsg& msg)
{
}

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

void CCannon::Fire(ECannonballType type, const RwV3d* pDirection, RwReal speed)
{
    if (!pDirection) return;

    RwV3d firePos;
    if (m_pGunBarrelFrame)
    {
        RWS::FrameHelper::GetPosition(m_pGunBarrelFrame, &firePos);
    }
    else if (m_pClump)
    {
        RWS::FrameHelper::GetPosition(RpClumpGetFrame(m_pClump), &firePos);
    }

    RwV3d vel;
    vel.x = pDirection->x * speed;
    vel.y = pDirection->y * speed;
    vel.z = pDirection->z * speed;

    SCannonballSpawnData spawnData;
    spawnData.m_Pos = firePos;
    spawnData.m_Vel = vel;
    spawnData.m_Type = type;
    spawnData.m_uOwnerID = m_uOwnerID;

    // Top ateşleme sesi
    CAudioManager* pAudio = CAudioManager::GetSingletonPtr();
    if (pAudio)
    {
        pAudio->PlaySound("Audio_CannonFire", &firePos);
    }
}

void CCannon::Aim(RwReal elevation, RwReal azimuth)
{
    // Namlu ve kaide dönüşleri
}
