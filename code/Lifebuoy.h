#pragma once
#include "Flotsam.h"

namespace RWS {
class CLifebuoy : public CFlotsam {
public:
  RWS_MAKENEWCLASS(CLifebuoy);
  RWS_DECLARE_CLASS(CLifebuoy);

  CLifebuoy(const CAttributePacket &packet);
  virtual ~CLifebuoy();

  virtual void HandleAttributes(const CAttributePacket &packet);
  virtual void HandleEvents(CMsg &msg);

  virtual void OnCollect();

private:
  RwInt32 m_scoreBonus;              // 0xac
  CEventId m_eventLifebuoyCollected; // 0xb0
};
} // namespace RWS
