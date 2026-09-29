#ifndef SHOWTEXT_H
#define SHOWTEXT_H

// ============================================================================
// CannonCruise - Show Text Box Entity (ShowText.h)
// Original path: D:\Projects\CannonCruisePC\code\ShowText.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include "framework/core/eventhandler/ceventhandler.h"
#include <rwcore.h>
#include <string>

class CShowText : public RWS::CEventHandler
{
public:
    CShowText();
    virtual ~CShowText();

    virtual void HandleEvents(const RWS::CMsg& msg);

    void Show(const char* pText, RwReal duration = 3.0f);
    void Hide();
    void Render();

private:
    std::string m_Text;
    RwReal m_fDuration;
    RwReal m_fTimer;
    RwBool m_bVisible;
};

#endif // SHOWTEXT_H
