#ifndef GUIHEALTHBAR_H
#define GUIHEALTHBAR_H

// ============================================================================
// CannonCruise - GUI Health Bar (GUIHealthBar.h)
// Original path: D:\Projects\CannonCruisePC\code\GUIHealthBar.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include "framework/core/eventhandler/ceventhandler.h"
#include <rwcore.h>

/**
 * @class CGUIHealthBar
 * @brief HUD player health bar display element.
 *
 * Renders a 2D normalized screen-space health gauge for the player vessel.
 * Listens to health update events and manages bounding box coordinates and texture.
 */
class CGUIHealthBar : public RWS::CEventHandler
{
public:
    /**
     * @brief Constructs a new CGUIHealthBar instance with default 100% health and screen positioning.
     * @see FUN_00483ca0
     */
    CGUIHealthBar();

    /**
     * @brief Destructor. Releases allocated resources.
     */
    virtual ~CGUIHealthBar();

    /**
     * @brief Handles incoming event messages (e.g. MSG_UPDATE_PLAYER_HEALTH).
     * @param msg Event message container.
     */
    virtual void HandleEvents(const RWS::CMsg& msg);

    /**
     * @brief Updates the current health percentage value.
     * @param fHealth Normalized health value between [0.0, 1.0].
     * @see FUN_00484500
     */
    void SetHealth(RwReal fHealth);

    /**
     * @brief Sets screen-space normalized coordinates and dimensions for the health bar.
     * @param fX Left position [0.0, 1.0].
     * @param fY Top position [0.0, 1.0].
     * @param fWidth Bar width [0.0, 1.0].
     * @param fHeight Bar height [0.0, 1.0].
     * @see FUN_00484030
     */
    void SetDimensions(RwReal fX, RwReal fY, RwReal fWidth, RwReal fHeight);

    /**
     * @brief Renders the health bar using RenderWare 2D immediate mode (RwIm2DVertex).
     * @see FUN_004849e0
     */
    void Render();

private:
    RwReal     m_fHealth;  ///< Current normalized health ratio [0.0, 1.0]
    RwReal     m_fX;       ///< Normalized screen X coordinate (left anchor)
    RwReal     m_fY;       ///< Normalized screen Y coordinate (top anchor)
    RwReal     m_fWidth;   ///< Normalized screen width
    RwReal     m_fHeight;  ///< Normalized screen height
    RwTexture* m_pTexture; ///< Health bar background / foreground texture
};

#endif // GUIHEALTHBAR_H
