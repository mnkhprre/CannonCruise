#ifndef GUIHEALTHBAR_H
#define GUIHEALTHBAR_H

// ============================================================================
// CannonCruise - GUI Health Bar (GUIHealthBar.h)
// Original path: D:\Projects\CannonCruisePC\code\GUIHealthBar.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include "framework/core/eventhandler/ceventhandler.h"
#include <rwcore.h>

class CGUIHealthBar : public RWS::CEventHandler
{
public:
    CGUIHealthBar();
    virtual ~CGUIHealthBar();

    virtual void HandleEvents(const RWS::CMsg& msg);

    void SetHealth(RwReal fHealth);
    void SetDimensions(RwReal fX, RwReal fY, RwReal fWidth, RwReal fHeight);
    void Render();

private:
    RwReal m_fHealth;
    RwReal m_fX;
    RwReal m_fY;
    RwReal m_fWidth;
    RwReal m_fHeight;
    RwTexture* m_pTexture;
};

#endif // GUIHEALTHBAR_H
