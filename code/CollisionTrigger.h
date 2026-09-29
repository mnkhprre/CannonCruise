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

class CCollisionTrigger : public RWS::CEventHandler {
public:
  CCollisionTrigger();
  virtual ~CCollisionTrigger();

  virtual void HandleEvents(const RWS::CMsg &msg);

  void Initialize(RpClump *pClump);
  void OnEntityEnter(void *pEntity);
  void OnEntityLeave(void *pEntity);

private:
  RpClump *m_pClump;
  void *m_pPhantom;
  RwInt32 m_iEnterSpeakCategory;
  RwInt32 m_iEnterSpeakIndex;
  RwInt32 m_iLeaveSpeakCategory;
  RwInt32 m_iLeaveSpeakIndex;
};

#endif // COLLISIONTRIGGER_H
