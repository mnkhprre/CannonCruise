#ifndef MENUGETFRAMEDELEGATE_H
#define MENUGETFRAMEDELEGATE_H

// ============================================================================
// CannonCruise - Menu Get Frame Delegate (MenuGetFrameDelegate.h)
// Original path: D:\Projects\CannonCruisePC\code\MenuGetFrameDelegate.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include "MenuDelegate.h"

/**
 * @class CMenuGetFrameDelegate
 * @brief Delegate for querying current 2D animation frame index.
 *
 * Inspects the current frame counter of a RenderWare 2D animation sequence.
 */
class CMenuGetFrameDelegate : public CMenuDelegate
{
public:
    /**
     * @brief Constructs a new CMenuGetFrameDelegate instance.
     * @param pMenu Optional owning menu behavior.
     * @see FUN_00445100
     */
    CMenuGetFrameDelegate(CMenuBehavior* pMenu = 0);

    /**
     * @brief Destructor.
     */
    virtual ~CMenuGetFrameDelegate();

    /**
     * @brief Executes the frame query command.
     * @see FUN_00445270
     */
    virtual void Execute();

    /**
     * @brief Retrieves the active keyframe index.
     * @return Integer animation frame index.
     */
    RwInt32 GetFrame() const;

private:
    void* m_pAnim; ///< Internal animation sequence pointer
};

#endif // MENUGETFRAMEDELEGATE_H
