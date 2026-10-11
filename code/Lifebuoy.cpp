// ============================================================================
// CannonCruise - Lifebuoy Score Pickup (Lifebuoy.cpp)
// Original path: D:\Projects\CannonCruisePC\code\Lifebuoy.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include "Lifebuoy.h"
#include "AudioManager.h"
#include "framework/core/attributehandler/cclassfactory.h"
#include "framework/core/attributehandler/csystemcommands.h"

namespace RWS {
// Register CLifebuoy with RenderWare Studio reflection system
RWS_REGISTER_CLASS(CLifebuoy);

// ---------------------------------------------------------------------------
// Constructor – initializes score bonus value and registers event ID.
// ---------------------------------------------------------------------------
CLifebuoy::CLifebuoy(const CAttributePacket &packet)
    : CFlotsam(packet), m_scoreBonus(100), m_eventLifebuoyCollected(0) {
}

// ---------------------------------------------------------------------------
// Destructor.
// ---------------------------------------------------------------------------
CLifebuoy::~CLifebuoy() {}

// ---------------------------------------------------------------------------
// HandleAttributes – dynamic property updater from workspace / level editor.
// Command mapping:
//   0: Score bonus award (RwInt32)
// ---------------------------------------------------------------------------
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

// ---------------------------------------------------------------------------
// HandleEvents – forwards framework events to base CFlotsam.
// ---------------------------------------------------------------------------
void CLifebuoy::HandleEvents(CMsg &msg) { CFlotsam::HandleEvents(msg); }

// ---------------------------------------------------------------------------
// OnCollect – plays sound, sends EVENT_LIFEBUOY_COLLECTED with bonus score,
// and extracts pickup entity from the active simulation world.
// ---------------------------------------------------------------------------
void CLifebuoy::OnCollect() {
  CAudioManager* pAudio = CAudioManager::GetSingletonPtr();
  if (pAudio) {
    pAudio->PlaySound("Audio_HealthPickup");
  }
  CMsg msg(m_eventLifebuoyCollected, &m_scoreBonus);
  CEventHandler::SendMsg(msg);
  CFlotsam::OnCollect();
}
} // namespace RWS

