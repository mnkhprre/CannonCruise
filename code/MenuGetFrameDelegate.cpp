// ============================================================================
// CannonCruise - Menu Get Frame Delegate (MenuGetFrameDelegate.cpp)
// Original path: D:\Projects\CannonCruisePC\code\MenuGetFrameDelegate.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_00445100 — CMenuGetFrameDelegate constructor
//   FUN_00445270 — CMenuGetFrameDelegate::Execute
// ============================================================================

#include "MenuGetFrameDelegate.h"
#include <cassert>

CMenuGetFrameDelegate::CMenuGetFrameDelegate(CMenuBehavior* pMenu)
    : CMenuDelegate(pMenu),
      m_pAnim(0)
{
}

CMenuGetFrameDelegate::~CMenuGetFrameDelegate()
{
}

void CMenuGetFrameDelegate::Execute()
{
    // m_pAnim != NULL Failed kontrolü
}

RwInt32 CMenuGetFrameDelegate::GetFrame() const
{
    return 0;
}
