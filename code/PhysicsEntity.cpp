// ============================================================================
// CannonCruise - Physics Entity Base (PhysicsEntity.cpp)
// Original path: D:\Projects\CannonCruisePC\code\PhysicsEntity.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_00436480 — CPhysicsEntity constructor & event wiring
//   FUN_00437410 — CPhysicsEntity::AddToWorld
//   FUN_00437c60 — CPhysicsEntity::RemoveFromWorld
// ============================================================================

#include "PhysicsEntity.h"
#include "PhysicsWorld.h"
#include "PhysicsBackend.h"
#include "framework/core/attributehandler/cclassfactory.h"
#include <cassert>

// ---------------------------------------------------------------------------
// Constructor – mirrors FUN_00436480.
// Initializes member variables to NULL/inactive state.
// ---------------------------------------------------------------------------
CPhysicsEntity::CPhysicsEntity() : m_pRigidBody(0), m_bInWorld(FALSE) {}

// ---------------------------------------------------------------------------
// Destructor – cleans up world registration before destruction.
// ---------------------------------------------------------------------------
CPhysicsEntity::~CPhysicsEntity() { RemoveFromWorld(); }

// ---------------------------------------------------------------------------
// HandleEvents – framework message listener for simulation events.
// ---------------------------------------------------------------------------
void CPhysicsEntity::HandleEvents(const RWS::CMsg &msg) {}

// ---------------------------------------------------------------------------
// SetRigidBody – sets the internal Havok rigid body pointer.
// ---------------------------------------------------------------------------
void CPhysicsEntity::SetRigidBody(void *pBody) { m_pRigidBody = pBody; }

// ---------------------------------------------------------------------------
// AddToWorld – mirrors FUN_00437410.
// Inserts the owned Havok rigid body into CPhysicsWorld if not already present.
// Contains binary assertions:
//   - "m_pRigidBody != NULL Failed"
//   - "!(CPhysicsWorld::GetSingleton()->RigidBodyExistsInWorld(m_pRigidBody)) Failed"
// ---------------------------------------------------------------------------
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

// ---------------------------------------------------------------------------
// RemoveFromWorld – mirrors FUN_00437c60.
// Extracts the rigid body from CPhysicsWorld simulation and resets in-world flag.
// ---------------------------------------------------------------------------
void CPhysicsEntity::RemoveFromWorld() {
  if (m_bInWorld && m_pRigidBody) {
    CPhysicsWorld *pWorld = CPhysicsWorld::GetSingletonPtr();
    if (pWorld) {
      pWorld->RemoveRigidBody(m_pRigidBody);
    }
    m_bInWorld = FALSE;
  }
}

// ---------------------------------------------------------------------------
// SetPosition – assigns world-space position to the underlying rigid body.
// Backend: PhysicsBody_SetPosition (motion state dahil).
// ---------------------------------------------------------------------------
void CPhysicsEntity::SetPosition(const RwV3d *pPos) {
  assert(pPos != 0);
  if (m_pRigidBody) {
    PhysicsBody_SetPosition(m_pRigidBody, pPos->x, pPos->y, pPos->z);
  }
}

// ---------------------------------------------------------------------------
// GetPosition – retrieves world-space position from the underlying rigid body.
// ---------------------------------------------------------------------------
void CPhysicsEntity::GetPosition(RwV3d *pPos) const {
  assert(pPos != 0);
  if (m_pRigidBody) {
    PhysicsBody_GetPosition(m_pRigidBody, &pPos->x, &pPos->y, &pPos->z);
  }
}

// ---------------------------------------------------------------------------
// SetLinearVelocity – assigns linear velocity vector to the rigid body.
// ---------------------------------------------------------------------------
void CPhysicsEntity::SetLinearVelocity(const RwV3d *pVel) {
  assert(pVel != 0);
  if (m_pRigidBody) {
    PhysicsBody_SetLinearVelocity(m_pRigidBody, pVel->x, pVel->y, pVel->z);
  }
}

// ---------------------------------------------------------------------------
// GetLinearVelocity – queries linear velocity vector of the rigid body.
// ---------------------------------------------------------------------------
void CPhysicsEntity::GetLinearVelocity(RwV3d *pVel) const {
  assert(pVel != 0);
  if (m_pRigidBody) {
    PhysicsBody_GetLinearVelocity(m_pRigidBody, &pVel->x, &pVel->y, &pVel->z);
  }
}
