#ifndef RWS_RWGFXRESOURCE_H
#define RWS_RWGFXRESOURCE_H

// ============================================================================
// CannonCruise - RenderWare Studio Framework (rwgfxresource.h)
// Original path: D:\Projects\CannonCruisePC\code\framework\toolkits\rwgfxresource\rwgfxresource.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// Binary ref: FUN_00409800 - FUN_004098f0 (RWS::RwGfxResource)
// ============================================================================

#include <rwcore.h>
#include <rpworld.h>

namespace RWS
{
    /**
     * @class RwGfxResource
     * @brief RenderWare grafik kaynaklarının (.dff, .bsp, .txd vb.) yüklenmesini sağlar.
     *
     * `CResourceManager` ve `CStreamHandler` ile entegre çalışır. Kaynakların
     * tipi (`pType`) string'ine göre uygun RenderWare yığın işleyicisini seçer.
     *
     * **Binary referansları:**
     * - Load: `FUN_00409800`
     * - Unload: `FUN_00409860`
     * - RegisterHandlers: `FUN_004098b0`
     */
    class RwGfxResource
    {
    public:
        /**
         * @brief Grafik kaynağını dosya yolundan okuyarak belleğe yükler.
         * @param pFilename Okunacak dosya yolu.
         * @param pType     Kaynak tipi (`CTextureResource`, `CWorldGeometryResource` vb.)
         * @return void* — Oluşturulan kaynağa işaretçi (RpClump, RwTexture vb.)
         */
        static void* Load(const char* pFilename, const char* pType);

        /**
         * @brief Grafik kaynağını bellekten siler.
         * @param pResource Yok edilecek kaynak.
         * @param pType     Kaynak tipi.
         */
        static void Unload(void* pResource, const char* pType);

        /**
         * @brief Grafik okuma yığın işleyicilerini `CStreamHandler`'a kaydeder.
         *
         * Motor başlangıcında çağrılarak `.dff`, `.bsp` gibi dosyaların
         * ayrıştırılabilmesi sağlanır.
         */
        static void RegisterHandlers();
    };
}

#endif // RWS_RWGFXRESOURCE_H

