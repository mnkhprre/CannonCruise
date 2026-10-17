// ============================================================================
// CannonCruise - Collision Trigger Entity (CollisionTrigger.cpp)
// Original path: D:\Projects\CannonCruisePC\code\CollisionTrigger.cpp
// Reverse-engineered via Ghidra and CC.asm analysis.
//
// Binary functions:
//   FUN_0046f420 — CCollisionTrigger constructor & registration
//   FUN_0046f5e0 — CCollisionTrigger::Initialize (clump → hkpPhantom)
//   FUN_004702c0 — CCollisionTrigger::OnEntityEnter
//
// Backend: hkpPhantom -> btGhostObject (PhysicsBackend). Initialize,
// creates AABB box phantom from clump bounding sphere and registers
// with CPhysicsWorld singleton (CPhysicsWorld::AddPhantom ->
// backend PhysicsWorld_AddTrigger). HandleEvents polls overlap list
// every frame; delta flows to OnEntityEnter/OnEntityLeave.
//
// DEVIATION: binary OnEntityEnter/Leave triggers CSpeakManager speak event
// (assert "IsSpeakValid(m_iEnterSpeakCategory...") — speak manager
// not ported, fields preserved. Clump sphere NULL in RW stubs -> real
// data flows in librw.
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
    const RwInt32 kMaxOverlaps = 16; // max bodies reported per turn
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

  // FUN_0046f5e0 — get bounding sphere from clump (stub: NULL -> early return;
  // in librw, AABB phantom is created when filled)
  const RwSphere *pSphere = RpClumpGetBoundingSphere(pClump);
  if (!pSphere) {
    return; // no data (RW stub condition)
  }
  InitializeWithSphere(&pSphere->center, pSphere->radius);
}

void CCollisionTrigger::InitializeWithSphere(const RwV3d *pCenter,
                                             RwReal radius) {
  assert(pCenter != 0);
  assert(radius > 0.0f && "trigger radius > 0 Failed");
  assert(m_pPhantom == 0 && "phantom already created");

  // sphere -> AABB box (half-extent = radius) — hkpBoxPhantom emulation
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

  // leavers — were present last turn, gone this turn
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

  // enterers — present this turn, absent last turn
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
  // FUN_004702c0 — binary triggers CSpeakManager speak event here
  // (m_iEnterSpeakCategory/m_iEnterSpeakIndex); speak manager not ported.
  (void)pEntity;
}

void CCollisionTrigger::OnEntityLeave(void *pEntity) {
  (void)pEntity;
}
