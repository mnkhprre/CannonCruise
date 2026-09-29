#ifndef MENUGETTIMEDELEGATE_H
#define MENUGETTIMEDELEGATE_H

// ============================================================================
// CannonCruise - Menu Get Time Delegate (MenuGetTimeDelegate.h)
// Original path: D:\Projects\CannonCruisePC\code\MenuGetTimeDelegate.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include "MenuDelegate.h"

class CMenuGetTimeDelegate : public CMenuDelegate
{
public:
    CMenuGetTimeDelegate(CMenuBehavior* pMenu = 0);
    virtual ~CMenuGetTimeDelegate();

    virtual void Execute();
    RwReal GetTime() const;

private:
    void* m_pAnim;
};

#endif // MENUGETTIMEDELEGATE_H
