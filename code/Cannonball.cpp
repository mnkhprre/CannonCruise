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

RWS_REGISTER_CLASS(CCannonball, []() -> void* { return new CCannonball(); }, sizeof(CCannonball));

RwUInt32 CCannonball::sm_nCannonBallsAddedToWorlds = 0;

CCannonball::CCannonball()
    : m_pClump(0),
      m_eType(CANNONBALL_STANDARD),
      m_fLifeTimer(0.0f),
      m_uOwnerID(0)
{
    sm_nCannonBallsAddedToWorlds++;
}

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

void CCannonball::HandleEvents(const RWS::CMsg& msg)
{
}

void CCannonball::Spawn(const SCannonballSpawnData& spawnData)
{
    m_eType = spawnData.m_Type;
    m_uOwnerID = spawnData.m_uOwnerID;
    m_fLifeTimer = 5.0f;

    SetPosition(&spawnData.m_Pos);
    SetLinearVelocity(&spawnData.m_Vel);
    AddToWorld();

    CAudioManager* pAudio = CAudioManager::GetSingletonPtr();
    if (pAudio)
    {
        pAudio->PlaySound("Audio_CannonBall", &spawnData.m_Pos);
    }
}

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
