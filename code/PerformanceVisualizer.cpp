// ============================================================================
// CannonCruise - Performance Visualizer (PerformanceVisualizer.cpp)
// Original path: D:\Projects\CannonCruisePC\code\PerformanceVisualizer.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_00497de0 — CPerformanceVisualizer constructor & registration
//   FUN_00498020 — CPerformanceVisualizer::Render
// ============================================================================

#include "PerformanceVisualizer.h"
#include "framework/core/attributehandler/cclassfactory.h"
#include <cassert>

RWS_REGISTER_CLASS(CPerformanceVisualizer, []() -> void* { return new CPerformanceVisualizer(); }, sizeof(CPerformanceVisualizer));

CPerformanceVisualizer::CPerformanceVisualizer()
    : m_bEnabled(FALSE),
      m_pCharset(0)
{
}

CPerformanceVisualizer::~CPerformanceVisualizer()
{
}

void CPerformanceVisualizer::HandleEvents(const RWS::CMsg& msg)
{
    // MSG_PROFILE_RENDER
}

void CPerformanceVisualizer::Render()
{
    if (!m_bEnabled) return;

    RwCamera* pCamera = RwCameraGetCurrentCamera();
    if (!pCamera) return;

    RwRaster* pRaster = RwCameraGetRaster(pCamera);
    if (!pRaster) return;

    RwReal recipZ = 1.0f / RwCameraGetNearClipPlane(pCamera);
    RwReal nearZ = RwCameraGetNearClipPlane(pCamera);

    // RenderStates ayarları
    RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void*)0);
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)FALSE);
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)FALSE);
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
    RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
    RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);

    // Sol üst köşe, Performans (FPS/Profil) arka planı
    RwReal panelWidth = 200.0f;
    RwReal panelHeight = 100.0f;
    RwReal panelX = 10.0f;
    RwReal panelY = 10.0f;
    
    RwIm2DVertex bgVerts[4];
    for (int i = 0; i < 4; i++) {
        RwIm2DVertexSetScreenZ(&bgVerts[i], nearZ);
        RwIm2DVertexSetRecipCameraZ(&bgVerts[i], recipZ);
        RwIm2DVertexSetIntRGBA(&bgVerts[i], 0, 0, 0, 150); // Yarı saydam siyah
    }
    
    RwIm2DVertexSetScreenX(&bgVerts[0], panelX);              RwIm2DVertexSetScreenY(&bgVerts[0], panelY);
    RwIm2DVertexSetScreenX(&bgVerts[1], panelX);              RwIm2DVertexSetScreenY(&bgVerts[1], panelY + panelHeight);
    RwIm2DVertexSetScreenX(&bgVerts[2], panelX + panelWidth); RwIm2DVertexSetScreenY(&bgVerts[2], panelY);
    RwIm2DVertexSetScreenX(&bgVerts[3], panelX + panelWidth); RwIm2DVertexSetScreenY(&bgVerts[3], panelY + panelHeight);
    
    RwIm2DRenderPrimitive(rwPRIMTYPETRISTRIP, bgVerts, 4);

    // Varsayılan duruma geri dön
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)TRUE);
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)TRUE);
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)FALSE);
}
