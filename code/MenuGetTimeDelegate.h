#ifndef MENUGETTIMEDELEGATE_H
#define MENUGETTIMEDELEGATE_H

// ============================================================================
// CannonCruise - Menu Get Time Delegate (MenuGetTimeDelegate.h)
// Original path: D:\Projects\CannonCruisePC\code\MenuGetTimeDelegate.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include "MenuDelegate.h"

/**
 * @class CMenuGetTimeDelegate
 * @brief Delegate for querying current 2D animation playback time position.
 *
 * Inspects the current normalized time of a RenderWare 2D animation track.
 */
class CMenuGetTimeDelegate : public CMenuDelegate
{
public:
    /**
     * @brief Constructs a new CMenuGetTimeDelegate instance.
     * @param pMenu Optional owning menu behavior.
     * @see FUN_004455e0
     */
    CMenuGetTimeDelegate(CMenuBehavior* pMenu = 0);

    /**
     * @brief Destructor.
     */
    virtual ~CMenuGetTimeDelegate();

    /**
     * @brief Executes the animation time lookup.
     * @see FUN_00445750
     */
    virtual void Execute();

    /**
     * @brief Retrieves the active animation playback time.
     * @return Normalized playback time in seconds or ratio.
     */
    RwReal GetTime() const;

private:
    void* m_pAnim; ///< Internal animation sequence pointer
};

#endif // MENUGETTIMEDELEGATE_H
