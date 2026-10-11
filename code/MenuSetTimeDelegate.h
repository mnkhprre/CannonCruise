#ifndef MENUSETTIMEDELEGATE_H
#define MENUSETTIMEDELEGATE_H

// ============================================================================
// CannonCruise - Menu Set Time Delegate (MenuSetTimeDelegate.h)
// Original path: D:\Projects\CannonCruisePC\code\MenuSetTimeDelegate.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include "MenuDelegate.h"

/**
 * @class CMenuSetTimeDelegate
 * @brief Delegate for setting 2D animation playback time position.
 *
 * Sets the normalized playback time position of a RenderWare 2D animation track.
 */
class CMenuSetTimeDelegate : public CMenuDelegate
{
public:
    /**
     * @brief Constructs a new CMenuSetTimeDelegate instance.
     * @param pMenu Optional owning menu behavior.
     * @see FUN_00446220
     */
    CMenuSetTimeDelegate(CMenuBehavior* pMenu = 0);

    /**
     * @brief Destructor.
     */
    virtual ~CMenuSetTimeDelegate();

    /**
     * @brief Applies playback timestamp mutation to target animation object.
     */
    virtual void Execute();

    /**
     * @brief Sets target playback time position with range validation.
     * @param fTime Normalized playback time ratio in range [0.0, 1.0].
     * @see FUN_004465d0
     */
    void SetTime(RwReal fTime);

private:
    RwReal m_fTime; ///< Target animation playback time position
};

#endif // MENUSETTIMEDELEGATE_H
