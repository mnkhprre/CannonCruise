#pragma once

// ============================================================================
// CannonCruise - Lifebuoy Score Pickup (Lifebuoy.h)
// Original path: D:\Projects\CannonCruisePC\code\Lifebuoy.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include "Flotsam.h"

namespace RWS {
/**
 * @class CLifebuoy
 * @brief Floating life ring / rescue buoy rewarding player score bonus upon pickup.
 *
 * Inherits flotation physics from CFlotsam. Dispatches `EVENT_LIFEBUOY_COLLECTED`
 * containing score award (default 100 points) and plays audio feedback on collect.
 */
class CLifebuoy : public CFlotsam {
public:
  RWS_MAKENEWCLASS(CLifebuoy);
  RWS_DECLARE_CLASS(CLifebuoy);

  /**
   * @brief Constructs lifebuoy instance from attribute packet.
   * @param packet  Attribute initialization packet.
   */
  CLifebuoy(const CAttributePacket &packet);

  /**
   * @brief Destructor.
   */
  virtual ~CLifebuoy();

  /**
   * @brief Attribute dispatcher for score bonus configuration.
   * @param packet  Contains score bonus command payload.
   */
  virtual void HandleAttributes(const CAttributePacket &packet);

  /**
   * @brief Message dispatcher (forwards to CFlotsam).
   * @param msg  Message descriptor.
   */
  virtual void HandleEvents(CMsg &msg);

  /**
   * @brief Collection handler – awards score, fires event, and despawns.
   */
  virtual void OnCollect();

private:
  RwInt32 m_scoreBonus;              ///< Points awarded to player upon retrieval (offset 0xac).
  CEventId m_eventLifebuoyCollected; ///< Event ID ("EVENT_LIFEBUOY_COLLECTED", offset 0xb0).
};
} // namespace RWS
