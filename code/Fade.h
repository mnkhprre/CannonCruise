#ifndef FADE_H
#define FADE_H

// ============================================================================
// CannonCruise - Screen Fade Effect (Fade.h)
// Original path: D:\Projects\CannonCruisePC\code\Fade.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include "framework/core/eventhandler/ceventhandler.h"
#include <rwcore.h>

class CFade : public RWS::CEventHandler
{
public:
    enum EFadeState
    {
        FADE_IDLE = 0,
        FADE_IN,
        FADE_OUT,
        FADE_HOLD
    };

    CFade();
    virtual ~CFade();

    virtual void HandleEvents(const RWS::CMsg& msg);

    void StartFade(EFadeState state, RwReal duration, RwRGBA color);
    void Render();
    void Update(RwReal deltaTime);

    RwBool IsFading() const { return m_eState != FADE_IDLE; }

private:
    EFadeState m_eState;
    RwReal m_fDuration;
    RwReal m_fCurrentTime;
    RwRGBA m_Color;
    RwInt32 m_iFocusID;
};

#endif // FADE_H
