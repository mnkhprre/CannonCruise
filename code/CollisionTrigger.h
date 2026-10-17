#ifndef COLLISIONTRIGGER_H
#define COLLISIONTRIGGER_H

// ============================================================================
// CannonCruise - Collision Trigger Entity (CollisionTrigger.h)
// Original path: D:\Projects\CannonCruisePC\code\CollisionTrigger.h
// Reverse-engineered via Ghidra and CC.asm analysis.
// ============================================================================

#include "framework/core/eventhandler/ceventhandler.h"
#include <rpworld.h>
#include <rwcore.h>
#include <vector>

namespace CollisionTrigger
{
    struct CollisionInfo
    {
        void* pEntityA;
        void* pEntityB;
        RwV3d contactPoint;
    };
}

class CCollisionTrigger : public RWS::CEventHandler {
public:
  typedef CollisionTrigger::CollisionInfo CollisionInfo;

  CCollisionTrigger();
  virtual ~CCollisionTrigger();

  virtual void HandleEvents(const RWS::CMsg &msg);

  // binary: FUN_0046f5e0 — creates phantom from clump bounding sphere.
  // RW stubs return NULL for sphere -> real data flows in librw.
  void Initialize(RpClump *pClump);
  // port extension (test + no-clump path): registers AABB box phantom
  // with CPhysicsWorld singleton (derived from sphere radius).
  void InitializeWithSphere(const RwV3d *pCenter, RwReal radius);
  // overlap turu — HandleEvents bunu çağırır; test doğrudan çağırabilir.
  void PollOverlaps();
  virtual void OnEntityEnter(void *pEntity);
  virtual void OnEntityLeave(void *pEntity);

private:
  RpClump *m_pClump;
  void *m_pPhantom;                 // btGhostObject* (PhysicsBackend)
  std::vector<void *> m_overlaps;   // overlap bodies from previous turn
  RwInt32 m_iEnterSpeakCategory;
  RwInt32 m_iEnterSpeakIndex;
  RwInt32 m_iLeaveSpeakCategory;
  RwInt32 m_iLeaveSpeakIndex;
};

namespace CollisionTrigger
{

    typedef ::CCollisionTrigger CCollisionTrigger;
}

#endif // COLLISIONTRIGGER_H

