#include "Lifebuoy.h"
#include "AudioManager.h"
#include "framework/core/attributehandler/cclassfactory.h"
#include "framework/core/attributehandler/csystemcommands.h"

namespace RWS {
RWS_REGISTER_CLASS(CLifebuoy);

CLifebuoy::CLifebuoy(const CAttributePacket &packet)
    : CFlotsam(packet), m_scoreBonus(100) {
  m_eventLifebuoyCollected.SetId("EVENT_LIFEBUOY_COLLECTED");
}

CLifebuoy::~CLifebuoy() {}

void CLifebuoy::HandleAttributes(const CAttributePacket &packet) {
  CFlotsam::HandleAttributes(packet);

  CAttributeCommandIterator it(packet, RWS_CLASS_INDEX(CLifebuoy));
  while (!it.IsFinished()) {
    switch (it->GetCommandId()) {
    case 0:
      m_scoreBonus = *reinterpret_cast<const RwInt32 *>(it->GetCommandData());
      break;
    default:
      break;
    }
    ++it;
  }
}

void CLifebuoy::HandleEvents(CMsg &msg) { CFlotsam::HandleEvents(msg); }

void CLifebuoy::OnCollect() {
  AudioManager::PlaySound("Audio_HealthPickup");
  CMsg msg(m_eventLifebuoyCollected, &m_scoreBonus);
  SendMsg(msg);
  CFlotsam::OnCollect();
}
} // namespace RWS
