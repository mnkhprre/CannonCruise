#ifndef INPUTMANAGER_H
#define INPUTMANAGER_H

// ============================================================================
// CannonCruise - Input Manager (InputManager.h)
// Original path: D:\Projects\CannonCruisePC\code\InputManager.h
// Tersine mÃ¼hendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazÄ±lmÄ±ÅŸtÄ±r.
// ============================================================================

#include "Singleton.h"
#include "framework/core/eventhandler/ceventhandler.h"
#include <rwcore.h>

#define MAX_FOCUS_ID_COUNT 16

/**
 * @enum EPlayerInput
 * @brief Logical input button action bitmask IDs for Player 1 and Player 2.
 */
enum EPlayerInput
{
    // Player 1 controller & keyboard actions
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

    // Player 2 actions (split-screen / co-op / versus)
    P2_INPUT_1 = 0,
    P2_INPUT_2,
    P2_INPUT_3,
    P2_INPUT_4,
    P2_INPUT_5,
    P2_INPUT_6,
    P2_INPUT_COUNT
};

/**
 * @class CInputManager
 * @brief Centralized input arbiter managing focus contexts and gamepad/keyboard states.
 *
 * Implements a focus stack (m_aFocusStack) of depth up to MAX_FOCUS_ID_COUNT (16),
 * allowing UI menus, dialogue boxes, cinematics, and gameplay actors to modal-claim
 * or restore input focus cleanly without cross-interference.
 * Also monitors digital button states, edge triggers, and analog joystick axes.
 */
class CInputManager : public CSingleton<CInputManager>, public RWS::CEventHandler
{
public:
    /**
     * @brief Constructs singleton input manager, initializing pools and button states.
     */
    CInputManager();

    /**
     * @brief Destructor.
     */
    virtual ~CInputManager();

    /**
     * @brief Framework message dispatcher.
     * @param msg  Message descriptor.
     */
    virtual void HandleEvents(const RWS::CMsg& msg);

    // ---- Focus Context Management -------------------------------------------

    /**
     * @brief Reserves a unique focus identifier from the pool.
     * @return Allocated focus ID in range [0..MAX_FOCUS_ID_COUNT-1], or -1 if exhausted.
     */
    RwInt32 AllocateFocusID();

    /**
     * @brief Releases a previously allocated focus ID back to the pool.
     * @param iFocusID  Focus identifier to deallocate.
     */
    void FreeFocusID(RwInt32 iFocusID);

    /**
     * @brief Pushes an allocated focus ID onto the top of the modal input stack.
     * @param iFocusID  Focus ID to push.
     */
    void PushFocusID(RwInt32 iFocusID);

    /**
     * @brief Removes a specified focus ID from the input stack, shifting remaining IDs.
     * @param iFocusID  Focus ID to remove.
     */
    void PopFocusID(RwInt32 iFocusID);

    /**
     * @brief Retrieves the topmost active modal focus ID.
     * @return Active focus ID, or -1 if stack is empty.
     */
    RwInt32 GetCurrentFocusID() const;

    /**
     * @brief Checks if a specific focus ID is currently allocated.
     * @param iFocusID  ID to test.
     */
    RwBool IsFocusIDAllocated(RwInt32 iFocusID) const;

    /**
     * @brief Checks if a specific focus ID is currently present in the focus stack.
     * @param iFocusID  ID to test.
     */
    RwBool IsFocusIDInStack(RwInt32 iFocusID) const;

    // ---- Button and Axis Queries --------------------------------------------

    /**
     * @brief Checks if a button is currently held down in the active frame.
     * @param playerIndex  Player index (0 for P1, 1 for P2).
     * @param inputId      Button bit index (EPlayerInput).
     */
    RwBool IsButtonDown(RwInt32 playerIndex, RwInt32 inputId) const;

    /**
     * @brief Checks if a button was pressed down this frame (rising edge trigger).
     * @param playerIndex  Player index (0 for P1, 1 for P2).
     * @param inputId      Button bit index (EPlayerInput).
     */
    RwBool IsButtonPressed(RwInt32 playerIndex, RwInt32 inputId) const;

    /**
     * @brief Reads analog joystick axis deflection value [-1.0 .. +1.0].
     * @param playerIndex  Player index.
     * @param axisId       Axis identifier (0: X, 1: Y).
     */
    RwReal GetAxisValue(RwInt32 playerIndex, RwInt32 axisId) const;

    /**
     * @brief Sets digital button state for a player.
     * @param playerIndex Player index (0=P1, 1=P2).
     * @param inputId Button bit index (EPlayerInput).
     * @param bDown TRUE if held, FALSE if released.
     */
    void SetButtonState(RwInt32 playerIndex, RwInt32 inputId, RwBool bDown);

    /**
     * @brief Directly sets full 32-bit button bitmask for a player.
     */
    void SetButtonsBitmask(RwInt32 playerIndex, RwUInt32 bitmask);

    /**
     * @brief Sets analog axis deflection value [-1.0 .. +1.0].
     */
    void SetAxisValue(RwInt32 playerIndex, RwInt32 axisId, RwReal value);

    /**
     * @brief Polls platform keyboard and translates arrow keys/WASD into P1 inputs.
     */
    void PollPlatformInput();

    /**
     * @brief Per-frame latch copying current button states to previous frame registers.
     */
    void Update();

private:
    RwBool m_bFocusIDAllocated[MAX_FOCUS_ID_COUNT]; ///< Allocation flags for each ID in pool.
    RwInt32 m_aFocusStack[MAX_FOCUS_ID_COUNT];       ///< Ordered LIFO stack of active focus contexts.
    RwUInt32 m_iFocusStackIndex;                     ///< Current depth of focus stack.
    RwUInt32 m_iFocusIDPoolIndex;                    ///< Number of currently claimed focus IDs.

    RwUInt32 m_uP1Buttons;                           ///< Player 1 instantaneous digital button bitmask.
    RwUInt32 m_uP1PrevButtons;                       ///< Player 1 previous frame digital button bitmask.
    RwUInt32 m_uP2Buttons;                           ///< Player 2 instantaneous digital button bitmask.
    RwUInt32 m_uP2PrevButtons;                       ///< Player 2 previous frame digital button bitmask.
};

#endif // INPUTMANAGER_H
