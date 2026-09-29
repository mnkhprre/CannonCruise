#ifndef MENUBEHAVIOR_H
#define MENUBEHAVIOR_H

// ============================================================================
// CannonCruise - 2D Maestro Menu Behavior (MenuBehavior.h)
// Original path: D:\Projects\CannonCruisePC\code\MenuBehavior.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include "framework/core/eventhandler/ceventhandler.h"
#include <rwcore.h>
#include <rt2d.h>
#include <rt2danim.h>

class CMenuBehavior : public RWS::CEventHandler
{
public:
    CMenuBehavior();
    virtual ~CMenuBehavior();

    virtual void HandleEvents(const RWS::CMsg& msg);

    void Update(RwReal deltaTime);
    void Render();

    Rt2dMaestro* GetMaestro() const { return m_pMaestro; }

private:
    Rt2dMaestro* m_pMaestro;
    RwInt32 m_iInputFocusID;
};

#endif // MENUBEHAVIOR_H
