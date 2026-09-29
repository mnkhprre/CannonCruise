// ============================================================================
// CannonCruise - Collision Trigger Entity (CollisionTrigger.cpp)
// Original path: D:\Projects\CannonCruisePC\code\CollisionTrigger.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_0046f420 — CCollisionTrigger constructor & registration
//   FUN_0046f5e0 — CCollisionTrigger::Initialize
//   FUN_004702c0 — CCollisionTrigger::OnEntityEnter
// ============================================================================

#include "CollisionTrigger.h"
#include "framework/core/attributehandler/cclassfactory.h"
#include <cassert>

RWS_REGISTER_CLASS(
    CCollisionTrigger, []() -> void * { return new CCollisionTrigger(); },
    sizeof(CCollisionTrigger));

CCollisionTrigger::CCollisionTrigger()
    : m_pClump(0), m_pPhantom(0), m_iEnterSpeakCategory(0),
      m_iEnterSpeakIndex(0), m_iLeaveSpeakCategory(0), m_iLeaveSpeakIndex(0) {}

CCollisionTrigger::~CCollisionTrigger() {}

void CCollisionTrigger::HandleEvents(const RWS::CMsg &msg) {}

void CCollisionTrigger::Initialize(RpClump *pClump) {
  assert(pClump != 0 &&
         "Could not create clump for collision trigger ! A clump (asset) must "
         "be present at start for every collision trigger");
  m_pClump = pClump;

  // Havok hkpPhantom oluşturma
}

void CCollisionTrigger::OnEntityEnter(void *pEntity) {
  // Giriş diyalog/olay tetikleme
}

void CCollisionTrigger::OnEntityLeave(void *pEntity) {
  // Çıkış diyalog/olay tetikleme
}
