// ============================================================================
// CannonCruise - Collision Trigger Entity (CollisionTrigger.cpp)
// Original path: D:\Projects\CannonCruisePC\code\CollisionTrigger.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_0046f420 — CCollisionTrigger constructor & registration
//   FUN_0046f5e0 — CCollisionTrigger::Initialize (clump → hkpPhantom)
//   FUN_004702c0 — CCollisionTrigger::OnEntityEnter
//
// Backend: hkpPhantom → btGhostObject (PhysicsBackend). Initialize,
// clump bounding sphere'inden AABB kutu phantom üretir ve kayıtlı
// CPhysicsWorld singleton'ına ekler (CPhysicsWorld::AddPhantom →
// backend PhysicsWorld_AddTrigger). HandleEvents her turda overlap
// listesini poll eder; fark OnEntityEnter/OnEntityLeave'a akar.
//
// SAPMA: binary OnEntityEnter/Leave içinde CSpeakManager speak olayı
// tetikler (assert "IsSpeakValid(m_iEnterSpeakCategory...") — speak
// manager portlanmadı, alanlar korunuyor. Clump sphere'i RW stub'larında
// NULL → librw'de gerçek veriye akacak.
// ============================================================================

#include "CollisionTrigger.h"
#include "PhysicsBackend.h"
#include "PhysicsWorld.h"
#include "framework/core/attributehandler/cclassfactory.h"
#include <cassert>

RWS_REGISTER_CLASS(
    CCollisionTrigger, []() -> void * { return new CCollisionTrigger(); },
    sizeof(CCollisionTrigger));

namespace
{
    const RwInt32 kMaxOverlaps = 16; // tek turda raporlanan en fazla gövde
}

CCollisionTrigger::CCollisionTrigger()
    : m_pClump(0), m_pPhantom(0), m_iEnterSpeakCategory(0),
      m_iEnterSpeakIndex(0), m_iLeaveSpeakCategory(0), m_iLeaveSpeakIndex(0) {}

CCollisionTrigger::~CCollisionTrigger() {
  if (m_pPhantom) {
    CPhysicsWorld *pWorld = CPhysicsWorld::GetSingletonPtr();
    if (pWorld) {
      pWorld->RemovePhantom(m_pPhantom);
    }
    PhysicsTrigger_Destroy(m_pPhantom);
    m_pPhantom = 0;
  }
}

void CCollisionTrigger::HandleEvents(const RWS::CMsg &msg) {
  (void)msg;
  PollOverlaps();
}

void CCollisionTrigger::Initialize(RpClump *pClump) {
  assert(pClump != 0 &&
         "Could not create clump for collision trigger ! A clump (asset) must "
         "be present at start for every collision trigger");
  m_pClump = pClump;

  // FUN_0046f5e0 — clump'tan bounding sphere al (stub: NULL → erken dönüş;
  // librw'de dolduğunda AABB phantom oluşur)
  const RwSphere *pSphere = RpClumpGetBoundingSphere(pClump);
  if (!pSphere) {
    return; // veri yok (RW stub durumu)
  }
  InitializeWithSphere(&pSphere->center, pSphere->radius);
}

void CCollisionTrigger::InitializeWithSphere(const RwV3d *pCenter,
                                             RwReal radius) {
  assert(pCenter != 0);
  assert(radius > 0.0f && "trigger radius > 0 Failed");
  assert(m_pPhantom == 0 && "phantom already created");

  // sphere → AABB kutu (yarı eksen = radius) — hkpBoxPhantom taklidi
  const RwReal half[3] = {radius, radius, radius};
  const RwReal pos[3] = {pCenter->x, pCenter->y, pCenter->z};
  m_pPhantom = PhysicsTrigger_CreateBox(half, pos);
  assert(m_pPhantom != 0 && "phantom create Failed");

  CPhysicsWorld *pWorld = CPhysicsWorld::GetSingletonPtr();
  if (pWorld) {
    pWorld->AddPhantom(m_pPhantom);
  }
}

void CCollisionTrigger::PollOverlaps() {
  if (!m_pPhantom) {
    return;
  }

  void *hits[kMaxOverlaps] = {0};
  const RwInt32 n =
      PhysicsTrigger_GetOverlaps(m_pPhantom, hits, kMaxOverlaps);

  // çıkanlar — önceki turda vardı, bu turda yok
  for (size_t i = 0; i < m_overlaps.size(); ++i) {
    bool bStill = false;
    for (RwInt32 h = 0; h < n; ++h) {
      if (hits[h] == m_overlaps[i]) {
        bStill = true;
        break;
      }
    }
    if (!bStill) {
      OnEntityLeave(m_overlaps[i]);
    }
  }

  // girenler — bu turda var, önceki turda yok
  for (RwInt32 h = 0; h < n; ++h) {
    bool bWas = false;
    for (size_t i = 0; i < m_overlaps.size(); ++i) {
      if (m_overlaps[i] == hits[h]) {
        bWas = true;
        break;
      }
    }
    if (!bWas) {
      OnEntityEnter(hits[h]);
    }
  }

  m_overlaps.assign(hits, hits + n);
}

void CCollisionTrigger::OnEntityEnter(void *pEntity) {
  // FUN_004702c0 — binary burada CSpeakManager speak olayı tetikler
  // (m_iEnterSpeakCategory/m_iEnterSpeakIndex); speak manager portlanmadı.
  (void)pEntity;
}

void CCollisionTrigger::OnEntityLeave(void *pEntity) {
  (void)pEntity;
}
