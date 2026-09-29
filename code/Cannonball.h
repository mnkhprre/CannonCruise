#ifndef CANNONBALL_H
#define CANNONBALL_H

// ============================================================================
// CannonCruise - Cannonball Projectile (Cannonball.h)
// Original path: D:\Projects\CannonCruisePC\code\Cannonball.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include "PhysicsEntity.h"
#include <rpworld.h>
#include <rwcore.h>

enum ECannonballType {
  CANNONBALL_STANDARD = 0,
  CANNONBALL_LAVA,
  CANNONBALL_PLASMA,
  CANNONBALL_COUNT
};

struct SCannonballSpawnData {
  RwV3d m_Pos;
  RwV3d m_Vel;
  ECannonballType m_Type;
  RwUInt32 m_uOwnerID;
};

class CCannonball : public CPhysicsEntity {
public:
  CCannonball();
  virtual ~CCannonball();

  virtual void HandleEvents(const RWS::CMsg &msg);

  void Spawn(const SCannonballSpawnData &spawnData);
  void Update(RwReal deltaTime);
  void OnHit(void *pTarget, RwUInt32 hitType);

  static RwUInt32 GetActiveCannonballCount() {
    return sm_nCannonBallsAddedToWorlds;
  }

private:
  RpClump *m_pClump;
  ECannonballType m_eType;
  RwReal m_fLifeTimer;
  RwUInt32 m_uOwnerID;

  static RwUInt32 sm_nCannonBallsAddedToWorlds;
};

#endif // CANNONBALL_H
