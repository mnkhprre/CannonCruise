// ============================================================================
// CannonCruise - Player Ship Input Proxy (ShipPlayerInputProxy.cpp)
// Original path: D:\Projects\CannonCruisePC\code\ShipPlayerInputProxy.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_004ae6c0 — CShipPlayer registration
//   FUN_004b2fe0 — CShipPlayerInputProxy::Update
// ============================================================================

#include "ShipPlayerInputProxy.h"
#include "InputManager.h"
#include "framework/core/attributehandler/cclassfactory.h"
#include <cassert>

CShipPlayerInputProxy::CShipPlayerInputProxy(CShip *pShip)
    : m_pShip(pShip), m_iPlayerIndex(0) {}

CShipPlayerInputProxy::~CShipPlayerInputProxy() {}

void CShipPlayerInputProxy::AttachShip(CShip *pShip) { m_pShip = pShip; }

void CShipPlayerInputProxy::HandleEvents(const RWS::CMsg &msg) {
  // iMsgTeleportShip
}

void CShipPlayerInputProxy::Update() {
  if (!m_pShip)
    return;

  CInputManager *pInput = CInputManager::GetSingletonPtr();
  if (!pInput)
    return;

  // Dümen kontrolü
  if (pInput->IsButtonDown(m_iPlayerIndex, P1_INPUT_1)) // Sol
  {
    m_pShip->SetRudder(-1.0f);
  } else if (pInput->IsButtonDown(m_iPlayerIndex, P1_INPUT_2)) // Sağ
  {
    m_pShip->SetRudder(1.0f);
  } else {
    m_pShip->SetRudder(0.0f);
  }

  // İskele (sol) ve Sancak (sağ) top atışları
  if (pInput->IsButtonPressed(m_iPlayerIndex, P1_INPUT_4)) {
    m_pShip->FirePortCannons();
  }
  if (pInput->IsButtonPressed(m_iPlayerIndex, P1_INPUT_5)) {
    m_pShip->FireStarboardCannons();
  }
}
