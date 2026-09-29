#ifndef MENUSETTIMEDELEGATE_H
#define MENUSETTIMEDELEGATE_H

// ============================================================================
// CannonCruise - Menu Set Time Delegate (MenuSetTimeDelegate.h)
// Original path: D:\Projects\CannonCruisePC\code\MenuSetTimeDelegate.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include "MenuDelegate.h"

class CMenuSetTimeDelegate : public CMenuDelegate
{
public:
    CMenuSetTimeDelegate(CMenuBehavior* pMenu = 0);
    virtual ~CMenuSetTimeDelegate();

    virtual void Execute();
    void SetTime(RwReal fTime);

private:
    RwReal m_fTime;
};

#endif // MENUSETTIMEDELEGATE_H
