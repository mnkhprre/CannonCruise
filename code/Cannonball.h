#ifndef CANNONBALL_H
#define CANNONBALL_H

// ============================================================================
// CannonCruise - Cannonball Projectile (Cannonball.h)
// Original path: D:\Projects\CannonCruisePC\code\Cannonball.h
// Reverse-engineered via Ghidra and CC.asm analysis.
// ============================================================================

#include "PhysicsEntity.h"
#include <rpworld.h>
#include <rwcore.h>

/**
 * @enum ECannonballType
 * @brief Identifies the projectile ammo type fired from cannons.
 *
 * Different types may feature varying visual effects, damage profiles,
 * and physics characteristics during flight and detonation.
 */
enum ECannonballType {
  CANNONBALL_STANDARD = 0,  ///< Standard iron ball, primary ammunition.
  CANNONBALL_LAVA,          ///< Incendiary lava ball causing splash/burn damage.
  CANNONBALL_PLASMA,        ///< High-energy plasma round.
  CANNONBALL_COUNT          ///< Total number of distinct cannonball variants.
};

/**
 * @struct SCannonballSpawnData
 * @brief Initialization parameters passed to CCannonball::Spawn.
 *
 * Contains launch kinematics, projectile classification, and originating
 * entity identity for attribution and friendly-fire filtering.
 */
struct SCannonballSpawnData {
  RwV3d m_Pos;              ///< Initial world-space muzzle position.
  RwV3d m_Vel;              ///< Initial velocity vector in world units/sec.
  ECannonballType m_Type;   ///< Visual and damage variant.
  RwUInt32 m_uOwnerID;      ///< Entity ID of firing actor (e.g. player ship or enemy).
};

/**
 * @class CCannonball
 * @brief Physics-driven projectile spawned when ships or defenses fire cannons.
 *
 * Inherits from CPhysicsEntity to participate in Havok/custom physics integration,
 * handling ballistic trajectory, water impacts, ship hull collisions, and lifetime
 * expiration. Tracks global projectile count for resource budget monitoring.
 */
class CCannonball : public CPhysicsEntity {
public:
  /**
   * @brief Constructs a cannonball instance and increments active world counter.
   *
   * Initialises default timers, clears clump pointer, and registers itself
   * into the global tracking statistics.
   */
  CCannonball();

  /**
   * @brief Destructor – releases graphical assets and decrements active world counter.
   *
   * Ensures the clump is destroyed and asserts that active count remains non-negative.
   */
  virtual ~CCannonball();

  /**
   * @brief Message handler for framework events.
   * @param msg  Incoming message descriptor.
   */
  virtual void HandleEvents(const RWS::CMsg &msg);

  /**
   * @brief Spawns the projectile into the physics world with initial parameters.
   * @param spawnData  Spawn configuration including position, velocity, and type.
   *
   * Assigns kinematic state, sets lifetime (default 5.0s), adds the entity
   * to the active simulation world, and plays muzzle audio.
   */
  void Spawn(const SCannonballSpawnData &spawnData);

  /**
   * @brief Per-frame simulation tick.
   * @param deltaTime  Elapsed time since previous frame in seconds.
   *
   * Counts down lifetime; if it drops below zero, despawns the projectile.
   */
  void Update(RwReal deltaTime);

  /**
   * @brief Callback invoked upon physical contact with another entity or surface.
   * @param pTarget   Pointer to hit entity or geometry (or NULL).
   * @param hitType   Collision classification (0: ship/target, 1: water splash, other: static mesh).
   *
   * Triggers contextual impact audio, visual FX, and removes the projectile from the world.
   */
  void OnHit(void *pTarget, RwUInt32 hitType);

  /**
   * @brief Queries the total number of cannonballs currently active in game worlds.
   * @return Count of live cannonball instances.
   */
  static RwUInt32 GetActiveCannonballCount() {
    return sm_nCannonBallsAddedToWorlds;
  }

private:
  RpClump *m_pClump;          ///< RenderWare Clump model for 3D visual representation.
  ECannonballType m_eType;    ///< Projectile ammo type.
  RwReal m_fLifeTimer;        ///< Remaining lifespan timer in seconds before auto-removal.
  RwUInt32 m_uOwnerID;        ///< ID of firing owner entity to prevent self-collision.

  static RwUInt32 sm_nCannonBallsAddedToWorlds; ///< Global counter of active cannonballs.
};

#endif // CANNONBALL_H
