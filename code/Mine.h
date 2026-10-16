#ifndef MINE_H
#define MINE_H

// ============================================================================
// CannonCruise - Naval Sea Mine Entity (Mine.h)
// Original path: D:\Projects\CannonCruisePC\code\Mine.cpp
// Tersine muhendislik ile Ghidra ve CC_ida.asm analiz edilerek yeniden yazilmistir.
//
// Binary Fonksiyonlari:
//   sub_495E90 - CClassFactory::Register("CMine", 0xE4)
//   sub_495ED0 - CMine constructor & event registration
//   Audio_LayMine, Audio_MineImpact, Mine_collision
// ============================================================================

#include "PhysicsEntity.h"
#include "framework/core/attributehandler/cclassfactory.h"
#include <rwcore.h>

/**
 * @class CMine
 * @brief Floating naval mine detonating on ship hull collision.
 *
 * Inherits buoyancy and water interaction from CPhysicsEntity.
 * Size in original binary: 0xE4 (228 bytes).
 * Triggers audio cues "Audio_LayMine", "Audio_MineImpact", and "Mine_collision".
 */
class CMine : public CPhysicsEntity {
public:
  /**
   * @brief Default constructor - registers instance with CClassFactory.
   * @see sub_495ED0
   */
  CMine();

  /**
   * @brief Destructor.
   */
  virtual ~CMine();

  /**
   * @brief Framework message dispatcher.
   * @param msg Incoming message descriptor.
   */
  virtual void HandleEvents(const RWS::CMsg &msg);

  /**
   * @brief Spawns mine at given world position with explosive parameters.
   * @param pos World coordinate where mine floats.
   * @param damage Explosive damage dealt to nearby ships (default 50.0).
   * @param radius Blast radius in world units (default 5.0).
   */
  void Spawn(const RwV3d &pos, RwReal damage = 50.0f, RwReal radius = 5.0f);

  /**
   * @brief Detonates mine, deals damage, and marks entity for removal.
   */
  void Explode();

  /** @brief Returns TRUE if mine has exploded and is pending deletion. */
  RwBool IsExploded() const { return m_bExploded; }

  /** @brief Returns explosive damage value. */
  RwReal GetDamage() const { return m_fDamage; }

  /** @brief Returns explosive blast radius. */
  RwReal GetRadius() const { return m_fBlastRadius; }

private:
  RwReal m_fDamage;       ///< Hull damage dealt on direct impact.
  RwReal m_fBlastRadius;  ///< Explosion effect and splash area.
  RwBool m_bExploded;     ///< True once impact detonation occurs.
  RwReal m_fArmTimer;     ///< Arming delay timer after being laid.
};

#endif // MINE_H