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

/**
 * @class CShip
 * @brief Base class representing a ship entity in the game world.
 *
 * Inherits from CPhysicsEntity for physics integration (buoyancy, collisions).
 * Encapsulates ship-specific state: health, speed, rudder steering, and
 * port/starboard cannon batteries. Subclassed by CShipEnemy for AI-controlled
 * vessels. All logic faithfully reproduces the original binary behaviour.
 */
class CShip : public CPhysicsEntity {
public:
  /** @brief Default constructor – sets all members to safe initial values. */
  CShip();

  /** @brief Virtual destructor – destroys the owned RpClump, if any. */
  virtual ~CShip();

  /**
   * @brief Receives engine-level messages each frame.
   * @param msg  The message descriptor (physics tick, running tick, etc.).
   *
   * Originally handles m_PhysicsWorldUpdateEvent and iMsgRunningTick.
   */
  virtual void HandleEvents(const RWS::CMsg &msg);

  /**
   * @brief Binds a 3-D model (clump) to this ship and locates sub-frames.
   * @param pClump  Pointer to the RpClump loaded from the asset file.
   *
   * Searches the clump hierarchy for the "Rudder" and "Wheel" frames
   * so they can be animated at runtime.
   */
  void Initialize(RpClump *pClump);

  /**
   * @brief Per-frame update – interpolates speed toward the throttle target.
   * @param deltaTime  Elapsed time since the previous frame (seconds).
   *
   * Skipped entirely while the ship is in the sinking state.
   */
  void Update(RwReal deltaTime);

  /**
   * @brief Sets the throttle input.
   * @param fThrottle  Normalised throttle value; multiplied by 25 to obtain
   *                   the target speed in world units per second.
   */
  void SetThrottle(RwReal fThrottle);

  /**
   * @brief Sets the rudder deflection angle.
   * @param fRudder  Rudder angle in radians (positive = starboard turn).
   */
  void SetRudder(RwReal fRudder);

  /**
   * @brief Fires all cannons on the port (left) side.
   * @param type  Projectile type (default: CANNONBALL_STANDARD).
   *
   * The fire direction is hard-coded to {-1, 0.2, 0} (left and slightly up).
   */
  void FirePortCannons(ECannonballType type = CANNONBALL_STANDARD);

  /**
   * @brief Fires all cannons on the starboard (right) side.
   * @param type  Projectile type (default: CANNONBALL_STANDARD).
   *
   * The fire direction is hard-coded to {1, 0.2, 0} (right and slightly up).
   */
  void FireStarboardCannons(ECannonballType type = CANNONBALL_STANDARD);

  /**
   * @brief Reduces health by the given amount; triggers Sink() at zero HP.
   * @param damage  Amount of damage to apply.
   */
  void TakeDamage(RwReal damage);

  /**
   * @brief Initiates the sinking sequence.
   *
   * Sets m_bSinking, plays the "Ship_Sink" audio cue via CAudioManager,
   * and prevents further Update() processing.
   */
  void Sink();

  /** @brief Returns current health value (0 .. m_fMaxHealth). */
  RwReal GetHealth() const { return m_fHealth; }

  /** @brief Returns the current forward speed in world units/sec. */
  RwReal GetSpeed() const { return m_fCurrentSpeed; }

protected:
  // ---- Rendering / transformation data ------------------------------------
  RpClump *m_pClump;          ///< The ship's 3-D model (owned, destroyed in dtor).
  RwFrame *m_pRudderFrame;    ///< Cached frame for rudder visual rotation.
  RwFrame *m_pWheelFrame;     ///< Cached frame for wheel/propeller animation.

  // ---- Weapon systems -----------------------------------------------------
  CCannon m_PortCannons;      ///< Cannon battery on the port (left) side.
  CCannon m_StarboardCannons; ///< Cannon battery on the starboard (right) side.

  // ---- Core ship attributes -----------------------------------------------
  RwReal m_fHealth;          ///< Current hit-points.
  RwReal m_fMaxHealth;       ///< Maximum hit-points (set at spawn).
  RwReal m_fCurrentSpeed;    ///< Actual forward speed (interpolated each frame).
  RwReal m_fTargetSpeed;     ///< Desired speed, derived from throttle input.
  RwReal m_fRudderAngle;     ///< Current rudder deflection (radians).
  RwBool m_bSinking;         ///< TRUE once Sink() has been called.
};

#endif // SHIP_H
