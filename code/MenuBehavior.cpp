// ============================================================================
// CannonCruise - 2D Maestro Menu Behavior (MenuBehavior.cpp)
// Original path: D:\Projects\CannonCruisePC\code\MenuBehavior.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_00443050 — CMenuBehavior constructor & registration
//   FUN_00443800 — CMenuBehavior::Update
//   FUN_00443c20 — CMenuBehavior::Render
// ============================================================================

#include "MenuBehavior.h"
#include "InputManager.h"
#include "framework/core/attributehandler/cclassfactory.h"
#include <cassert>

RWS_REGISTER_CLASS(CMenuBehavior, []() -> void* { return new CMenuBehavior(); }, sizeof(CMenuBehavior));

CMenuBehavior::CMenuBehavior()
    : m_pMaestro(0),
      m_iInputFocusID(-1)
{
    CInputManager* pInputMgr = CInputManager::GetSingletonPtr();
    if (pInputMgr)
    {
        m_iInputFocusID = pInputMgr->AllocateFocusID();
    }
}

CMenuBehavior::~CMenuBehavior()
{
    if (m_iInputFocusID != -1)
    {
        CInputManager* pInputMgr = CInputManager::GetSingletonPtr();
        if (pInputMgr)
        {
            pInputMgr->FreeFocusID(m_iInputFocusID);
        }
        m_iInputFocusID = -1;
    }

    if (m_pMaestro)
    {
        Rt2dMaestroDestroy(m_pMaestro);
        m_pMaestro = 0;
    }
}

void CMenuBehavior::HandleEvents(const RWS::CMsg& msg)
{
    // MSG_MENU_RENDER_FRONT_RT2D
}

void CMenuBehavior::Update(RwReal deltaTime)
{
    if (m_pMaestro)
    {
        Rt2dMaestroUpdate(m_pMaestro, deltaTime);
    }
}

void CMenuBehavior::Render()
{
    if (m_pMaestro)
    {
        Rt2dMaestroRender(m_pMaestro);
    }
}
