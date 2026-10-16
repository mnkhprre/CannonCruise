// ============================================================================
// CannonCruise - Player Ship Input Proxy (ShipPlayerInputProxy.cpp)
// Original path: D:\Projects\CannonCruisePC\code\ShipPlayerInputProxy.cpp
// Tersine mÃ¼hendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazÄ±lmÄ±ÅŸtÄ±r.
//
// Binary FonksiyonlarÄ±:
//   FUN_004ae6c0 â€” CShipPlayer registration & setup
//   FUN_004b2fe0 â€” CShipPlayerInputProxy::Update
// ============================================================================

#include "ShipPlayerInputProxy.h"
#include "InputManager.h"
#include "framework/core/attributehandler/cclassfactory.h"
#include <cassert>

// ---------------------------------------------------------------------------
// Constructor â€“ binds proxy to optional ship and sets player index to 0.
// ---------------------------------------------------------------------------
CShipPlayerInputProxy::CShipPlayerInputProxy(CShip *pShip)
    : m_pShip(pShip), m_iPlayerIndex(0) {}

// ---------------------------------------------------------------------------
// Destructor.
// ---------------------------------------------------------------------------
CShipPlayerInputProxy::~CShipPlayerInputProxy() {}

// ---------------------------------------------------------------------------
// AttachShip â€“ associates a new CShip instance with this input controller.
// ---------------------------------------------------------------------------
void CShipPlayerInputProxy::AttachShip(CShip *pShip) { m_pShip = pShip; }

// ---------------------------------------------------------------------------
// HandleEvents â€“ listens for gameplay messages such as iMsgTeleportShip.
// ---------------------------------------------------------------------------
void CShipPlayerInputProxy::HandleEvents(const RWS::CMsg &msg) {
  // iMsgTeleportShip
}

// ---------------------------------------------------------------------------
// Update â€“ mirrors FUN_004b2fe0.
// Samples gamepad and keyboard buttons via CInputManager:
//   - Directional rudder: P1_INPUT_1 (port turn) vs P1_INPUT_2 (starboard turn)
//   - Cannon broadsides: P1_INPUT_4 (port volley) and P1_INPUT_5 (starboard volley)
// ---------------------------------------------------------------------------
void CShipPlayerInputProxy::Update() {
  if (!m_pShip)
    return;

  CInputManager *pInput = CInputManager::GetSingletonPtr();
  if (!pInput)
    return;

  // Rudder steering control
  if (pInput->IsButtonDown(m_iPlayerIndex, P1_INPUT_1)) // Left turn
  {
    m_pShip->SetRudder(-1.0f);
  } else if (pInput->IsButtonDown(m_iPlayerIndex, P1_INPUT_2)) // Right turn
  {
    m_pShip->SetRudder(1.0f);
  } else {
    m_pShip->SetRudder(0.0f);
  }

  // Broadside cannon battery salvoes
  if (pInput->IsButtonPressed(m_iPlayerIndex, P1_INPUT_4)) {
    m_pShip->FirePortCannons();
  }
  if (pInput->IsButtonPressed(m_iPlayerIndex, P1_INPUT_5)) {
    m_pShip->FireStarboardCannons();
  }
}
