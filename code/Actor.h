#pragma once
#include "AnimController.h"
#include "framework/core/attributehandler/cattributehandler.h"
#include "framework/core/eventhandler/ceventhandler.h"
#include <rpworld.h>
#include <rwcore.h>

namespace RWS {
class CActor : public CAttributeHandler, public CEventHandler {
public:
  RWS_MAKENEWCLASS(CActor);
  RWS_DECLARE_CLASS(CActor);

  CActor(const CAttributePacket &packet);
  virtual ~CActor();

  virtual void HandleAttributes(const CAttributePacket &packet);
  virtual void HandleEvents(CMsg &msg);

  void AddToWorld();
  void RemoveFromWorld();

  RpClump *GetClump() const { return m_pClump; }
  CAnimController *GetAnimController() const { return m_pAnimController; }
  RwBool IsAddedToWorld() const { return m_bAddedToWorld; }

protected:
  CAnimController *m_pAnimController; // 0x24
  RpClump *m_pClump;                  // 0x28
  RwBool m_bAddedToWorld;             // 0x2c
};
} // namespace RWS
