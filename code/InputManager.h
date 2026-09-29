#ifndef INPUTMANAGER_H
#define INPUTMANAGER_H

// ============================================================================
// CannonCruise - Input Manager (InputManager.h)
// Original path: D:\Projects\CannonCruisePC\code\InputManager.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include "Singleton.h"
#include "framework/core/eventhandler/ceventhandler.h"
#include <rwcore.h>

#define MAX_FOCUS_ID_COUNT 16

enum EPlayerInput
{
    // Oyuncu 1 Girişleri
    P1_INPUT_1 = 0,
    P1_INPUT_2,
    P1_INPUT_3,
    P1_INPUT_4,
    P1_INPUT_5,
    P1_INPUT_6,
    P1_INPUT_7,
    P1_INPUT_8,
    P1_INPUT_9,
    P1_INPUT_10,
    P1_INPUT_11,
    P1_INPUT_12,
    P1_INPUT_13,
    P1_INPUT_14,
    P1_INPUT_COUNT,

    // Oyuncu 2 Girişleri
    P2_INPUT_1 = 0,
    P2_INPUT_2,
    P2_INPUT_3,
    P2_INPUT_4,
    P2_INPUT_5,
    P2_INPUT_6,
    P2_INPUT_COUNT
};

class CInputManager : public CSingleton<CInputManager>, public RWS::CEventHandler
{
public:
    CInputManager();
    virtual ~CInputManager();

    virtual void HandleEvents(const RWS::CMsg& msg);

    // Odak (Focus ID) Yönetimi
    RwInt32 AllocateFocusID();
    void FreeFocusID(RwInt32 iFocusID);
    void PushFocusID(RwInt32 iFocusID);
    void PopFocusID(RwInt32 iFocusID);
    RwInt32 GetCurrentFocusID() const;

    RwBool IsFocusIDAllocated(RwInt32 iFocusID) const;
    RwBool IsFocusIDInStack(RwInt32 iFocusID) const;

    // Tuş & Giriş Durumları
    RwBool IsButtonDown(RwInt32 playerIndex, RwInt32 inputId) const;
    RwBool IsButtonPressed(RwInt32 playerIndex, RwInt32 inputId) const;
    RwReal GetAxisValue(RwInt32 playerIndex, RwInt32 axisId) const;

    void Update();

private:
    RwBool m_bFocusIDAllocated[MAX_FOCUS_ID_COUNT];
    RwInt32 m_aFocusStack[MAX_FOCUS_ID_COUNT];
    RwUInt32 m_iFocusStackIndex;
    RwUInt32 m_iFocusIDPoolIndex;

    RwUInt32 m_uP1Buttons;
    RwUInt32 m_uP1PrevButtons;
    RwUInt32 m_uP2Buttons;
    RwUInt32 m_uP2PrevButtons;
};

#endif // INPUTMANAGER_H
