// ============================================================================
// CannonCruise - Ship Base Class (Ship.cpp)
// Original path: D:\Projects\CannonCruisePC\code\Ship.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_004a7430 — CShip constructor & initialization
//   FUN_004a76d0 — CShip::Update
//   FUN_004a7bc0 — CShip::TakeDamage
// ============================================================================

#include "Ship.h"
#include "AudioManager.h"
#include "framework/toolkits/world/helpers/framehelper.h"
#include <cassert>

CShip::CShip()
    : m_pClump(0), m_pRudderFrame(0), m_pWheelFrame(0), m_fHealth(100.0f),
      m_fMaxHealth(100.0f), m_fCurrentSpeed(0.0f), m_fTargetSpeed(0.0f),
      m_fRudderAngle(0.0f), m_bSinking(FALSE) {}

CShip::~CShip() {
  if (m_pClump) {
    RpClumpDestroy(m_pClump);
    m_pClump = 0;
  }
}

void CShip::HandleEvents(const RWS::CMsg &msg) {
  // CShip HE m_PhysicsWorldUpdateEvent, CShip HE iMsgRunningTick
}

void CShip::Initialize(RpClump *pClump) {
  assert(m_pClump == 0 && "Ship already has a clump");
  m_pClump = pClump;

  if (m_pClump) {
    RwFrame *pRoot = RpClumpGetFrame(m_pClump);
    m_pRudderFrame = RWS::FrameHelper::FindFrameByName(pRoot, "Rudder");
    m_pWheelFrame = RWS::FrameHelper::FindFrameByName(pRoot, "Wheel");
  }
}

void CShip::SetThrottle(RwReal fThrottle) {
  m_fTargetSpeed = fThrottle * 25.0f;
}

void CShip::SetRudder(RwReal fRudder) { m_fRudderAngle = fRudder; }

void CShip::FirePortCannons(ECannonballType type) {
  RwV3d leftDir = {-1.0f, 0.2f, 0.0f};
  m_PortCannons.Fire(type, &leftDir);
}

void CShip::FireStarboardCannons(ECannonballType type) {
  RwV3d rightDir = {1.0f, 0.2f, 0.0f};
  m_StarboardCannons.Fire(type, &rightDir);
}

void CShip::TakeDamage(RwReal damage) {
  m_fHealth -= damage;
  if (m_fHealth <= 0.0f && !m_bSinking) {
    Sink();
  }
}

void CShip::Sink() {
  m_bSinking = TRUE;
  CAudioManager *pAudio = CAudioManager::GetSingletonPtr();
  if (pAudio) {
    pAudio->PlaySound("Ship_Sink");
  }
}

void CShip::Update(RwReal deltaTime) {
  if (m_bSinking)
    return;

  // Hız ve rotasyon interpolasyonu
  m_fCurrentSpeed += (m_fTargetSpeed - m_fCurrentSpeed) * deltaTime * 2.0f;
}
