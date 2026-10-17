#ifndef FADE_H
#define FADE_H

// ============================================================================
// CannonCruise - Screen Fade Effect (Fade.h)
// Original path: D:\Projects\CannonCruisePC\code\Fade.h
// Reverse-engineered via Ghidra and CC.asm analysis.
// ============================================================================

#include "framework/core/eventhandler/ceventhandler.h"
#include <rwcore.h>

/**
 * @class CFade
 * @brief Fullscreen color fade transition controller.
 *
 * Handles smooth screen fading (Fade In, Fade Out, Hold) using full-screen
 * 2D immediate mode textured/solid quads for scene transitions and cutscenes.
 */
class CFade : public RWS::CEventHandler
{
public:
    /**
     * @enum EFadeState
     * @brief Screen transition animation states.
     */
    enum EFadeState
    {
        FADE_IDLE = 0, ///< No fade active; completely transparent
        FADE_IN,       ///< Fading from color to transparent (entering scene)
        FADE_OUT,      ///< Fading from transparent to color (leaving scene)
        FADE_HOLD      ///< Maintaining solid color covering the screen
    };

    /**
     * @brief Constructs a CFade effect handler.
     * @see FUN_00479e90
     */
    CFade();

    /**
     * @brief Destructor. Releases input focus lock if held.
     */
    virtual ~CFade();

    /**
     * @brief Handles incoming event messages (e.g. MSG_FADE_RENDER).
     * @param msg Event message container.
     */
    virtual void HandleEvents(const RWS::CMsg& msg);

    /**
     * @brief Initiates a screen fade transition with color and duration.
     * @param state Target fade transition state.
     * @param duration Fade transition duration in seconds.
     * @param color Target RGBA tint color.
     */
    void StartFade(EFadeState state, RwReal duration, RwRGBA color);

    /**
     * @brief Renders the full-screen colored quad overlay.
     * @see FUN_0047a950
     */
    void Render();

    /**
     * @brief Updates alpha interpolation over elapsed time.
     * @param deltaTime Elapsed frame time in seconds.
     * @see FUN_0047a530
     */
    void Update(RwReal deltaTime);

    /**
     * @brief Checks if a fade sequence is actively progressing.
     * @return TRUE if fading or holding color, FALSE if idle.
     */
    RwBool IsFading() const { return m_eState != FADE_IDLE; }

private:
    EFadeState m_eState;       ///< Current fade transition state
    RwReal     m_fDuration;    ///< Target total transition time in seconds
    RwReal     m_fCurrentTime; ///< Elapsed transition time in seconds
    RwRGBA     m_Color;        ///< Current color and alpha values
    RwInt32    m_iFocusID;     ///< Input focus lock ID to disable gameplay during fade
};

#endif // FADE_H
