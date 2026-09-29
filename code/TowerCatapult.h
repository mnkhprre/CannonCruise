#ifndef TOWERCATAPULT_H
#define TOWERCATAPULT_H

// ============================================================================
// CannonCruise - Tower Catapult Defense Entity (TowerCatapult.h)
// Original path: D:\Projects\CannonCruisePC\code\TowerCatapult.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include "PhysicsEntity.h"
#include <rpworld.h>
#include <rwcore.h>

class CTowerCatapult : public CPhysicsEntity {
public:
  CTowerCatapult();
  virtual ~CTowerCatapult();

  virtual void HandleEvents(const RWS::CMsg &msg);

  void Initialize(RpClump *pUndamagedClump, RpClump *pDamagedClump = 0);
  void TakeDamage(RwReal damage);
  void DestroyTower();

  static RwUInt32 GetActiveTowerCount() { return sm_nTowersAddedToWorlds; }

private:
  RpClump *m_pUndamagedClump;
  RpClump *m_pDamagedClump;
  RwReal m_fHealth;
  RwBool m_bDestroyed;

  static RwUInt32 sm_nTowersAddedToWorlds;
};

#endif // TOWERCATAPULT_H
