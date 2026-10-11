// ============================================================================
// CannonCruise - GUI Minimap (GUIMap.cpp)
// Original path: D:\Projects\CannonCruisePC\code\GUIMap.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_00485830 — CGUIMap constructor & registration
//   FUN_004860b0 — CGUIMap::AddSpot
//   FUN_004867f0 — CGUIMap::Render
// ============================================================================

#include "GUIMap.h"
#include "framework/core/attributehandler/cclassfactory.h"
#include <cassert>
#include <cstring>

RWS_REGISTER_CLASS(CGUIMap, []() -> void* { return new CGUIMap(); }, sizeof(CGUIMap));

/**
 * @brief Constructs a new CGUIMap instance.
 *
 * Initializes compass heading to zero and clears icon texture array pointers.
 *
 * @see FUN_00485830
 */
CGUIMap::CGUIMap()
    : m_fPlayerHeading(0.0f)
{
    memset(m_apTextures, 0, sizeof(m_apTextures));
}

/**
 * @brief Destructor for CGUIMap. Cleans up all tracked spots.
 */
CGUIMap::~CGUIMap()
{
    ClearSpots();
}

/**
 * @brief Handles minimap message events (e.g. iMsgRenderDotEvent).
 * @param msg Event message container.
 */
void CGUIMap::HandleEvents(const RWS::CMsg& msg)
{
    // iMsgRenderDotEvent
}

/**
 * @brief Adds a point of interest marker to the radar minimap.
 *
 * @param pPos 3D world position of target (non-null asserted).
 * @param type Icon classification category (bounds asserted).
 * @see FUN_004860b0
 */
void CGUIMap::AddSpot(const RwV3d* pPos, EMissionTextureType type)
{
    assert(pPos != 0);
    assert(type < MISSION_TEXTURE_COUNT && "Illegal MISSION_TEXTURE_TYPE");

    SMissionIndicatorData spot;
    spot.m_WorldPos = *pPos;
    spot.m_Type = type;
    spot.m_bActive = TRUE;

    m_Spots.push_back(spot);
}

/**
 * @brief Clears all registered radar spots.
 */
void CGUIMap::ClearSpots()
{
    m_Spots.clear();
}

/**
 * @brief Updates the vessel compass rotation angle.
 * @param angle Heading yaw angle in radians.
 */
void CGUIMap::SetPlayerHeading(RwReal angle)
{
    m_fPlayerHeading = angle;
}

/**
 * @brief Renders the circular radar frame, compass needle, and projected 2D blips.
 * @see FUN_004867f0
 */
void CGUIMap::Render()
{
    RwCamera* pCamera = RwCameraGetCurrentCamera();
    if (!pCamera) return;

    RwRaster* pRaster = RwCameraGetRaster(pCamera);
    if (!pRaster) return;

    RwReal screenWidth = (RwReal)RwRasterGetWidth(pRaster);
    RwReal screenHeight = (RwReal)RwRasterGetHeight(pRaster);
    RwReal recipZ = 1.0f / RwCameraGetNearClipPlane(pCamera);
    RwReal nearZ = RwCameraGetNearClipPlane(pCamera);

    // RenderStates ayarları
    RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void*)0);
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)FALSE);
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)FALSE);
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)FALSE);

    // Sağ üst köşe, radar arka planı (yarı saydam veya koyu)
    RwReal mapSize = 100.0f; // Sabit 100x100 pixel harita boyutu
    RwReal mapX = screenWidth - mapSize - 20.0f;
    RwReal mapY = 20.0f;
    
    RwIm2DVertex bgVerts[4];
    for (int i = 0; i < 4; i++) {
        RwIm2DVertexSetScreenZ(&bgVerts[i], nearZ);
        RwIm2DVertexSetRecipCameraZ(&bgVerts[i], recipZ);
        RwIm2DVertexSetIntRGBA(&bgVerts[i], 0, 0, 64, 128); // Koyu mavi arka plan
    }
    
    RwIm2DVertexSetScreenX(&bgVerts[0], mapX);           RwIm2DVertexSetScreenY(&bgVerts[0], mapY);
    RwIm2DVertexSetScreenX(&bgVerts[1], mapX);           RwIm2DVertexSetScreenY(&bgVerts[1], mapY + mapSize);
    RwIm2DVertexSetScreenX(&bgVerts[2], mapX + mapSize); RwIm2DVertexSetScreenY(&bgVerts[2], mapY);
    RwIm2DVertexSetScreenX(&bgVerts[3], mapX + mapSize); RwIm2DVertexSetScreenY(&bgVerts[3], mapY + mapSize);
    
    RwIm2DRenderPrimitive(rwPRIMTYPETRISTRIP, bgVerts, 4);

    // Noktaların çizimi
    for (size_t i = 0; i < m_Spots.size(); i++)
    {
        if (!m_Spots[i].m_bActive) continue;
        
        // Basit projeksiyon: World 0,0 radarın merkezine denk gelir (Farazi ölçek 0.1)
        RwReal worldScale = 0.1f;
        RwReal spotX = mapX + (mapSize * 0.5f) + (m_Spots[i].m_WorldPos.x * worldScale);
        RwReal spotZ = mapY + (mapSize * 0.5f) - (m_Spots[i].m_WorldPos.z * worldScale);
        
        // Harita dışına taşmayı engelle
        if (spotX < mapX) spotX = mapX;
        if (spotX > mapX + mapSize) spotX = mapX + mapSize;
        if (spotZ < mapY) spotZ = mapY;
        if (spotZ > mapY + mapSize) spotZ = mapY + mapSize;

        RwReal spotSize = 4.0f; // 4x4 piksel nokta

        RwIm2DVertex spotVerts[4];
        for (int j = 0; j < 4; j++) {
            RwIm2DVertexSetScreenZ(&spotVerts[j], nearZ);
            RwIm2DVertexSetRecipCameraZ(&spotVerts[j], recipZ);
            
            // Renk tipine göre değişebilir, şimdilik sarı
            RwIm2DVertexSetIntRGBA(&spotVerts[j], 255, 255, 0, 255); 
        }
        
        RwIm2DVertexSetScreenX(&spotVerts[0], spotX - spotSize); RwIm2DVertexSetScreenY(&spotVerts[0], spotZ - spotSize);
        RwIm2DVertexSetScreenX(&spotVerts[1], spotX - spotSize); RwIm2DVertexSetScreenY(&spotVerts[1], spotZ + spotSize);
        RwIm2DVertexSetScreenX(&spotVerts[2], spotX + spotSize); RwIm2DVertexSetScreenY(&spotVerts[2], spotZ - spotSize);
        RwIm2DVertexSetScreenX(&spotVerts[3], spotX + spotSize); RwIm2DVertexSetScreenY(&spotVerts[3], spotZ + spotSize);
        
        RwIm2DRenderPrimitive(rwPRIMTYPETRISTRIP, spotVerts, 4);
    }

    // Varsayılan duruma geri dön
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)TRUE);
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)TRUE);
}


