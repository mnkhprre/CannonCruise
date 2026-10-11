#ifndef RWS_CAMERAHELPER_H
#define RWS_CAMERAHELPER_H

// ============================================================================
// CannonCruise - RenderWare Studio Framework (camerahelper.h)
// Original path: D:\Projects\CannonCruisePC\code\framework\toolkits\world\helpers\camerahelper.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// Binary ref: FUN_00408000 - FUN_00408200 (RWS::CameraHelper)
// ============================================================================

#include <rwcore.h>

namespace RWS
{
    /**
     * @class CameraHelper
     * @brief RenderWare kamera nesneleri için yardımcı araç sınıfı.
     *
     * `CScene` ve `CWbCamera` sınıfları bu yardımcı aracılığıyla
     * RwCamera nesnelerini oluşturur, yapılandırır ve yönetir.
     *
     * **Binary referansları:**
     * - CreateCamera: `FUN_00408000`
     * - SetFieldOfView: `FUN_00408080`
     * - SetNearFarClip: `FUN_004080f0`
     */
    class CameraHelper
    {
    public:
        /**
         * @brief Yeni bir RenderWare kamerası oluşturur.
         * @param width   Render hedefi genişliği (piksel).
         * @param height  Render hedefi yüksekliği (piksel).
         * @param zBuffer Z-buffer etkin mi.
         * @return RwCamera* — Oluşturulan kamera; başarısızlık durumunda nullptr.
         */
        static RwCamera* CreateCamera(RwInt32 width, RwInt32 height, RwBool zBuffer);

        /**
         * @brief Kamerayı ve ilişkili render hedeflerini yok eder.
         * @param pCamera Yok edilecek kamera.
         */
        static void DestroyCamera(RwCamera* pCamera);

        /**
         * @brief Kameranın görüş alanını (FoV) ayarlar.
         * @param pCamera Hedef kamera.
         * @param fov     Dikey görüş açısı (radyan).
         * @param width   Görünüm genişliği.
         * @param height  Görünüm yüksekliği.
         */
        static void SetFieldOfView(RwCamera* pCamera, RwReal fov, RwInt32 width, RwInt32 height);

        /**
         * @brief Kameranın ön/arka kırpma düzlemlerini ayarlar.
         * @param pCamera  Hedef kamera.
         * @param nearClip Ön kırpma mesafesi.
         * @param farClip  Arka kırpma mesafesi.
         */
        static void SetNearFarClip(RwCamera* pCamera, RwReal nearClip, RwReal farClip);

        /**
         * @brief Kameranın render hedefini yeni ekran boyutuna göre günceller.
         * @param pCamera Hedef kamera.
         * @param width   Yeni genişlik (piksel).
         * @param height  Yeni yükseklik (piksel).
         */
        static void UpdateScreenSize(RwCamera* pCamera, RwInt32 width, RwInt32 height);
    };
}

#endif // RWS_CAMERAHELPER_H
