// ============================================================================
// CannonCruise - Menu Delegate Base (MenuDelegate.cpp)
// Original path: D:\Projects\CannonCruisePC\code\MenuDelegate.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_00444c80 — CMenuDelegate constructor & initialization
// ============================================================================

#include "MenuDelegate.h"
#include <cassert>

CMenuDelegate::CMenuDelegate(CMenuBehavior* pMenu)
    : m_pMenu(pMenu),
      m_pAnimProps(0)
{
}

CMenuDelegate::~CMenuDelegate()
{
}
