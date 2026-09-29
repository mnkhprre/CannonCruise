#ifndef MENUDELEGATE_H
#define MENUDELEGATE_H

// ============================================================================
// CannonCruise - Menu Delegate Base (MenuDelegate.h)
// Original path: D:\Projects\CannonCruisePC\code\MenuDelegate.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include "framework/core/eventhandler/ceventhandler.h"
#include <rwcore.h>

class CMenuBehavior;

class CMenuDelegate : public RWS::CEventHandler
{
public:
    CMenuDelegate(CMenuBehavior* pMenu = 0);
    virtual ~CMenuDelegate();

    virtual void Execute() = 0;

protected:
    CMenuBehavior* m_pMenu;
    void* m_pAnimProps;
};

#endif // MENUDELEGATE_H
