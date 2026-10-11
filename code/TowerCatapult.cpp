// ============================================================================
// CannonCruise - Tower Catapult Defense Entity (TowerCatapult.cpp)
// Original path: D:\Projects\CannonCruisePC\code\TowerCatapult.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_004bc3f0 — CTowerCatapult constructor & class registration ("CTowerCatapult")
//   FUN_004bcf30 — CTowerCatapult::Initialize ("Standard graphics (undamaged) not available...")
//   FUN_004be0b0 — CTowerCatapult::TakeDamage
//   FUN_004bf120 — CTowerCatapult destructor & world counter ("ms_nTowersAddedToWorlds != 0 Failed")
// ============================================================================

#include "TowerCatapult.h"
#include "AudioManager.h"
#include "framework/core/attributehandler/cclassfactory.h"
#include <cassert>

// Register CTowerCatapult with RenderWare Studio reflection system
RWS_REGISTER_CLASS(
    CTowerCatapult, []() -> void * { return new CTowerCatapult(); },
    sizeof(CTowerCatapult));

// Global counter tracking live defensive towers across scenes
RwUInt32 CTowerCatapult::sm_nTowersAddedToWorlds = 0;

// ---------------------------------------------------------------------------
// Constructor – mirrors FUN_004bc3f0.
// Initializes durability to 150 HP and increments global tower counter.
// ---------------------------------------------------------------------------
CTowerCatapult::CTowerCatapult()
    : m_pUndamagedClump(0), m_pDamagedClump(0), m_fHealth(150.0f),
      m_bDestroyed(FALSE) {
  sm_nTowersAddedToWorlds++;
}

// ---------------------------------------------------------------------------
// Destructor – mirrors FUN_004bf120.
// Decrements global counter with assert check and destroys owned clumps.
// ---------------------------------------------------------------------------
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

// ---------------------------------------------------------------------------
// HandleEvents – framework message listener for simulation events.
// ---------------------------------------------------------------------------
void CTowerCatapult::HandleEvents(const RWS::CMsg &msg) {}

// ---------------------------------------------------------------------------
// Initialize – mirrors FUN_004bcf30.
// Binds intact and optional damaged clump models, validating undamaged clump.
// ---------------------------------------------------------------------------
void CTowerCatapult::Initialize(RpClump *pUndamagedClump,
                                RpClump *pDamagedClump) {
  assert(pUndamagedClump != 0 &&
         "Standard graphics (undamaged) not available for TowerCatapult !");
  assert(m_pUndamagedClump == 0 &&
         "Undamaged clump already loaded for TowerCatapult");

  m_pUndamagedClump = pUndamagedClump;
  m_pDamagedClump = pDamagedClump;
}

// ---------------------------------------------------------------------------
// TakeDamage – mirrors FUN_004be0b0.
// Subtracts damage from health pool and triggers DestroyTower() at or below 0.
// ---------------------------------------------------------------------------
void CTowerCatapult::TakeDamage(RwReal damage) {
  if (m_bDestroyed)
    return;

  m_fHealth -= damage;
  if (m_fHealth <= 0.0f) {
    DestroyTower();
  }
}

// ---------------------------------------------------------------------------
// DestroyTower – switches state to destroyed, plays "Audio_TowerDestroy".
// ---------------------------------------------------------------------------
void CTowerCatapult::DestroyTower() {
  m_bDestroyed = TRUE;

  CAudioManager *pAudio = CAudioManager::GetSingletonPtr();
  if (pAudio) {
    pAudio->PlaySound("Audio_TowerDestroy");
  }

  // Switch to damaged clump representation
}
