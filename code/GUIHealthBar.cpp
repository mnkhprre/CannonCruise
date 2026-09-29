// ============================================================================
// CannonCruise - GUI Health Bar (GUIHealthBar.cpp)
// Original path: D:\Projects\CannonCruisePC\code\GUIHealthBar.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_00483ca0 — CGUIHealthBar constructor & registration
//   FUN_00484030 — CGUIHealthBar::SetDimensions
//   FUN_00484500 — CGUIHealthBar::SetHealth
//   FUN_004849e0 — CGUIHealthBar::Render
// ============================================================================

#include "GUIHealthBar.h"
#include "framework/core/attributehandler/cclassfactory.h"
#include <cassert>

RWS_REGISTER_CLASS(CGUIHealthBar, []() -> void* { return new CGUIHealthBar(); }, sizeof(CGUIHealthBar));

CGUIHealthBar::CGUIHealthBar()
    : m_fHealth(1.0f),
      m_fX(0.05f),
      m_fY(0.05f),
      m_fWidth(0.2f),
      m_fHeight(0.03f),
      m_pTexture(0)
{
}

CGUIHealthBar::~CGUIHealthBar()
{
}

void CGUIHealthBar::HandleEvents(const RWS::CMsg& msg)
{
    // MSG_UPDATE_PLAYER_HEALTH
    if (msg.m_pData)
    {
        const RwReal* pNewHealth = (const RwReal*)msg.m_pData;
        SetHealth(*pNewHealth);
    }
}

void CGUIHealthBar::SetHealth(RwReal fHealth)
{
    assert(fHealth >= 0.0f && "*pNewHealth >= 0.0f Failed");
    assert(fHealth <= 1.0f && "*pNewHealth <= 1.0f Failed");

    m_fHealth = fHealth;
}

void CGUIHealthBar::SetDimensions(RwReal fX, RwReal fY, RwReal fWidth, RwReal fHeight)
{
    assert(fX >= 0.0f && "fX >= 0.0f Failed");
    assert(fY >= 0.0f && "fY >= 0.0f Failed");
    assert(fWidth <= 1.0f && "fWidth <= 1.0f Failed");
    assert(fHeight <= 1.0f && "fHeight <= 1.0f Failed");

    m_fX = fX;
    m_fY = fY;
    m_fWidth = fWidth;
    m_fHeight = fHeight;
}

void CGUIHealthBar::Render()
{
    // 2D Can Barı Çizimi (RwIm2DVertex)
}
