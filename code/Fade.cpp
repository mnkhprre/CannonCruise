// ============================================================================
// CannonCruise - Screen Fade Effect (Fade.cpp)
// Original path: D:\Projects\CannonCruisePC\code\Fade.cpp
// Reverse-engineered via Ghidra and CC.asm analysis.
//
// Binary functions:
//   FUN_00479e90 — CFade constructor & registration
//   FUN_0047a530 — CFade::Update
//   FUN_0047a950 — CFade::Render
// ============================================================================

#include "Fade.h"
#include "InputManager.h"
#include "framework/core/attributehandler/cclassfactory.h"
#include <cassert>

RWS_REGISTER_CLASS(CFade, []() -> void* { return new CFade(); }, sizeof(CFade));

/**
 * @brief Constructs a new CFade effect instance.
 *
 * Initializes black transparent color and sets initial state to FADE_IDLE.
 *
 * @see FUN_00479e90
 */
CFade::CFade()
    : m_eState(FADE_IDLE),
      m_fDuration(1.0f),
      m_fCurrentTime(0.0f),
      m_iFocusID(-1)
{
    m_Color.red = 0;
    m_Color.green = 0;
    m_Color.blue = 0;
    m_Color.alpha = 0;
}

/**
 * @brief Destructor.
 *
 * Releases any input lock held with the InputManager.
 */
CFade::~CFade()
{
    if (m_iFocusID != -1)
    {
        CInputManager* pInputMgr = CInputManager::GetSingletonPtr();
        if (pInputMgr)
        {
            pInputMgr->FreeFocusID(m_iFocusID);
        }
        m_iFocusID = -1;
    }
}

/**
 * @brief Handles incoming event messages (such as MSG_FADE_RENDER).
 * @param msg Event message container.
 */
void CFade::HandleEvents(const RWS::CMsg& msg)
{
    // MSG_FADE_RENDER event
}

/**
 * @brief Triggers a screen transition state.
 *
 * Configures transition start alpha (0 for Fade Out, 255 for Fade In)
 * and resets timer.
 *
 * @param state Transition mode (FADE_IN, FADE_OUT, FADE_HOLD).
 * @param duration Duration in seconds.
 * @param color Tint color.
 */
void CFade::StartFade(EFadeState state, RwReal duration, RwRGBA color)
{
    m_eState = state;
    m_fDuration = (duration > 0.0f) ? duration : 1.0f;
    m_fCurrentTime = 0.0f;
    m_Color = color;

    if (m_eState == FADE_OUT)
    {
        m_Color.alpha = 0;
    }
    else if (m_eState == FADE_IN)
    {
        m_Color.alpha = 255;
    }
}

/**
 * @brief Updates linear interpolation of alpha opacity over time.
 *
 * Transitions between states when time elapsed exceeds duration.
 *
 * @param deltaTime Elapsed frame time in seconds.
 * @see FUN_0047a530
 */
void CFade::Update(RwReal deltaTime)
{
    if (m_eState == FADE_IDLE) return;

    m_fCurrentTime += deltaTime;
    RwReal t = m_fCurrentTime / m_fDuration;
    if (t > 1.0f) t = 1.0f;

    switch (m_eState)
    {
        case FADE_IN:
            m_Color.alpha = (RwUInt8)((1.0f - t) * 255.0f);
            if (t >= 1.0f) m_eState = FADE_IDLE;
            break;

        case FADE_OUT:
            m_Color.alpha = (RwUInt8)(t * 255.0f);
            if (t >= 1.0f) m_eState = FADE_HOLD;
            break;

        case FADE_HOLD:
            m_Color.alpha = 255;
            break;

        default:
            assert(false && "Error, unknown state in CFade");
            break;
    }
}

/**
 * @brief Draws full-screen 2D quad overlay using RenderWare Im2D.
 * @see FUN_0047a950
 */
void CFade::Render()
{
    if (m_eState == FADE_IDLE || m_Color.alpha == 0) return;

    RwCamera* pCamera = RwCameraGetCurrentCamera();
    if (!pCamera) return;

    RwRaster* pRaster = RwCameraGetRaster(pCamera);
    if (!pRaster) return;

    RwReal width = (RwReal)RwRasterGetWidth(pRaster);
    RwReal height = (RwReal)RwRasterGetHeight(pRaster);
    RwReal recipZ = 1.0f / RwCameraGetNearClipPlane(pCamera);
    RwReal nearZ = RwCameraGetNearClipPlane(pCamera);

    RwIm2DVertex verts[4];
    
    // Top-Left
    RwIm2DVertexSetScreenX(&verts[0], 0.0f);
    RwIm2DVertexSetScreenY(&verts[0], 0.0f);
    RwIm2DVertexSetScreenZ(&verts[0], nearZ);
    RwIm2DVertexSetRecipCameraZ(&verts[0], recipZ);
    RwIm2DVertexSetIntRGBA(&verts[0], m_Color.red, m_Color.green, m_Color.blue, m_Color.alpha);

    // Bottom-Left
    RwIm2DVertexSetScreenX(&verts[1], 0.0f);
    RwIm2DVertexSetScreenY(&verts[1], height);
    RwIm2DVertexSetScreenZ(&verts[1], nearZ);
    RwIm2DVertexSetRecipCameraZ(&verts[1], recipZ);
    RwIm2DVertexSetIntRGBA(&verts[1], m_Color.red, m_Color.green, m_Color.blue, m_Color.alpha);

    // Top-Right
    RwIm2DVertexSetScreenX(&verts[2], width);
    RwIm2DVertexSetScreenY(&verts[2], 0.0f);
    RwIm2DVertexSetScreenZ(&verts[2], nearZ);
    RwIm2DVertexSetRecipCameraZ(&verts[2], recipZ);
    RwIm2DVertexSetIntRGBA(&verts[2], m_Color.red, m_Color.green, m_Color.blue, m_Color.alpha);

    // Bottom-Right
    RwIm2DVertexSetScreenX(&verts[3], width);
    RwIm2DVertexSetScreenY(&verts[3], height);
    RwIm2DVertexSetScreenZ(&verts[3], nearZ);
    RwIm2DVertexSetRecipCameraZ(&verts[3], recipZ);
    RwIm2DVertexSetIntRGBA(&verts[3], m_Color.red, m_Color.green, m_Color.blue, m_Color.alpha);

    // RenderState settings
    RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void*)0);
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)FALSE);
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)FALSE);
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
    RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
    RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);

    // Draw 2D quad to screen
    RwIm2DRenderPrimitive(rwPRIMTYPETRISTRIP, verts, 4);

    // Restore default state
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)TRUE);
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)TRUE);
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)FALSE);
}

