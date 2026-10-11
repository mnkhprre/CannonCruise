#ifndef MENUSETFRAMEDELEGATE_H
#define MENUSETFRAMEDELEGATE_H

// ============================================================================
// CannonCruise - Menu Set Frame Delegate (MenuSetFrameDelegate.h)
// Original path: D:\Projects\CannonCruisePC\code\MenuSetFrameDelegate.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include "MenuDelegate.h"

/**
 * @class CMenuSetFrameDelegate
 * @brief Delegate for setting 2D animation keyframe index.
 *
 * Seeks a RenderWare 2D animation sequence to a specific keyframe number.
 */
class CMenuSetFrameDelegate : public CMenuDelegate
{
public:
    /**
     * @brief Constructs a new CMenuSetFrameDelegate instance.
     * @param pMenu Optional owning menu behavior.
     * @see FUN_00445ac0
     */
    CMenuSetFrameDelegate(CMenuBehavior* pMenu = 0);

    /**
     * @brief Destructor.
     */
    virtual ~CMenuSetFrameDelegate();

    /**
     * @brief Applies keyframe seek to targeted animation object.
     */
    virtual void Execute();

    /**
     * @brief Sets the target animation keyframe number.
     * @param iFrame Non-negative keyframe index.
     * @see FUN_00445e90
     */
    void SetFrame(RwInt32 iFrame);

private:
    RwInt32 m_iFrame; ///< Target animation keyframe index
};

#endif // MENUSETFRAMEDELEGATE_H
