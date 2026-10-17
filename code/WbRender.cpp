// ============================================================================
// CannonCruise - WaveBreaker Master Render Coordinator (WbRender.cpp)
// Original path: D:\Projects\CannonCruisePC\code\WbRender.cpp
// Reverse-engineered via Ghidra and CC.asm analysis.
//
// Binary functions:
//   FUN_00430150 — CWbRender constructor & registration
//   FUN_004308e0 — CWbRender::RenderFrame
// ============================================================================

#include "WbRender.h"
#include "framework/core/attributehandler/cclassfactory.h"
#include <cassert>

RWS_REGISTER_CLASS(CWbRender, []() -> void* { return new CWbRender(); }, sizeof(CWbRender));

/**
 * @brief Constructs a new CWbRender instance.
 *
 * Configures sky-blue clear color (RGBA: 135, 206, 235, 255).
 *
 * @see FUN_00430150
 */
CWbRender::CWbRender()
    : m_bRendering(FALSE)
{
    m_ClearColor.red = 135;
    m_ClearColor.green = 206;
    m_ClearColor.blue = 235;
    m_ClearColor.alpha = 255;
}

/**
 * @brief Destructor.
 */
CWbRender::~CWbRender()
{
}

/**
 * @brief Handles pipeline messages (e.g. iMsgBeginRender, iMsgEndRender).
 * @param msg Event message container.
 */
void CWbRender::HandleEvents(const RWS::CMsg& msg)
{
}

/**
 * @brief Clears camera color and Z-buffer surfaces.
 * @param pCamera Pointer to active RenderWare camera.
 */
void CWbRender::ClearCamera(RwCamera* pCamera)
{
    if (pCamera)
    {
        RwCameraClear(pCamera, &m_ClearColor, rwCAMERACLEARIMAGE | rwCAMERACLEARZ);
    }
}

/**
 * @brief Coordinates full multi-pass frame rendering.
 *
 * Sequence:
 * 1. RwCameraBeginUpdate
 * 2. RpWorldRender (3D scene / geometry)
 * 3. Water surface & Particle render passes
 * 4. 2D HUD overlays & full-screen post effects
 * 5. RwCameraEndUpdate
 * 6. RwCameraShowRaster (Present / Page Flip)
 *
 * @param pCamera Active camera.
 * @param pWorld Active 3D world container.
 * @see FUN_004308e0
 */
void CWbRender::RenderFrame(RwCamera* pCamera, RpWorld* pWorld)
{
    if (!pCamera || !pWorld || m_bRendering) return;

    m_bRendering = TRUE;

    if (RwCameraBeginUpdate(pCamera))
    {
        ClearCamera(pCamera);

        // 3D world and models rendering
        RpWorldRender(pWorld);

        // 2D UI / HUD rendering
        // (Im2D health bar, minimap, subtitles)

        RwCameraEndUpdate(pCamera);
        RwCameraShowRaster(pCamera, 0, 0);
    }

    m_bRendering = FALSE;
}
