#ifndef RWS_TXTRESOURCE_H
#define RWS_TXTRESOURCE_H

// ============================================================================
// CannonCruise - RenderWare Studio Framework (txtresource.h)
// Original path: D:\Projects\CannonCruisePC\code\framework\toolkits\txtresource\txtresource.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// Binary ref: FUN_00409400 - FUN_004094a0 (RWS::TxtResource)
// ============================================================================

#include <rwcore.h>

namespace RWS
{
    /**
     * @class TxtResource
     * @brief Metin tabanlı kaynak dosyalarını (`.txt`, `.ini`, `.xml` vb.) yöneten sınıf.
     *
     * `CResourceManager` tarafından metin tabanlı oyun ayarlarını veya
     * diyalogları yüklemek için çağrılır.
     *
     * **Binary referansları:**
     * - Load: `FUN_00409400`
     * - Unload: `FUN_00409460`
     */
    class TxtResource
    {
    public:
        /**
         * @brief Dosya sisteminden metin kaynağını okur ve belleğe yükler.
         * @param pFilename Yüklenecek metin dosyasının yolu.
         * @return char* — Dosya içeriğini barındıran null ile sonlandırılmış dize (başarısızlıkta nullptr).
         */
        static char* Load(const char* pFilename);

        /**
         * @brief Yüklenmiş metin kaynağının belleğini serbest bırakır.
         * @param pTextData Serbest bırakılacak metin verisi.
         */
        static void Unload(char* pTextData);
    };
}

#endif // RWS_TXTRESOURCE_H

