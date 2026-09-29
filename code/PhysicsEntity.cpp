// ============================================================================
// CannonCruise - Physics Entity Base (PhysicsEntity.cpp)
// Original path: D:\Projects\CannonCruisePC\code\PhysicsEntity.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_00436480 — CPhysicsEntity constructor
//   FUN_00437410 — CPhysicsEntity::AddToWorld
//   FUN_00437c60 — CPhysicsEntity::RemoveFromWorld
// ============================================================================

#include "PhysicsEntity.h"
#include "PhysicsWorld.h"
#include "framework/core/attributehandler/cclassfactory.h"
#include <cassert>

CPhysicsEntity::CPhysicsEntity() : m_pRigidBody(0), m_bInWorld(FALSE) {}

CPhysicsEntity::~CPhysicsEntity() { RemoveFromWorld(); }

void CPhysicsEntity::HandleEvents(const RWS::CMsg &msg) {}

void CPhysicsEntity::SetRigidBody(void *pBody) { m_pRigidBody = pBody; }

void CPhysicsEntity::AddToWorld() {
  assert(m_pRigidBody != 0 && "m_pRigidBody != NULL Failed");

  CPhysicsWorld *pWorld = CPhysicsWorld::GetSingletonPtr();
  if (pWorld && !m_bInWorld) {
    assert(!(pWorld->RigidBodyExistsInWorld(m_pRigidBody)) &&
           "!(CPhysicsWorld::GetSingleton()->RigidBodyExistsInWorld(m_"
           "pRigidBody)) Failed");
    pWorld->AddRigidBody(m_pRigidBody);
    m_bInWorld = TRUE;
  }
}

void CPhysicsEntity::RemoveFromWorld() {
  if (m_bInWorld && m_pRigidBody) {
    CPhysicsWorld *pWorld = CPhysicsWorld::GetSingletonPtr();
    if (pWorld) {
      pWorld->RemoveRigidBody(m_pRigidBody);
    }
    m_bInWorld = FALSE;
  }
}

void CPhysicsEntity::SetPosition(const RwV3d *pPos) { assert(pPos != 0); }

void CPhysicsEntity::GetPosition(RwV3d *pPos) const { assert(pPos != 0); }

void CPhysicsEntity::SetLinearVelocity(const RwV3d *pVel) { assert(pVel != 0); }

void CPhysicsEntity::GetLinearVelocity(RwV3d *pVel) const { assert(pVel != 0); }
