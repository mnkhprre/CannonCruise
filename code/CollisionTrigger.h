#ifndef COLLISIONTRIGGER_H
#define COLLISIONTRIGGER_H

// ============================================================================
// CannonCruise - Collision Trigger Entity (CollisionTrigger.h)
// Original path: D:\Projects\CannonCruisePC\code\CollisionTrigger.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
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

  // binary: FUN_0046f5e0 — clump'tan bounding sphere alıp phantom oluşturur.
  // RW stub'ları dönen sphere'i NULL döndürür → librw'de gerçek veri akar.
  void Initialize(RpClump *pClump);
  // port eklentisi (test + clump'suz yol): AABB kutu phantom'ı kayıtlı
  // CPhysicsWorld singleton'ına ekler (sphere yarıçapından türetilmiş).
  void InitializeWithSphere(const RwV3d *pCenter, RwReal radius);
  // overlap turu — HandleEvents bunu çağırır; test doğrudan çağırabilir.
  void PollOverlaps();
  virtual void OnEntityEnter(void *pEntity);
  virtual void OnEntityLeave(void *pEntity);

private:
  RpClump *m_pClump;
  void *m_pPhantom;                 // btGhostObject* (PhysicsBackend)
  std::vector<void *> m_overlaps;   // önceki turdaki overlap gövdeleri
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

