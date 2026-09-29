#ifndef MENUGETFRAMEDELEGATE_H
#define MENUGETFRAMEDELEGATE_H

// ============================================================================
// CannonCruise - Menu Get Frame Delegate (MenuGetFrameDelegate.h)
// Original path: D:\Projects\CannonCruisePC\code\MenuGetFrameDelegate.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include "MenuDelegate.h"

class CMenuGetFrameDelegate : public CMenuDelegate
{
public:
    CMenuGetFrameDelegate(CMenuBehavior* pMenu = 0);
    virtual ~CMenuGetFrameDelegate();

    virtual void Execute();
    RwInt32 GetFrame() const;

private:
    void* m_pAnim;
};

#endif // MENUGETFRAMEDELEGATE_H
