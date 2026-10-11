// ============================================================================
// CannonCruise - GUI Health Bar (GUIHealthBar.cpp)
// Original path: D:\Projects\CannonCruisePC\code\GUIHealthBar.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_00483ca0 — CGUIHealthBar constructor & registration
//   FUN_00484030 — CGUIHealthBar::SetDimensions
//   FUN_00484500 — CGUIHealthBar::SetHealth
//   FUN_004849e0 — CGUIHealthBar::Render
// ============================================================================

#include "GUIHealthBar.h"
#include "framework/core/attributehandler/cclassfactory.h"
#include <cassert>

RWS_REGISTER_CLASS(CGUIHealthBar, []() -> void* { return new CGUIHealthBar(); }, sizeof(CGUIHealthBar));

/**
 * @brief Constructs a new CGUIHealthBar instance.
 *
 * Initializes default dimensions (5% margin left/top, 20% width, 3% height)
 * and sets the initial health capacity to full (1.0).
 *
 * @see FUN_00483ca0
 */
CGUIHealthBar::CGUIHealthBar()
    : m_fHealth(1.0f),
      m_fX(0.05f),
      m_fY(0.05f),
      m_fWidth(0.2f),
      m_fHeight(0.03f),
      m_pTexture(0)
{
}

/**
 * @brief Destructor for CGUIHealthBar.
 */
CGUIHealthBar::~CGUIHealthBar()
{
}

/**
 * @brief Handles incoming event messages (such as health delta/sync).
 *
 * Extracts updated health real value from message payload and updates bar ratio.
 *
 * @param msg Event message container.
 */
void CGUIHealthBar::HandleEvents(const RWS::CMsg& msg)
{
    // MSG_UPDATE_PLAYER_HEALTH
    if (msg.m_pData)
    {
        const RwReal* pNewHealth = (const RwReal*)msg.m_pData;
        SetHealth(*pNewHealth);
    }
}

/**
 * @brief Sets the normalized health value with validation assertions.
 *
 * @param fHealth Health percentage ratio, asserted within [0.0, 1.0].
 * @see FUN_00484500
 */
void CGUIHealthBar::SetHealth(RwReal fHealth)
{
    assert(fHealth >= 0.0f && "*pNewHealth >= 0.0f Failed");
    assert(fHealth <= 1.0f && "*pNewHealth <= 1.0f Failed");

    m_fHealth = fHealth;
}

/**
 * @brief Sets normalized screen coordinates and bounds with validation assertions.
 *
 * @param fX Left screen margin [0.0, 1.0].
 * @param fY Top screen margin [0.0, 1.0].
 * @param fWidth Bounding box width [0.0, 1.0].
 * @param fHeight Bounding box height [0.0, 1.0].
 * @see FUN_00484030
 */
void CGUIHealthBar::SetDimensions(RwReal fX, RwReal fY, RwReal fWidth, RwReal fHeight)
{
    assert(fX >= 0.0f && "fX >= 0.0f Failed");
    assert(fY >= 0.0f && "fY >= 0.0f Failed");
    assert(fWidth <= 1.0f && "fWidth <= 1.0f Failed");
    assert(fHeight <= 1.0f && "fHeight <= 1.0f Failed");

    m_fX = fX;
    m_fY = fY;
    m_fWidth = fWidth;
    m_fHeight = fHeight;
}

/**
 * @brief Renders the 2D health bar frame and fill geometry.
 *
 * Uses RenderWare 2D immediate mode rendering (RwIm2DVertex).
 *
 * @see FUN_004849e0
 */
void CGUIHealthBar::Render()
{
    RwCamera* pCamera = RwCameraGetCurrentCamera();
    if (!pCamera) return;

    RwRaster* pRaster = RwCameraGetRaster(pCamera);
    if (!pRaster) return;

    RwReal screenWidth = (RwReal)RwRasterGetWidth(pRaster);
    RwReal screenHeight = (RwReal)RwRasterGetHeight(pRaster);
    RwReal recipZ = 1.0f / RwCameraGetNearClipPlane(pCamera);
    RwReal nearZ = RwCameraGetNearClipPlane(pCamera);

    // Ekran koordinatlarını hesapla (Normalize 0.0-1.0 -> Pixel)
    RwReal x = m_fX * screenWidth;
    RwReal y = m_fY * screenHeight;
    RwReal w = m_fWidth * screenWidth;
    RwReal h = m_fHeight * screenHeight;
    RwReal healthW = w * m_fHealth;

    // RenderStates ayarları
    RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void*)0);
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)FALSE);
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)FALSE);
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)FALSE);

    // Arka plan (Boşalan canı temsil eden kırmızı alan)
    RwIm2DVertex bgVerts[4];
    for (int i = 0; i < 4; i++) {
        RwIm2DVertexSetScreenZ(&bgVerts[i], nearZ);
        RwIm2DVertexSetRecipCameraZ(&bgVerts[i], recipZ);
        RwIm2DVertexSetIntRGBA(&bgVerts[i], 128, 0, 0, 255); // Koyu kırmızı
    }
    
    RwIm2DVertexSetScreenX(&bgVerts[0], x);     RwIm2DVertexSetScreenY(&bgVerts[0], y);
    RwIm2DVertexSetScreenX(&bgVerts[1], x);     RwIm2DVertexSetScreenY(&bgVerts[1], y + h);
    RwIm2DVertexSetScreenX(&bgVerts[2], x + w); RwIm2DVertexSetScreenY(&bgVerts[2], y);
    RwIm2DVertexSetScreenX(&bgVerts[3], x + w); RwIm2DVertexSetScreenY(&bgVerts[3], y + h);
    
    RwIm2DRenderPrimitive(rwPRIMTYPETRISTRIP, bgVerts, 4);

    // Ön plan (Aktif canı temsil eden yeşil alan)
    if (m_fHealth > 0.0f)
    {
        RwIm2DVertex fgVerts[4];
        for (int i = 0; i < 4; i++) {
            RwIm2DVertexSetScreenZ(&fgVerts[i], nearZ);
            RwIm2DVertexSetRecipCameraZ(&fgVerts[i], recipZ);
            RwIm2DVertexSetIntRGBA(&fgVerts[i], 0, 255, 0, 255); // Parlak yeşil
        }
        
        RwIm2DVertexSetScreenX(&fgVerts[0], x);          RwIm2DVertexSetScreenY(&fgVerts[0], y);
        RwIm2DVertexSetScreenX(&fgVerts[1], x);          RwIm2DVertexSetScreenY(&fgVerts[1], y + h);
        RwIm2DVertexSetScreenX(&fgVerts[2], x + healthW);RwIm2DVertexSetScreenY(&fgVerts[2], y);
        RwIm2DVertexSetScreenX(&fgVerts[3], x + healthW);RwIm2DVertexSetScreenY(&fgVerts[3], y + h);

        RwIm2DRenderPrimitive(rwPRIMTYPETRISTRIP, fgVerts, 4);
    }

    // Varsayılan duruma geri dön
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)TRUE);
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)TRUE);
}

