#include "FernandoActor.h"
#include "Scene.h"
#include "framework/core/attributehandler/cclassfactory.h"
#include "framework/core/attributehandler/csystemcommands.h"
#include "framework/core/resourcemanager/cresourcemanager.h"
#include "framework/toolkits/world/helpers/clumphelper.h"

namespace RWS {
RWS_REGISTER_CLASS(CFernandoActor);

CFernandoActor::CFernandoActor(const CAttributePacket &packet)
    : CActor(packet), m_currentState(0), m_mastVisible(1), m_idleTimer(0.0f) {
  for (int i = 0; i < ANIM_COUNT; ++i) {
    m_animSpeeds[i] = 1.0f;
    m_animIndices[i] = -1;
  }

  // Initialize events
  m_eventChangeMastVisibility.SetId("EVENT_CHANGE_MAST_VISIBILITY");
  m_eventPlayerShipDeath.SetId("PlayerShipDeath");
  m_eventPlayerShipActorTakeDamage.SetId("EVENT_PLAYERSHIPACTOR_TAKEDAMAGE");
  m_eventInqPlayerShipTurnDamper.SetId("EVENT_INQ_PLAYERSHIP_TURNDAMPERV");
  m_eventPlayerShipDetachAllFrames.SetId("EVENT_PLAYERSHIP_DETACH_ALL_FRAM");
  m_eventPlayerShipAddFernandoFrame.SetId("EVENT_PLAYERSHIP_ADD_FERNANDOFRA");

  RegisterForMessage(m_eventChangeMastVisibility);
  RegisterForMessage(m_eventPlayerShipDeath);
  RegisterForMessage(m_eventPlayerShipActorTakeDamage);
  RegisterForMessage(m_eventPlayerShipDetachAllFrames);
  RegisterForMessage(m_eventPlayerShipAddFernandoFrame);

  AddToWorld();
}

CFernandoActor::~CFernandoActor() {
  UnregisterForMessage(m_eventChangeMastVisibility);
  UnregisterForMessage(m_eventPlayerShipDeath);
  UnregisterForMessage(m_eventPlayerShipActorTakeDamage);
  UnregisterForMessage(m_eventPlayerShipDetachAllFrames);
  UnregisterForMessage(m_eventPlayerShipAddFernandoFrame);
}

void CFernandoActor::HandleAttributes(const CAttributePacket &packet) {
  CActor::HandleAttributes(packet);

  CAttributeCommandIterator it(packet, RWS_CLASS_INDEX(CFernandoActor));
  while (!it.IsFinished()) {
    RwUInt32 cmdId = it->GetCommandId();
    if (cmdId < ANIM_COUNT) {
      m_animSpeeds[cmdId] =
          *reinterpret_cast<const RwReal *>(it->GetCommandData());
      if (m_pAnimController && m_animIndices[cmdId] >= 0) {
        m_pAnimController->SetAnimSpeed(m_animIndices[cmdId],
                                        m_animSpeeds[cmdId]);
      }
    }
    ++it;
  }
}

void CFernandoActor::HandleEvents(CMsg &msg) {
  if (msg.Id == m_eventChangeMastVisibility) {
    RwInt32 visible = *reinterpret_cast<const RwInt32 *>(msg.pData);
    SetMastVisibility(visible != 0);
  } else if (msg.Id == m_eventPlayerShipActorTakeDamage) {
    PlayAnimation(ANIM_HIT);
  } else if (msg.Id == m_eventPlayerShipDeath) {
    PlayAnimation(ANIM_FLY_LOOP);
  } else if (msg.Id == iMsgRunningTick) {
    if (m_pAnimController) {
      m_pAnimController->Update(0.0333f);
    }
  } else {
    CActor::HandleEvents(msg);
  }
}

void CFernandoActor::PlayAnimation(AnimState state, RwReal speed) {
  if (state >= 0 && state < ANIM_COUNT) {
    m_currentState = state;
    if (m_pAnimController && m_animIndices[state] >= 0) {
      m_pAnimController->SetAnimSpeed(m_animIndices[state],
                                      speed * m_animSpeeds[state]);
    }
  }
}

void CFernandoActor::SetMastVisibility(RwBool visible) {
  m_mastVisible = visible ? 1 : 0;
  if (m_pClump) {
    // Clump visibility toggle
  }
}
} // namespace RWS
