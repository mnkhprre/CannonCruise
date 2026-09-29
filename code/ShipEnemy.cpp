// ============================================================================
// CannonCruise - Enemy AI Ship (ShipEnemy.cpp)
// Original path: D:\Projects\CannonCruisePC\code\ShipEnemy.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_004a8cd0 — CShipEnemy constructor & registration
//   FUN_004aa2e0 — CShipEnemy::UpdateAI
//   FUN_004ad050 — CShipEnemy destructor
// ============================================================================

#include "ShipEnemy.h"
#include "framework/core/attributehandler/cclassfactory.h"
#include "SpeakManager.h"
#include "AudioManager.h"
#include <cassert>

RWS_REGISTER_CLASS(CShipEnemy, []() -> void* { return new CShipEnemy(); }, sizeof(CShipEnemy));

RwUInt32 CShipEnemy::sm_nShipsAddedToWorlds = 0;

CShipEnemy::CShipEnemy()
    : m_eState(STATE_PATROL),
      m_fHuntTimer(0.0f),
      m_iCurrentWaypoint(0),
      m_iAttackSpeakCategory(1),
      m_iAttackSpeakIndex(0),
      m_iDeathSpeakCategory(2),
      m_iDeathSpeakIndex(0)
{
    sm_nShipsAddedToWorlds++;
}

CShipEnemy::~CShipEnemy()
{
    assert(sm_nShipsAddedToWorlds != 0 && "ms_nShipsAddedToWorlds != 0 Failed");
    if (sm_nShipsAddedToWorlds > 0)
    {
        sm_nShipsAddedToWorlds--;
    }
}

void CShipEnemy::HandleEvents(const RWS::CMsg& msg)
{
    // CShipEnemy HE iMsgRunningTick
}

void CShipEnemy::SetAIState(EEnemyAIState state)
{
    m_eState = state;
    if (m_eState == STATE_ATTACK)
    {
        CSpeakManager* pSpeakMgr = CSpeakManager::GetSingletonPtr();
        if (pSpeakMgr && pSpeakMgr->IsSpeakValid(m_iAttackSpeakCategory, m_iAttackSpeakIndex))
        {
            pSpeakMgr->PlaySpeak(m_iAttackSpeakCategory, m_iAttackSpeakIndex);
        }
    }
}

void CShipEnemy::UpdateAI(RwReal deltaTime)
{
    switch (m_eState)
    {
        case STATE_PATROL:
            // Devriye rotası takip et
            break;

        case STATE_GUARD:
            // Bölgeyi savun
            break;

        case STATE_HUNT:
            m_fHuntTimer += deltaTime;
            break;

        case STATE_ATTACK:
            // Oyuncuya yaklaş ve ateş et
            break;
    }
}
