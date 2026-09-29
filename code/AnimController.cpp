#include "AnimController.h"
#include "framework/core/attributehandler/csystemcommands.h"
#include "framework/core/resourcemanager/cresourcemanager.h"
#include "framework/toolkits/Animation/AnimHelper.h"
#include "framework/toolkits/Animation/SkinSafe.h"
#include "framework/toolkits/world/helpers/clumphelper.h"

namespace RWS {
CAnimController::CAnimController()
    : m_pHierarchy(0), m_currentAnim(-1), m_currentTime(0.0f) {}

CAnimController::~CAnimController() {
  for (size_t i = 0; i < m_blends.size(); ++i) {
    // Release blend hierarchies
  }
  m_blends.clear();
  m_anims.clear();
}

void CAnimController::SetTarget(RpClump *pClump) {
  RWS_ASSERT(!HasTarget(), "Failed PRE condition (!HasTarget())");
  if (!pClump)
    return;

  m_pHierarchy = RpSkinSafeAtomicGetHAnimHierarchy(pClump);
  if (m_pHierarchy) {
    RpHAnimHierarchySetCurrentAnim(m_pHierarchy, 0);
    RpHAnimHierarchyUpdateMatrices(m_pHierarchy);
    RpSkinSafeClumpSetHAnimHierarchy(pClump, m_pHierarchy);
  }
}

RwInt32 CAnimController::AddAnimation(RwInt32 animId) {
  if (!m_pHierarchy)
    return -1;
  RpHAnimAnimation *pAnim = (RpHAnimAnimation *)animId;
  if (pAnim) {
    AnimEntry entry;
    entry.pAnim = pAnim;
    entry.speed = 1.0f;
    entry.flags = 0;
    entry.id = animId;
    m_anims.push_back(entry);
    return static_cast<RwInt32>(m_anims.size() - 1);
  }
  return -1;
}

RpHAnimAnimation *CAnimController::GetAnimation(RwInt32 index) {
  RWS_ASSERT(HasTarget(), "Failed PRE condition (HasTarget())");
  if (index >= 0 && index < static_cast<RwInt32>(m_anims.size())) {
    return m_anims[index].pAnim;
  }
  return 0;
}

RpHAnimAnimation *CAnimController::FindAnimation(RwInt32 animId) {
  RWS_ASSERT(HasTarget(), "Failed PRE condition (HasTarget())");
  for (size_t i = 0; i < m_anims.size(); ++i) {
    if (m_anims[i].id == static_cast<RwUInt32>(animId)) {
      return m_anims[i].pAnim;
    }
  }
  return 0;
}

void CAnimController::HandleAttributes(const CAttributePacket &packet) {
  CAttributeCommandIterator it(packet, RWS_CLASS_INDEX(CSystemCommands));
  while (!it.IsFinished()) {
    switch (it->GetCommandId()) {
    case 0:
      AddAnimResource(reinterpret_cast<const RwChar *>(it->GetCommandData()));
      break;
    default:
      break;
    }
    ++it;
  }
}

void CAnimController::AddAnimResource(const RwChar *name) {
  RwUInt32 size = 0;
  const void *pData = CResourceManager::Find(name, 0, &size);
  if (pData) {
    RpHAnimAnimation *pAnim = (RpHAnimAnimation *)pData;
    AnimEntry entry;
    entry.pAnim = pAnim;
    entry.speed = 1.0f;
    entry.flags = 0;
    entry.id = size;
    m_anims.push_back(entry);
  }
}

void CAnimController::SetAnimSpeed(RwInt32 index, RwReal speed) {
  RWS_ASSERT(HasTarget(), "Failed PRE condition (HasTarget())");
  if (index >= 0 && index < static_cast<RwInt32>(m_anims.size())) {
    m_anims[index].speed = speed;
  }
}

void CAnimController::Update(RwReal deltaTime) {
  if (m_pHierarchy && m_currentAnim >= 0 &&
      m_currentAnim < static_cast<RwInt32>(m_anims.size())) {
    RpHAnimHierarchyAddAnimTime(m_pHierarchy,
                                deltaTime * m_anims[m_currentAnim].speed);
    RpHAnimHierarchyUpdateMatrices(m_pHierarchy);
  }
}
} // namespace RWS
