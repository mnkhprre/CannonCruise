#ifndef MENUSETFRAMEDELEGATE_H
#define MENUSETFRAMEDELEGATE_H

// ============================================================================
// CannonCruise - Menu Set Frame Delegate (MenuSetFrameDelegate.h)
// Original path: D:\Projects\CannonCruisePC\code\MenuSetFrameDelegate.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include "MenuDelegate.h"

class CMenuSetFrameDelegate : public CMenuDelegate
{
public:
    CMenuSetFrameDelegate(CMenuBehavior* pMenu = 0);
    virtual ~CMenuSetFrameDelegate();

    virtual void Execute();
    void SetFrame(RwInt32 iFrame);

private:
    RwInt32 m_iFrame;
};

#endif // MENUSETFRAMEDELEGATE_H
