#include "Actor.h"
#include "Scene.h"
#include "framework/core/attributehandler/cclassfactory.h"
#include "framework/core/attributehandler/csystemcommands.h"
#include "framework/core/resourcemanager/cresourcemanager.h"
#include "framework/toolkits/world/helpers/clumphelper.h"

namespace RWS {
RWS_REGISTER_CLASS(CActor);

CActor::CActor(const CAttributePacket &packet)
    : CAttributeHandler(packet), CEventHandler(0), m_pAnimController(0),
      m_pClump(0), m_bAddedToWorld(FALSE) {
  m_pAnimController = new CAnimController();

  // Process initial clump from packet if present
  const void *pClumpData = CSystemCommands::ExtractClump(packet);
  if (pClumpData) {
    m_pClump = ClumpHelper::CreateClumpFromResource(pClumpData);
    if (m_pClump && m_pAnimController) {
      m_pAnimController->SetTarget(m_pClump);
    }
  }

  // Register for tick message
  RegisterForMessage(iMsgRunningTick);
}

CActor::~CActor() {
  UnregisterForMessage(iMsgRunningTick);

  if (m_pAnimController) {
    delete m_pAnimController;
    m_pAnimController = 0;
  }

  RemoveFromWorld();

  if (m_pClump) {
    ClumpHelper::DestroyClump(m_pClump);
    m_pClump = 0;
  }
}

void CActor::HandleAttributes(const CAttributePacket &packet) {
  CAttributeHandler::HandleAttributes(packet);

  if (m_pClump) {
    ClumpHelper::HandleAttributes(m_pClump, packet);
    if (m_pAnimController) {
      m_pAnimController->HandleAttributes(packet);
    }
  }
}

void CActor::HandleEvents(CMsg &msg) {
  const void *pClumpData = CSystemCommands::ExtractClump(msg);
  if (pClumpData) {
    m_pClump = ClumpHelper::CreateClumpFromResource(pClumpData);
    if (m_pAnimController && m_pClump) {
      m_pAnimController->SetTarget(m_pClump);
    }
  }
}

void CActor::AddToWorld() {
  if (!m_bAddedToWorld && m_pClump) {
    m_bAddedToWorld = TRUE;
    RpWorld *pWorld = CScene::GetWorld();
    if (pWorld) {
      RpWorldAddClump(pWorld, m_pClump);
    }
  }
}

void CActor::RemoveFromWorld() {
  if (m_bAddedToWorld) {
    m_bAddedToWorld = FALSE;
    RWS_ASSERT(m_pClump != 0, "m_pClump != NULL Failed");
    RpWorld *pWorld = CScene::GetWorld();
    RWS_ASSERT(pWorld != 0, "ms_pSingleton != NULL Failed");
    if (pWorld && m_pClump) {
      RpWorldRemoveClump(pWorld, m_pClump);
    }
  }
}
} // namespace RWS
