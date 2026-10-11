#ifndef MENUDELEGATE_H
#define MENUDELEGATE_H

// ============================================================================
// CannonCruise - Menu Delegate Base (MenuDelegate.h)
// Original path: D:\Projects\CannonCruisePC\code\MenuDelegate.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include "framework/core/eventhandler/ceventhandler.h"
#include <rwcore.h>

class CMenuBehavior;

/**
 * @class CMenuDelegate
 * @brief Abstract base class for menu action and animation property command delegates.
 *
 * Provides the interface for querying and mutating menu states, animation time positions,
 * and keyframe indices within the RenderWare 2D Maestro menu hierarchy.
 */
class CMenuDelegate : public RWS::CEventHandler
{
public:
    /**
     * @brief Constructs a new CMenuDelegate attached to an optional parent menu behavior.
     * @param pMenu Owning menu behavior instance.
     * @see FUN_00444c80
     */
    CMenuDelegate(CMenuBehavior* pMenu = 0);

    /**
     * @brief Destructor.
     */
    virtual ~CMenuDelegate();

    /**
     * @brief Executes the delegated menu action or property transfer.
     */
    virtual void Execute() = 0;

protected:
    CMenuBehavior* m_pMenu;      ///< Owning menu behavior controller
    void*          m_pAnimProps; ///< Internal animation properties handle
};

#endif // MENUDELEGATE_H
