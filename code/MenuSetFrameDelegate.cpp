// ============================================================================
// CannonCruise - Menu Set Frame Delegate (MenuSetFrameDelegate.cpp)
// Original path: D:\Projects\CannonCruisePC\code\MenuSetFrameDelegate.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_00445ac0 — CMenuSetFrameDelegate constructor
//   FUN_00445e90 — CMenuSetFrameDelegate::SetFrame
// ============================================================================

#include "MenuSetFrameDelegate.h"
#include <cassert>

/**
 * @brief Constructs a new CMenuSetFrameDelegate instance.
 *
 * @param pMenu Optional owning menu behavior.
 * @see FUN_00445ac0
 */
CMenuSetFrameDelegate::CMenuSetFrameDelegate(CMenuBehavior* pMenu)
    : CMenuDelegate(pMenu),
      m_iFrame(0)
{
}

/**
 * @brief Destructor.
 */
CMenuSetFrameDelegate::~CMenuSetFrameDelegate()
{
}

/**
 * @brief Executes the animation frame seek operation.
 */
void CMenuSetFrameDelegate::Execute()
{
}

/**
 * @brief Sets the target frame index with non-negative validation assertion.
 *
 * @param iFrame Target frame index (>= 0).
 * @see FUN_00445e90
 */
void CMenuSetFrameDelegate::SetFrame(RwInt32 iFrame)
{
    assert(iFrame >= 0 && "iAnimPos >= 0 Failed");
    m_iFrame = iFrame;
}

