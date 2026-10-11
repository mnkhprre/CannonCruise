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

/**
 * @brief Constructs a new CMenuDelegate base object.
 *
 * @param pMenu Optional owning menu behavior instance.
 * @see FUN_00444c80
 */
CMenuDelegate::CMenuDelegate(CMenuBehavior* pMenu)
    : m_pMenu(pMenu),
      m_pAnimProps(0)
{
}

/**
 * @brief Virtual destructor for CMenuDelegate.
 */
CMenuDelegate::~CMenuDelegate()
{
}

