#ifndef RWS_CAMERAHELPER_H
#define RWS_CAMERAHELPER_H

// ============================================================================
// CannonCruise - RenderWare Studio Framework (camerahelper.h)
// Original path: D:\Projects\CannonCruisePC\code\framework\toolkits\world\helpers\camerahelper.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include <rwcore.h>

namespace RWS
{
    class CameraHelper
    {
    public:
        // Kamera oluşturma & yok etme
        static RwCamera* CreateCamera(RwInt32 width, RwInt32 height, RwBool zBuffer);
        static void DestroyCamera(RwCamera* pCamera);

        // Görünüm alanı & projeksiyon
        static void SetFieldOfView(RwCamera* pCamera, RwReal fov, RwInt32 width, RwInt32 height);
        static void SetNearFarClip(RwCamera* pCamera, RwReal nearClip, RwReal farClip);

        // Ekran boyutunu güncelleme
        static void UpdateScreenSize(RwCamera* pCamera, RwInt32 width, RwInt32 height);
    };
}

#endif // RWS_CAMERAHELPER_H
