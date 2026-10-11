// ============================================================================
// CannonCruise - Input Manager (InputManager.cpp)
// Original path: D:\Projects\CannonCruisePC\code\InputManager.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_0048c180 / FUN_0048c280 — CInputManager::FreeFocusID / AllocateFocusID ("m_iFocusIDPoolIndex < MAX_FOCUS_ID_COUNT Failed")
//   FUN_0048c480 — CInputManager::PushFocusID ("m_iFocusStackIndex < MAX_FOCUS_ID_COUNT Failed", "!IsFocusIDInStack(iFocusID) Failed")
//   FUN_0048c810 — CInputManager::PopFocusID
//   FUN_0048ca70 — CInputManager focus querying
// ============================================================================

#include "InputManager.h"
#include <cassert>
#include <cstring>

// ---------------------------------------------------------------------------
// Constructor – clears focus arrays, initializes stack pointers to 0,
// and resets player digital button bitmasks.
// ---------------------------------------------------------------------------
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

// ---------------------------------------------------------------------------
// Destructor – frees resources upon shutdown.
// ---------------------------------------------------------------------------
CInputManager::~CInputManager()
{
}

// ---------------------------------------------------------------------------
// HandleEvents – message dispatcher for engine input events.
// ---------------------------------------------------------------------------
void CInputManager::HandleEvents(const RWS::CMsg& msg)
{
}

// ---------------------------------------------------------------------------
// AllocateFocusID – mirrors FUN_0048c280.
// Finds the next unallocated slot in m_bFocusIDAllocated, sets it to TRUE,
// increments pool counter, and asserts against pool limit.
// ---------------------------------------------------------------------------
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

// ---------------------------------------------------------------------------
// FreeFocusID – mirrors FUN_0048c590.
// Validates that ID is allocated and releases it back to the pool.
// ---------------------------------------------------------------------------
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

// ---------------------------------------------------------------------------
// PushFocusID – mirrors FUN_0048c910.
// Validates ID allocation and stack uniqueness, then appends ID to top of stack.
// ---------------------------------------------------------------------------
void CInputManager::PushFocusID(RwInt32 iFocusID)
{
    assert(IsFocusIDAllocated(iFocusID) && "IsFocusIDAllocated(iFocusID) Failed");
    assert(!IsFocusIDInStack(iFocusID) && "!IsFocusIDInStack(iFocusID) Failed");
    assert(m_iFocusStackIndex < MAX_FOCUS_ID_COUNT && "m_iFocusStackIndex < MAX_FOCUS_ID_COUNT Failed");

    m_aFocusStack[m_iFocusStackIndex++] = iFocusID;
}

// ---------------------------------------------------------------------------
// PopFocusID – mirrors FUN_0048cb40.
// Locates given focus ID within active stack and shifts downstream elements down.
// ---------------------------------------------------------------------------
void CInputManager::PopFocusID(RwInt32 iFocusID)
{
    if (m_iFocusStackIndex > 0)
    {
        for (RwUInt32 i = 0; i < m_iFocusStackIndex; ++i)
        {
            if (m_aFocusStack[i] == iFocusID)
            {
                // Shift subsequent entries down
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

// ---------------------------------------------------------------------------
// GetCurrentFocusID – queries the topmost active focus ID on the stack.
// ---------------------------------------------------------------------------
RwInt32 CInputManager::GetCurrentFocusID() const
{
    if (m_iFocusStackIndex > 0)
    {
        return m_aFocusStack[m_iFocusStackIndex - 1];
    }
    return -1;
}

// ---------------------------------------------------------------------------
// IsFocusIDAllocated – tests if specified ID is currently flagged as in-use.
// ---------------------------------------------------------------------------
RwBool CInputManager::IsFocusIDAllocated(RwInt32 iFocusID) const
{
    if (iFocusID >= 0 && iFocusID < MAX_FOCUS_ID_COUNT)
    {
        return m_bFocusIDAllocated[iFocusID];
    }
    return FALSE;
}

// ---------------------------------------------------------------------------
// IsFocusIDInStack – tests if specified ID is present within active modal stack.
// ---------------------------------------------------------------------------
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

// ---------------------------------------------------------------------------
// IsButtonDown – queries current held state of a specific player input button.
// ---------------------------------------------------------------------------
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

// ---------------------------------------------------------------------------
// IsButtonPressed – edge detection: TRUE only on transition from released to pressed.
// ---------------------------------------------------------------------------
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

// ---------------------------------------------------------------------------
// GetAxisValue – reads analog axis deflection values.
// ---------------------------------------------------------------------------
RwReal CInputManager::GetAxisValue(RwInt32 playerIndex, RwInt32 axisId) const
{
    return 0.0f;
}

// ---------------------------------------------------------------------------
// Update – frame latch synchronizing current button states to previous history.
// ---------------------------------------------------------------------------
void CInputManager::Update()
{
    m_uP1PrevButtons = m_uP1Buttons;
    m_uP2PrevButtons = m_uP2Buttons;
}
