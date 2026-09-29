// ============================================================================
// CannonCruise - Input Manager (InputManager.cpp)
// Original path: D:\Projects\CannonCruisePC\code\InputManager.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_0048c280 — CInputManager::AllocateFocusID
//   FUN_0048c590 — CInputManager::FreeFocusID
//   FUN_0048c910 — CInputManager::PushFocusID
//   FUN_0048cb40 — CInputManager::PopFocusID
// ============================================================================

#include "InputManager.h"
#include <cassert>
#include <cstring>

CInputManager::CInputManager()
    : m_iFocusStackIndex(0),
      m_iFocusIDPoolIndex(0),
      m_uP1Buttons(0),
      m_uP1PrevButtons(0),
      m_uP2Buttons(0),
      m_uP2PrevButtons(0)
{
    memset(m_bFocusIDAllocated, 0, sizeof(m_bFocusIDAllocated));
    memset(m_aFocusStack, -1, sizeof(m_aFocusStack));
}

CInputManager::~CInputManager()
{
}

void CInputManager::HandleEvents(const RWS::CMsg& msg)
{
}

// ============================================================================
// CInputManager::AllocateFocusID (FUN_0048c280)
// ASM: 0048c280 — Yeni bir odak ID'si tahsis et
// ============================================================================
RwInt32 CInputManager::AllocateFocusID()
{
    for (RwUInt32 i = 0; i < MAX_FOCUS_ID_COUNT; ++i)
    {
        if (!m_bFocusIDAllocated[i])
        {
            m_bFocusIDAllocated[i] = TRUE;
            m_iFocusIDPoolIndex++;
            assert(m_iFocusIDPoolIndex < MAX_FOCUS_ID_COUNT && "m_iFocusIDPoolIndex < MAX_FOCUS_ID_COUNT Failed");
            return (RwInt32)i;
        }
    }
    return -1;
}

// ============================================================================
// CInputManager::FreeFocusID (FUN_0048c590)
// ASM: 0048c590 — Odak ID'sini serbest bırak
// ============================================================================
void CInputManager::FreeFocusID(RwInt32 iFocusID)
{
    if (iFocusID >= 0 && iFocusID < MAX_FOCUS_ID_COUNT)
    {
        assert(IsFocusIDAllocated(iFocusID) && "IsFocusIDAllocated(iFocusID) Failed");
        m_bFocusIDAllocated[iFocusID] = FALSE;
        if (m_iFocusIDPoolIndex > 0)
        {
            m_iFocusIDPoolIndex--;
        }
    }
}

// ============================================================================
// CInputManager::PushFocusID (FUN_0048c910)
// ASM: 0048c910 — Odak yığıtına ID ekle
// ============================================================================
void CInputManager::PushFocusID(RwInt32 iFocusID)
{
    assert(IsFocusIDAllocated(iFocusID) && "IsFocusIDAllocated(iFocusID) Failed");
    assert(!IsFocusIDInStack(iFocusID) && "!IsFocusIDInStack(iFocusID) Failed");
    assert(m_iFocusStackIndex < MAX_FOCUS_ID_COUNT && "m_iFocusStackIndex < MAX_FOCUS_ID_COUNT Failed");

    m_aFocusStack[m_iFocusStackIndex++] = iFocusID;
}

// ============================================================================
// CInputManager::PopFocusID (FUN_0048cb40)
// ASM: 0048cb40 — Odak yığıtından ID çıkar
// ============================================================================
void CInputManager::PopFocusID(RwInt32 iFocusID)
{
    if (m_iFocusStackIndex > 0)
    {
        for (RwUInt32 i = 0; i < m_iFocusStackIndex; ++i)
        {
            if (m_aFocusStack[i] == iFocusID)
            {
                // Kalanları kaydır
                for (RwUInt32 j = i; j < m_iFocusStackIndex - 1; ++j)
                {
                    m_aFocusStack[j] = m_aFocusStack[j + 1];
                }
                m_aFocusStack[--m_iFocusStackIndex] = -1;
                break;
            }
        }
    }
}

RwInt32 CInputManager::GetCurrentFocusID() const
{
    if (m_iFocusStackIndex > 0)
    {
        return m_aFocusStack[m_iFocusStackIndex - 1];
    }
    return -1;
}

RwBool CInputManager::IsFocusIDAllocated(RwInt32 iFocusID) const
{
    if (iFocusID >= 0 && iFocusID < MAX_FOCUS_ID_COUNT)
    {
        return m_bFocusIDAllocated[iFocusID];
    }
    return FALSE;
}

RwBool CInputManager::IsFocusIDInStack(RwInt32 iFocusID) const
{
    for (RwUInt32 i = 0; i < m_iFocusStackIndex; ++i)
    {
        if (m_aFocusStack[i] == iFocusID)
        {
            return TRUE;
        }
    }
    return FALSE;
}

RwBool CInputManager::IsButtonDown(RwInt32 playerIndex, RwInt32 inputId) const
{
    if (playerIndex == 0)
    {
        return (m_uP1Buttons & (1 << inputId)) != 0;
    }
    else if (playerIndex == 1)
    {
        return (m_uP2Buttons & (1 << inputId)) != 0;
    }
    return FALSE;
}

RwBool CInputManager::IsButtonPressed(RwInt32 playerIndex, RwInt32 inputId) const
{
    if (playerIndex == 0)
    {
        return ((m_uP1Buttons & (1 << inputId)) != 0) && ((m_uP1PrevButtons & (1 << inputId)) == 0);
    }
    else if (playerIndex == 1)
    {
        return ((m_uP2Buttons & (1 << inputId)) != 0) && ((m_uP2PrevButtons & (1 << inputId)) == 0);
    }
    return FALSE;
}

RwReal CInputManager::GetAxisValue(RwInt32 playerIndex, RwInt32 axisId) const
{
    return 0.0f;
}

void CInputManager::Update()
{
    m_uP1PrevButtons = m_uP1Buttons;
    m_uP2PrevButtons = m_uP2Buttons;
}
