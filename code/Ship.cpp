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

// ---------------------------------------------------------------------------
// Constructor – mirrors FUN_004a7430.
// All pointers are NULLed and numeric values set to sensible defaults so the
// object is in a safe state before Initialize() is called.
// ---------------------------------------------------------------------------
CShip::CShip()
    : m_pClump(0), m_pRudderFrame(0), m_pWheelFrame(0), m_fHealth(100.0f),
      m_fMaxHealth(100.0f), m_fCurrentSpeed(0.0f), m_fTargetSpeed(0.0f),
      m_fRudderAngle(0.0f), m_bSinking(FALSE) {}

// ---------------------------------------------------------------------------
// Destructor – releases the RpClump that was handed to us via Initialize().
// The clump is exclusively owned by this ship instance.
// ---------------------------------------------------------------------------
CShip::~CShip() {
  if (m_pClump) {
    RpClumpDestroy(m_pClump);
    m_pClump = 0;
  }
}

// ---------------------------------------------------------------------------
// HandleEvents – message dispatcher called by the RenderWare Studio framework.
// The original binary listens for:
//   • m_PhysicsWorldUpdateEvent – triggers physics-related updates
//   • iMsgRunningTick           – triggers per-frame gameplay logic
// ---------------------------------------------------------------------------
void CShip::HandleEvents(const RWS::CMsg &msg) {
  // CShip HE m_PhysicsWorldUpdateEvent, CShip HE iMsgRunningTick
}

// ---------------------------------------------------------------------------
// Initialize – attaches a loaded 3-D model to this ship.
// After storing the clump pointer the method walks the frame hierarchy to
// find the "Rudder" and "Wheel" child frames. These cached pointers allow
// SetRudder() and Update() to animate the steering components cheaply.
// ---------------------------------------------------------------------------
void CShip::Initialize(RpClump *pClump) {
  assert(m_pClump == 0 && "Ship already has a clump");
  m_pClump = pClump;

  if (m_pClump) {
    RwFrame *pRoot = RpClumpGetFrame(m_pClump);
    m_pRudderFrame = RWS::FrameHelper::FindFrameByName(pRoot, "Rudder");
    m_pWheelFrame = RWS::FrameHelper::FindFrameByName(pRoot, "Wheel");
  }
}

// ---------------------------------------------------------------------------
// SetThrottle – converts a normalised input (typically -1..1) into a target
// speed by multiplying with the hard-coded factor 25.  This mirrors the
// constant found at the original call-site (FUN_004a76d0).
// ---------------------------------------------------------------------------
void CShip::SetThrottle(RwReal fThrottle) {
  m_fTargetSpeed = fThrottle * 25.0f;
}

// ---------------------------------------------------------------------------
// SetRudder – stores the raw rudder deflection. The value is consumed by
// the physics update step to rotate the ship around its vertical axis.
// ---------------------------------------------------------------------------
void CShip::SetRudder(RwReal fRudder) { m_fRudderAngle = fRudder; }

// ---------------------------------------------------------------------------
// FirePortCannons – launches cannonballs from the left side of the ship.
// Direction vector {-1, 0.2, 0}: left and slightly upward to create a
// visible arc trajectory matching the original game's feel.
// ---------------------------------------------------------------------------
void CShip::FirePortCannons(ECannonballType type) {
  RwV3d leftDir = {-1.0f, 0.2f, 0.0f};
  m_PortCannons.Fire(type, &leftDir);
}

// ---------------------------------------------------------------------------
// FireStarboardCannons – mirror of FirePortCannons for the right side.
// Direction vector {1, 0.2, 0}: right and slightly upward.
// ---------------------------------------------------------------------------
void CShip::FireStarboardCannons(ECannonballType type) {
  RwV3d rightDir = {1.0f, 0.2f, 0.0f};
  m_StarboardCannons.Fire(type, &rightDir);
}

// ---------------------------------------------------------------------------
// TakeDamage – subtracts the given amount from current health.
// When health drops to zero (or below) the ship enters the sinking state
// exactly once, reproducing the guard check from FUN_004a7bc0.
// ---------------------------------------------------------------------------
void CShip::TakeDamage(RwReal damage) {
  m_fHealth -= damage;
  if (m_fHealth <= 0.0f && !m_bSinking) {
    Sink();
  }
}

// ---------------------------------------------------------------------------
// Sink – begins the sinking sequence.
// Sets the sinking flag (which disables Update) and plays the "Ship_Sink"
// sound effect through the global AudioManager singleton.
// ---------------------------------------------------------------------------
void CShip::Sink() {
  m_bSinking = TRUE;
  CAudioManager *pAudio = CAudioManager::GetSingletonPtr();
  if (pAudio) {
    pAudio->PlaySound("Ship_Sink");
  }
}

// ---------------------------------------------------------------------------
// Update – called every frame (mirrors FUN_004a76d0).
// Uses simple exponential interpolation (lerp factor = deltaTime * 2) to
// smoothly accelerate or decelerate toward the target speed.
// The multiplication constant 2.0 controls how responsive the ship feels;
// this value is taken directly from the original binary.
// ---------------------------------------------------------------------------
void CShip::Update(RwReal deltaTime) {
  if (m_bSinking)
    return;

  // Hız ve rotasyon interpolasyonu
  m_fCurrentSpeed += (m_fTargetSpeed - m_fCurrentSpeed) * deltaTime * 2.0f;
}

