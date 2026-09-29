#ifndef SHIPENEMY_H
#define SHIPENEMY_H

// ============================================================================
// CannonCruise - Enemy AI Ship (ShipEnemy.h)
// Original path: D:\Projects\CannonCruisePC\code\ShipEnemy.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include "Ship.h"
#include <rwcore.h>

enum EEnemyAIState { STATE_PATROL = 0, STATE_GUARD, STATE_HUNT, STATE_ATTACK };

class CShipEnemy : public CShip {
public:
  CShipEnemy();
  virtual ~CShipEnemy();

  virtual void HandleEvents(const RWS::CMsg &msg);

  void UpdateAI(RwReal deltaTime);
  void SetAIState(EEnemyAIState state);

  static RwUInt32 GetActiveEnemyShipCount() { return sm_nShipsAddedToWorlds; }

private:
  EEnemyAIState m_eState;
  RwReal m_fHuntTimer;
  RwInt32 m_iCurrentWaypoint;
  RwInt32 m_iAttackSpeakCategory;
  RwInt32 m_iAttackSpeakIndex;
  RwInt32 m_iDeathSpeakCategory;
  RwInt32 m_iDeathSpeakIndex;

  static RwUInt32 sm_nShipsAddedToWorlds;
};

#endif // SHIPENEMY_H
