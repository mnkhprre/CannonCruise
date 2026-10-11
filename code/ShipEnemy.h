#ifndef SHIPENEMY_H
#define SHIPENEMY_H

// ============================================================================
// CannonCruise - Enemy AI Ship (ShipEnemy.h)
// Original path: D:\Projects\CannonCruisePC\code\ShipEnemy.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include "Ship.h"
#include <rwcore.h>

/**
 * @enum EEnemyAIState
 * @brief Finite state machine modes governing hostile vessel behavior.
 */
enum EEnemyAIState {
  STATE_PATROL = 0, ///< Cruising along defined patrol route waypoints.
  STATE_GUARD,      ///< Anchored or holding position to defend a tactical perimeter.
  STATE_HUNT,       ///< Searching and closing the distance toward the player vessel.
  STATE_ATTACK      ///< Engaging target with broadside cannon batteries and battle cries.
};

/**
 * @class CShipEnemy
 * @brief AI-controlled hostile warship navigating waterways and engaging the player.
 *
 * Inherits all naval mechanics from CShip (hull health, rudder control, port and
 * starboard cannon salvos) while adding an autonomous state machine (EEnemyAIState),
 * waypoint route following via `EVENT_INQUIRE_WAYPOINT`, voice line taunts via
 * CSpeakManager, and dedicated health bar UI linkage (`EnemyHealthBarRender`).
 */
class CShipEnemy : public CShip {
public:
  /**
   * @brief Default constructor – initialises AI attributes and registers instance.
   *
   * Increments the global active enemy ship counter (sm_nShipsAddedToWorlds)
   * and sets starting state to STATE_PATROL.
   */
  CShipEnemy();

  /**
   * @brief Destructor – decrements active enemy ship counter and cleans up state.
   */
  virtual ~CShipEnemy();

  /**
   * @brief Framework message dispatcher.
   * @param msg  Incoming message event (e.g. iMsgRunningTick).
   */
  virtual void HandleEvents(const RWS::CMsg &msg);

  /**
   * @brief Evaluates AI decisions, pathfinding, and combat maneuvers for this frame.
   * @param deltaTime  Frame elapsed duration in seconds.
   */
  void UpdateAI(RwReal deltaTime);

  /**
   * @brief Transitions the vessel into a new behavioral mode.
   * @param state  Target state (PATROL, GUARD, HUNT, ATTACK).
   *
   * When switching to STATE_ATTACK, triggers enemy vocal taunts and audio cues
   * via CSpeakManager if configured.
   */
  void SetAIState(EEnemyAIState state);

  /**
   * @brief Returns current total of active enemy ships in the game world.
   * @return Active enemy ship count.
   */
  static RwUInt32 GetActiveEnemyShipCount() { return sm_nShipsAddedToWorlds; }

private:
  EEnemyAIState m_eState;           ///< Current active AI operational state.
  RwReal m_fHuntTimer;              ///< Elapsed tracking duration when chasing target.
  RwInt32 m_iCurrentWaypoint;       ///< Index of next navigation waypoint in route.
  RwInt32 m_iAttackSpeakCategory;   ///< Voice line category for attack shouts (default 1).
  RwInt32 m_iAttackSpeakIndex;      ///< Voice line index for attack shouts.
  RwInt32 m_iDeathSpeakCategory;    ///< Voice line category for sinking screams (default 2).
  RwInt32 m_iDeathSpeakIndex;       ///< Voice line index for sinking screams.

  static RwUInt32 sm_nShipsAddedToWorlds; ///< Global counter of active enemy ships.
};

#endif // SHIPENEMY_H
