#include "Fish.h"
#include "AudioManager.h"
#include "Scene.h"
#include "framework/core/attributehandler/cclassfactory.h"
#include "framework/core/attributehandler/csystemcommands.h"
#include "framework/core/resourcemanager/cresourcemanager.h"
#include "framework/toolkits/world/helpers/clumphelper.h"

namespace RWS {
RWS_REGISTER_CLASS(CFish);
RWS_REGISTER_CLASS(CFishGreen);

RwUInt32 CFish::ms_nFishAddedToWorlds = 0;

CFish::CFish(const CAttributePacket &packet)
    : CAttributeHandler(packet), CEventHandler(0), m_pPhysEntity(0),
      m_pClump(0), m_pAnimController(0), m_state(STATE_SWIM), m_swimSpeed(3.0f),
      m_turnRate(1.0f), m_jumpForce(5.0f), m_healthValue(25.0f),
      m_bAddedToWorld(FALSE) {
  m_pAnimController = new CAnimController();

  const void *pClumpData = CSystemCommands::ExtractClump(packet);
  if (pClumpData) {
    m_pClump = ClumpHelper::CreateClumpFromResource(pClumpData);
    if (m_pClump && m_pAnimController) {
      m_pAnimController->SetTarget(m_pClump);
    }
  }

  RegisterForMessage(iMsgRunningTick);
  AddToWorld();
}

CFish::~CFish() {
  UnregisterForMessage(iMsgRunningTick);

  if (m_bAddedToWorld) {
    RWS_ASSERT(ms_nFishAddedToWorlds != 0, "ms_nFishAddedToWorlds != 0 Failed");
    --ms_nFishAddedToWorlds;
  }

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

void CFish::HandleAttributes(const CAttributePacket &packet) {
  CAttributeHandler::HandleAttributes(packet);

  if (m_pClump) {
    ClumpHelper::HandleAttributes(m_pClump, packet);
    if (m_pAnimController) {
      m_pAnimController->HandleAttributes(packet);
    }
  }

  CAttributeCommandIterator it(packet, RWS_CLASS_INDEX(CFish));
  while (!it.IsFinished()) {
    switch (it->GetCommandId()) {
    case 1:
      m_turnRate = *reinterpret_cast<const RwReal *>(it->GetCommandData());
      break;
    case 2:
      m_swimSpeed = *reinterpret_cast<const RwReal *>(it->GetCommandData());
      break;
    case 3:
      m_state = static_cast<FishState>(
          *reinterpret_cast<const RwInt32 *>(it->GetCommandData()));
      break;
    case 4:
      m_jumpForce = *reinterpret_cast<const RwReal *>(it->GetCommandData());
      break;
    case 5:
      m_healthValue = *reinterpret_cast<const RwReal *>(it->GetCommandData());
      break;
    default:
      break;
    }
    ++it;
  }
}

void CFish::HandleEvents(CMsg &msg) {
  if (msg.Id == iMsgRunningTick) {
    Update(0.0166667f);
  }
}

void CFish::AddToWorld() {
  if (!m_bAddedToWorld && m_pClump) {
    m_bAddedToWorld = TRUE;
    ++ms_nFishAddedToWorlds;
    RpWorld *pWorld = CScene::GetWorld();
    if (pWorld) {
      RpWorldAddClump(pWorld, m_pClump);
    }
  }
}

void CFish::RemoveFromWorld() {
  if (m_bAddedToWorld) {
    m_bAddedToWorld = FALSE;
    RWS_ASSERT(m_pClump != 0, "m_pClump != NULL Failed");
    RpWorld *pWorld = CScene::GetWorld();
    if (pWorld && m_pClump) {
      RpWorldRemoveClump(pWorld, m_pClump);
    }
  }
}

void CFish::Update(RwReal deltaTime) {
  if (m_pAnimController) {
    m_pAnimController->Update(deltaTime);
  }
}

void CFish::OnPickup() {
  AudioManager::PlaySound("Audio_HealthPickup");
  m_state = STATE_DEAD;
  RemoveFromWorld();
}

// CFishGreen
CFishGreen::CFishGreen(const CAttributePacket &packet) : CFish(packet) {}

CFishGreen::~CFishGreen() {}

void CFishGreen::HandleAttributes(const CAttributePacket &packet) {
  CFish::HandleAttributes(packet);
}

void CFishGreen::HandleEvents(CMsg &msg) { CFish::HandleEvents(msg); }
} // namespace RWS
