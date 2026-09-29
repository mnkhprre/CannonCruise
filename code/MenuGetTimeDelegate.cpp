// ============================================================================
// CannonCruise - Menu Get Time Delegate (MenuGetTimeDelegate.cpp)
// Original path: D:\Projects\CannonCruisePC\code\MenuGetTimeDelegate.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_004455e0 — CMenuGetTimeDelegate constructor
//   FUN_00445750 — CMenuGetTimeDelegate::Execute
// ============================================================================

#include "MenuGetTimeDelegate.h"
#include <cassert>

CMenuGetTimeDelegate::CMenuGetTimeDelegate(CMenuBehavior* pMenu)
    : CMenuDelegate(pMenu),
      m_pAnim(0)
{
}

CMenuGetTimeDelegate::~CMenuGetTimeDelegate()
{
}

void CMenuGetTimeDelegate::Execute()
{
}

RwReal CMenuGetTimeDelegate::GetTime() const
{
    return 0.0f;
}
