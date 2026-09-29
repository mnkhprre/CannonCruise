// ============================================================================
// CannonCruise - Tower Catapult Defense Entity (TowerCatapult.cpp)
// Original path: D:\Projects\CannonCruisePC\code\TowerCatapult.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_004bc3f0 — CTowerCatapult constructor & registration
//   FUN_004bcf30 — CTowerCatapult::Initialize
//   FUN_004be0b0 — CTowerCatapult::TakeDamage
//   FUN_004bf120 — CTowerCatapult destructor & world counter
// ============================================================================

#include "TowerCatapult.h"
#include "AudioManager.h"
#include "framework/core/attributehandler/cclassfactory.h"
#include <cassert>

RWS_REGISTER_CLASS(
    CTowerCatapult, []() -> void * { return new CTowerCatapult(); },
    sizeof(CTowerCatapult));

RwUInt32 CTowerCatapult::sm_nTowersAddedToWorlds = 0;

CTowerCatapult::CTowerCatapult()
    : m_pUndamagedClump(0), m_pDamagedClump(0), m_fHealth(150.0f),
      m_bDestroyed(FALSE) {
  sm_nTowersAddedToWorlds++;
}

CTowerCatapult::~CTowerCatapult() {
  assert(sm_nTowersAddedToWorlds != 0 && "ms_nTowersAddedToWorlds != 0 Failed");
  if (sm_nTowersAddedToWorlds > 0) {
    sm_nTowersAddedToWorlds--;
  }

  if (m_pUndamagedClump) {
    RpClumpDestroy(m_pUndamagedClump);
    m_pUndamagedClump = 0;
  }
  if (m_pDamagedClump) {
    RpClumpDestroy(m_pDamagedClump);
    m_pDamagedClump = 0;
  }
}

void CTowerCatapult::HandleEvents(const RWS::CMsg &msg) {}

void CTowerCatapult::Initialize(RpClump *pUndamagedClump,
                                RpClump *pDamagedClump) {
  assert(pUndamagedClump != 0 &&
         "Standard graphics (undamaged) not available for TowerCatapult !");
  assert(m_pUndamagedClump == 0 &&
         "Undamaged clump already loaded for TowerCatapult");

  m_pUndamagedClump = pUndamagedClump;
  m_pDamagedClump = pDamagedClump;
}

void CTowerCatapult::TakeDamage(RwReal damage) {
  if (m_bDestroyed)
    return;

  m_fHealth -= damage;
  if (m_fHealth <= 0.0f) {
    DestroyTower();
  }
}

void CTowerCatapult::DestroyTower() {
  m_bDestroyed = TRUE;

  CAudioManager *pAudio = CAudioManager::GetSingletonPtr();
  if (pAudio) {
    pAudio->PlaySound("Audio_TowerDestroy");
  }

  // Hasarlı clump görünümüne geçiş
}
