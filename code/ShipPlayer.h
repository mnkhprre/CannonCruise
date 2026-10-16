#ifndef SHIPPLAYER_H
#define SHIPPLAYER_H

// ============================================================================
// CannonCruise - Player Ship Entity (ShipPlayer.h)
// Original path: D:\Projects\CannonCruisePC\code\ShipPlayer.cpp
// Tersine muhendislik ile Ghidra ve CC_ida.asm analiz edilerek yeniden yazilmistir.
//
// Binary Fonksiyonlari:
//   sub_4AE6C0 - CClassFactory::Register("CShipPlayer", 0x488)
//   sub_4AE700 - CShipPlayer constructor & event registration
//   sub_4AF3C0 - CShipPlayer per-frame update & physics
// ============================================================================

#include "Ship.h"
#include "framework/core/attributehandler/cclassfactory.h"
#include <rwcore.h>

/**
 * @class CShipPlayer
 * @brief Human player-controlled vessel with score tracking, respawning, and camera focus.
 *
 * Inherits naval dynamics and combat mechanics from CShip.
 * Registered with CClassFactory as "CShipPlayer" (binary size 0x488 = 1160 bytes).
 * Handles storm conditions, dynamic/static score increments, lifebuoys, and respawn fades.
 */
class CShipPlayer : public CShip {
public:
  /**
   * @brief Constructs player ship, registers events, and stores singleton pointer.
   * @see sub_4AE700
   */
  CShipPlayer();

  /**
   * @brief Destructor. Cleans up singleton pointer.
   */
  virtual ~CShipPlayer();

  /**
   * @brief Framework message dispatcher.
   * @param msg Incoming event message.
   */
  virtual void HandleEvents(const RWS::CMsg &msg);

  /**
   * @brief Per-frame update tick for player mechanics.
   * @param deltaTime Frame time in seconds.
   */
  void UpdatePlayer(RwReal deltaTime);

  /**
   * @brief Respawns player vessel at specified position.
   */
  void Respawn(const RwV3d &spawnPos);

  /**
   * @brief Adds to static score (targets destroyed, collectibles).
   */
  void IncreaseStaticScore(RwInt32 points) { m_iStaticScore += points; }

  /**
   * @brief Adds to dynamic score (combo points, time bonuses).
   */
  void IncreaseDynamicScore(RwInt32 points) { m_iDynamicScore += points; }

  /** @brief Returns total static score. */
  RwInt32 GetStaticScore() const { return m_iStaticScore; }

  /** @brief Returns total dynamic score. */
  RwInt32 GetDynamicScore() const { return m_iDynamicScore; }

  /** @brief Returns total combined score. */
  RwInt32 GetTotalScore() const { return m_iStaticScore + m_iDynamicScore; }

  /** @brief Returns whether storm weather is currently active. */
  RwBool IsStormActive() const { return m_bStormActive; }

  /**
   * @brief Returns currently active player ship instance (if spawned).
   */
  static CShipPlayer *GetPlayerShip() { return sm_pPlayerShip; }

private:
  RwInt32 m_iStaticScore;     ///< Accumulated static level score.
  RwInt32 m_iDynamicScore;    ///< Dynamic multiplier score.
  RwBool  m_bStormActive;     ///< Storm weather effect toggle.
  RwBool  m_bRespawning;      ///< True while respawn fade transition is playing.
  RwReal  m_fRespawnTimer;    ///< Respawn sequence timer.

  static CShipPlayer *sm_pPlayerShip; ///< Global player ship instance pointer.
};

#endif // SHIPPLAYER_H