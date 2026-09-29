// ============================================================================
// CannonCruise - RenderWare Studio Framework (camerahelper.cpp)
// Original path: D:\Projects\CannonCruisePC\code\framework\toolkits\world\helpers\camerahelper.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_00429170 — CameraHelper::CreateCamera
//   FUN_00429430 — CameraHelper::DestroyCamera
//   FUN_004295b0 — CameraHelper::SetFieldOfView
// ============================================================================

#include "camerahelper.h"
#include <cassert>
#include <cmath>

namespace RWS
{
    // ========================================================================
    // CameraHelper::CreateCamera (FUN_00429170)
    // ASM: 00429170 — Kamera, frame, raster ve Z-raster oluştur
    // ========================================================================
    RwCamera* CameraHelper::CreateCamera(RwInt32 width, RwInt32 height, RwBool zBuffer)
    {
        RwCamera* pCamera = RwCameraCreate();
        assert(pCamera != 0 && "Failed POST-condition: _camera");

        RwFrame* pFrame = RwFrameCreate();
        assert(pFrame != 0 && "Failed POST-condition: _frame");

        RwCameraSetFrame(pCamera, pFrame);

        RwRaster* pRaster = RwRasterCreate(width, height, 0, rwRASTERTYPECAMERASUB);
        assert(pRaster != 0 && "Failed POST-condition: RwCameraGetRaster(_camera)");
        RwCameraSetRaster(pCamera, pRaster);

        if (zBuffer)
        {
            RwRaster* pZRaster = RwRasterCreate(width, height, 0, rwRASTERTYPEZBUFFER);
            assert(pZRaster != 0 && "Failed POST-condition: RwCameraGetZRaster(_camera)");
            RwCameraSetZRaster(pCamera, pZRaster);
        }

        return pCamera;
    }

    // ========================================================================
    // CameraHelper::DestroyCamera (FUN_00429430)
    // ASM: 00429430 — Kamera ve bağlı raster/frameleri temizle
    // ========================================================================
    void CameraHelper::DestroyCamera(RwCamera* pCamera)
    {
        assert(pCamera != 0 && "Failed PRE-condition: pCamera");

        RwFrame* pFrame = RwCameraGetFrame(pCamera);
        if (pFrame)
        {
            RwCameraSetFrame(pCamera, 0);
            RwFrameDestroy(pFrame);
        }

        RwRaster* pRaster = RwCameraGetRaster(pCamera);
        if (pRaster)
        {
            RwCameraSetRaster(pCamera, 0);
            RwRasterDestroy(pRaster);
        }

        RwRaster* pZRaster = RwCameraGetZRaster(pCamera);
        if (pZRaster)
        {
            RwCameraSetZRaster(pCamera, 0);
            RwRasterDestroy(pZRaster);
        }

        RwCameraDestroy(pCamera);
    }

    // ========================================================================
    // CameraHelper::SetFieldOfView (FUN_004295b0)
    // ASM: 004295b0 — Kamera FOV ve projeksiyon matrisi ayarla
    // ========================================================================
    void CameraHelper::SetFieldOfView(RwCamera* pCamera, RwReal fov, RwInt32 width, RwInt32 height)
    {
        assert(pCamera != 0 && "Failed PRE-condition: pCamera");

        RwV2d viewWindow;
        RwReal halfFovRad = (fov * 0.5f) * (3.14159265f / 180.0f);
        viewWindow.x = std::tan(halfFovRad);
        viewWindow.y = viewWindow.x * ((RwReal)height / (RwReal)width);

        RwCameraSetViewWindow(pCamera, &viewWindow);
    }

    void CameraHelper::SetNearFarClip(RwCamera* pCamera, RwReal nearClip, RwReal farClip)
    {
        assert(pCamera != 0 && "Failed PRE-condition: pCamera");

        RwCameraSetNearClipPlane(pCamera, nearClip);
        RwCameraSetFarClipPlane(pCamera, farClip);
    }

    void CameraHelper::UpdateScreenSize(RwCamera* pCamera, RwInt32 width, RwInt32 height)
    {
        if (!pCamera) return;

        RwRaster* pOldRaster = RwCameraGetRaster(pCamera);
        if (pOldRaster)
        {
            RwRaster* pNewRaster = RwRasterCreate(width, height, 0, rwRASTERTYPECAMERASUB);
            RwCameraSetRaster(pCamera, pNewRaster);
            RwRasterDestroy(pOldRaster);
        }

        RwRaster* pOldZRaster = RwCameraGetZRaster(pCamera);
        if (pOldZRaster)
        {
            RwRaster* pNewZRaster = RwRasterCreate(width, height, 0, rwRASTERTYPEZBUFFER);
            RwCameraSetZRaster(pCamera, pNewZRaster);
            RwRasterDestroy(pOldZRaster);
        }
    }
}
