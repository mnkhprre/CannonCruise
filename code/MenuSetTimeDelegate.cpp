// ============================================================================
// CannonCruise - Menu Set Time Delegate (MenuSetTimeDelegate.cpp)
// Original path: D:\Projects\CannonCruisePC\code\MenuSetTimeDelegate.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_00446220 — CMenuSetTimeDelegate constructor
//   FUN_004465d0 — CMenuSetTimeDelegate::SetTime
// ============================================================================

#include "MenuSetTimeDelegate.h"
#include <cassert>

CMenuSetTimeDelegate::CMenuSetTimeDelegate(CMenuBehavior* pMenu)
    : CMenuDelegate(pMenu),
      m_fTime(0.0f)
{
}

CMenuSetTimeDelegate::~CMenuSetTimeDelegate()
{
}

void CMenuSetTimeDelegate::Execute()
{
}

void CMenuSetTimeDelegate::SetTime(RwReal fTime)
{
    assert(fTime >= 0.0f && "fNewTime >= 0.0f Failed");
    assert(fTime <= 1.0f && "fNewTime <= 1.0f Failed");

    m_fTime = fTime;
}
