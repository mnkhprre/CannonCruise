#ifndef WBRENDER_H
#define WBRENDER_H

// ============================================================================
// CannonCruise - WaveBreaker Master Render Coordinator (WbRender.h)
// Original path: D:\Projects\CannonCruisePC\code\WbRender.h
// Reverse-engineered via Ghidra and CC.asm analysis.
// ============================================================================

#include "framework/core/eventhandler/ceventhandler.h"
#include <rwcore.h>
#include <rpworld.h>

/**
 * @class CWbRender
 * @brief Master render loop pipeline orchestrator ("WaveBreaker Render").
 *
 * Coordinates multi-pass rendering stages:
 * 1. Camera buffer clearance (color and Z-buffer).
 * 2. 3D World BSP and static sector rendering.
 * 3. Dynamic entities, ships, cannonballs, and foliage.
 * 4. Ocean dynamic water surface mesh.
 * 5. 3D particle effects and alpha blended sprites.
 * 6. 2D HUD overlays, minimap, health bar, subtitles, and fullscreen fade.
 */
class CWbRender : public RWS::CEventHandler
{
public:
    /**
     * @brief Constructs a CWbRender pipeline manager.
     * @see FUN_00430150
     */
    CWbRender();

    /**
     * @brief Destructor.
     */
    virtual ~CWbRender();

    /**
     * @brief Handles render pipeline event messages.
     * @param msg Event message container.
     */
    virtual void HandleEvents(const RWS::CMsg& msg);

    /**
     * @brief Executes a single complete frame render pass.
     * @param pCamera Active rendering camera.
     * @param pWorld Active RenderWare RpWorld.
     * @see FUN_004308e0
     */
    void RenderFrame(RwCamera* pCamera, RpWorld* pWorld);

    /**
     * @brief Clears camera raster viewport with clear color and depth.
     * @param pCamera Active camera.
     */
    void ClearCamera(RwCamera* pCamera);

    /**
     * @brief Sets clear backdrop color.
     * @param color RGBA background color.
     */
    void SetClearColor(RwRGBA color) { m_ClearColor = color; }

private:
    RwRGBA m_ClearColor; ///< Camera background clear color (ocean sky/fog)
    RwBool m_bRendering; ///< Reentrancy protection flag
};

#endif // WBRENDER_H
