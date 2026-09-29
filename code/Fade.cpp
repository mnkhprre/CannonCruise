// ============================================================================
// CannonCruise - Screen Fade Effect (Fade.cpp)
// Original path: D:\Projects\CannonCruisePC\code\Fade.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_00479e90 — CFade constructor & registration
//   FUN_0047a530 — CFade::Update
//   FUN_0047a950 — CFade::Render
// ============================================================================

#include "Fade.h"
#include "InputManager.h"
#include "framework/core/attributehandler/cclassfactory.h"
#include <cassert>

RWS_REGISTER_CLASS(CFade, []() -> void* { return new CFade(); }, sizeof(CFade));

CFade::CFade()
    : m_eState(FADE_IDLE),
      m_fDuration(1.0f),
      m_fCurrentTime(0.0f),
      m_iFocusID(-1)
{
    m_Color.red = 0;
    m_Color.green = 0;
    m_Color.blue = 0;
    m_Color.alpha = 0;
}

CFade::~CFade()
{
    if (m_iFocusID != -1)
    {
        CInputManager* pInputMgr = CInputManager::GetSingletonPtr();
        if (pInputMgr)
        {
            pInputMgr->FreeFocusID(m_iFocusID);
        }
        m_iFocusID = -1;
    }
}

void CFade::HandleEvents(const RWS::CMsg& msg)
{
    // MSG_FADE_RENDER olayı
}

void CFade::StartFade(EFadeState state, RwReal duration, RwRGBA color)
{
    m_eState = state;
    m_fDuration = (duration > 0.0f) ? duration : 1.0f;
    m_fCurrentTime = 0.0f;
    m_Color = color;

    if (m_eState == FADE_OUT)
    {
        m_Color.alpha = 0;
    }
    else if (m_eState == FADE_IN)
    {
        m_Color.alpha = 255;
    }
}

void CFade::Update(RwReal deltaTime)
{
    if (m_eState == FADE_IDLE) return;

    m_fCurrentTime += deltaTime;
    RwReal t = m_fCurrentTime / m_fDuration;
    if (t > 1.0f) t = 1.0f;

    switch (m_eState)
    {
        case FADE_IN:
            m_Color.alpha = (RwUInt8)((1.0f - t) * 255.0f);
            if (t >= 1.0f) m_eState = FADE_IDLE;
            break;

        case FADE_OUT:
            m_Color.alpha = (RwUInt8)(t * 255.0f);
            if (t >= 1.0f) m_eState = FADE_HOLD;
            break;

        case FADE_HOLD:
            m_Color.alpha = 255;
            break;

        default:
            assert(false && "Error, unknown state in CFade");
            break;
    }
}

void CFade::Render()
{
    if (m_eState == FADE_IDLE || m_Color.alpha == 0) return;

    // 2D Dörtgen RenderWare Im2D ile ekranı kapla
}
