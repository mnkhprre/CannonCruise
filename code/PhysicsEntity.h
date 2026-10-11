#ifndef PHYSICSENTITY_H
#define PHYSICSENTITY_H

// ============================================================================
// CannonCruise - Physics Entity Base (PhysicsEntity.h)
// Original path: D:\Projects\CannonCruisePC\code\PhysicsEntity.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include "framework/core/eventhandler/ceventhandler.h"
#include <rwcore.h>

/**
 * @class CPhysicsEntity
 * @brief Base class for all game objects integrated with the Havok physics simulation.
 *
 * Inherits from RWS::CEventHandler so that physical objects can register for framework
 * tick events, transformation broadcasts, and collision callbacks. Wraps an underlying
 * Havok rigid body instance (m_pRigidBody) and manages its lifecycle in CPhysicsWorld.
 *
 * Derived classes include CShip, CCannonball, CSpawnable, CFlotsam, and various
 * interactive debris/pickup entities.
 */
class CPhysicsEntity : public RWS::CEventHandler {
public:
  /**
   * @brief Constructs an uninitialized physics entity.
   *
   * Sets m_pRigidBody to NULL and marks the entity as not yet added to the world.
   */
  CPhysicsEntity();

  /**
   * @brief Virtual destructor – safely detaches the entity from the physics world.
   *
   * Invokes RemoveFromWorld() before destruction to prevent dangling references
   * in the active physics simulation.
   */
  virtual ~CPhysicsEntity();

  /**
   * @brief Framework message dispatcher.
   * @param msg  Message descriptor received from the event system.
   */
  virtual void HandleEvents(const RWS::CMsg &msg);

  /**
   * @brief Returns the underlying Havok rigid body pointer.
   * @return Pointer to hkpRigidBody, or NULL if not yet created.
   */
  void *GetRigidBody() const { return m_pRigidBody; }

  /**
   * @brief Associates a Havok rigid body with this entity.
   * @param pBody  Pointer to the allocated rigid body instance.
   */
  void SetRigidBody(void *pBody);

  /**
   * @brief Registers this entity's rigid body with the active CPhysicsWorld simulation.
   *
   * Validates that m_pRigidBody is non-null and that it is not already in the world,
   * then adds it to CPhysicsWorld and sets m_bInWorld to TRUE.
   */
  void AddToWorld();

  /**
   * @brief Deregisters this entity's rigid body from the active CPhysicsWorld simulation.
   *
   * If currently in the world, removes the body from CPhysicsWorld and sets
   * m_bInWorld to FALSE.
   */
  void RemoveFromWorld();

  /**
   * @brief Teleports or sets the physical position of the rigid body.
   * @param pPos  New position vector in world space.
   */
  void SetPosition(const RwV3d *pPos);

  /**
   * @brief Retrieves the current world-space position from the rigid body.
   * @param[out] pPos  Buffer receiving the coordinates.
   */
  void GetPosition(RwV3d *pPos) const;

  /**
   * @brief Applies or sets the linear velocity vector of the rigid body.
   * @param pVel  Velocity vector in world units per second.
   */
  void SetLinearVelocity(const RwV3d *pVel);

  /**
   * @brief Queries the current linear velocity of the rigid body.
   * @param[out] pVel  Buffer receiving the velocity vector.
   */
  void GetLinearVelocity(RwV3d *pVel) const;

protected:
  void *m_pRigidBody; ///< Pointer to Havok rigid body instance (hkpRigidBody*).
  RwBool m_bInWorld;  ///< Flag indicating whether the body is currently in CPhysicsWorld.
};

#endif // PHYSICSENTITY_H
