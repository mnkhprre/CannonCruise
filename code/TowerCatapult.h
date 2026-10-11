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

/**
 * @class CTowerCatapult
 * @brief Stationary defensive catapult fortification defending island coastlines.
 *
 * Inherits physical presence and collision processing from CPhysicsEntity.
 * Maintains dual visual states (undamaged clump vs damaged rubble clump),
 * takes damage from ship cannonball impacts, and transitions to destroyed
 * rubble upon health depletion with dedicated audio cue ("Audio_TowerDestroy").
 */
class CTowerCatapult : public CPhysicsEntity {
public:
  /**
   * @brief Constructs catapult tower and increments active world tower counter.
   */
  CTowerCatapult();

  /**
   * @brief Destructor – releases undamaged and damaged clumps and decrements counter.
   */
  virtual ~CTowerCatapult();

  /**
   * @brief Framework message dispatcher.
   * @param msg  Message descriptor.
   */
  virtual void HandleEvents(const RWS::CMsg &msg);

  /**
   * @brief Binds 3D clumps for intact and destroyed states.
   * @param pUndamagedClump  Primary intact tower model clump.
   * @param pDamagedClump    Optional destroyed rubble model clump.
   */
  void Initialize(RpClump *pUndamagedClump, RpClump *pDamagedClump = 0);

  /**
   * @brief Inflicts damage to the fortification; triggers DestroyTower() at zero HP.
   * @param damage  Health deduction amount.
   */
  void TakeDamage(RwReal damage);

  /**
   * @brief Initiates destruction sequence, playing collapse audio and swapping mesh.
   */
  void DestroyTower();

  /**
   * @brief Queries total number of active catapult towers in the simulation.
   * @return Active tower count.
   */
  static RwUInt32 GetActiveTowerCount() { return sm_nTowersAddedToWorlds; }

private:
  RpClump *m_pUndamagedClump; ///< Intact tower 3D clump.
  RpClump *m_pDamagedClump;   ///< Destroyed/collapsed tower 3D clump.
  RwReal m_fHealth;           ///< Hit point durability (default 150.0).
  RwBool m_bDestroyed;        ///< TRUE once destroyed.

  static RwUInt32 sm_nTowersAddedToWorlds; ///< Global counter of active towers in simulation.
};

#endif // TOWERCATAPULT_H
