// ============================================================================
// CannonCruise - Show Text Box Entity (ShowText.cpp)
// Original path: D:\Projects\CannonCruisePC\code\ShowText.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_004b38d0 — CShowText constructor & registration
//   FUN_004b4270 — CShowText::Show
//   FUN_004b4dcf — CShowText::Render
// ============================================================================

#include "ShowText.h"
#include "framework/core/attributehandler/cclassfactory.h"
#include <cassert>

RWS_REGISTER_CLASS(CShowText, []() -> void* { return new CShowText(); }, sizeof(CShowText));

CShowText::CShowText()
    : m_fDuration(3.0f),
      m_fTimer(0.0f),
      m_bVisible(FALSE)
{
}

CShowText::~CShowText()
{
}

void CShowText::HandleEvents(const RWS::CMsg& msg)
{
    // MSG_SHOWTEXT_RENDER
}

void CShowText::Show(const char* pText, RwReal duration)
{
    if (pText)
    {
        m_Text = pText;
        m_fDuration = duration;
        m_fTimer = 0.0f;
        m_bVisible = TRUE;
    }
}

void CShowText::Hide()
{
    m_bVisible = FALSE;
}

void CShowText::Render()
{
    if (!m_bVisible) return;

    // 2D Metin Kutusu Çizimi
}
