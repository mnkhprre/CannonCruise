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

CMenuSetFrameDelegate::CMenuSetFrameDelegate(CMenuBehavior* pMenu)
    : CMenuDelegate(pMenu),
      m_iFrame(0)
{
}

CMenuSetFrameDelegate::~CMenuSetFrameDelegate()
{
}

void CMenuSetFrameDelegate::Execute()
{
}

void CMenuSetFrameDelegate::SetFrame(RwInt32 iFrame)
{
    assert(iFrame >= 0 && "iAnimPos >= 0 Failed");
    m_iFrame = iFrame;
}
