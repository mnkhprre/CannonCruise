#ifndef SHIP_H
#define SHIP_H

// ============================================================================
// CannonCruise - Ship Base Class (Ship.h)
// Original path: D:\Projects\CannonCruisePC\code\Ship.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include "Cannon.h"
#include "PhysicsEntity.h"
#include <rpworld.h>
#include <rwcore.h>

class CShip : public CPhysicsEntity {
public:
  CShip();
  virtual ~CShip();

  virtual void HandleEvents(const RWS::CMsg &msg);

  void Initialize(RpClump *pClump);
  void Update(RwReal deltaTime);

  void SetThrottle(RwReal fThrottle);
  void SetRudder(RwReal fRudder);

  void FirePortCannons(ECannonballType type = CANNONBALL_STANDARD);
  void FireStarboardCannons(ECannonballType type = CANNONBALL_STANDARD);

  void TakeDamage(RwReal damage);
  void Sink();

  RwReal GetHealth() const { return m_fHealth; }
  RwReal GetSpeed() const { return m_fCurrentSpeed; }

protected:
  RpClump *m_pClump;
  RwFrame *m_pRudderFrame;
  RwFrame *m_pWheelFrame;

  CCannon m_PortCannons;
  CCannon m_StarboardCannons;

  RwReal m_fHealth;
  RwReal m_fMaxHealth;
  RwReal m_fCurrentSpeed;
  RwReal m_fTargetSpeed;
  RwReal m_fRudderAngle;
  RwBool m_bSinking;
};

#endif // SHIP_H
