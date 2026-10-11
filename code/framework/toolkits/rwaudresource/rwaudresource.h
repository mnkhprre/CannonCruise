#ifndef RWS_RWAUDRESOURCE_H
#define RWS_RWAUDRESOURCE_H

// ============================================================================
// CannonCruise - RenderWare Studio Framework (rwaudresource.h)
// Original path: D:\Projects\CannonCruisePC\code\framework\toolkits\rwaudresource\rwaudresource.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// Binary ref: FUN_00409600 - FUN_004096a0 (RWS::RwAudResource)
// ============================================================================

#include <rwcore.h>

namespace RWS
{
    /**
     * @class RwAudResource
     * @brief Ses veritabanlarını ve sözlüklerini (`Wave Dictionary`) yükleyen sınıf.
     *
     * `CResourceManager` tarafından `.rwa` veya `.wav` paketlerini yönetmek
     * için çağrılır; altta yatan Miles Sound System yapılarına bağlanır.
     *
     * **Binary referansları:**
     * - LoadWaveDict: `FUN_00409600`
     * - UnloadWaveDict: `FUN_00409660`
     */
    class RwAudResource
    {
    public:
        /**
         * @brief Belirtilen dosyadaki ses sözlüğünü belleğe yükler.
         * @param pFilename Yüklenecek ses sözlüğü dosyası.
         * @return void* — Yüklenen Wave Dictionary yapısına işaretçi.
         */
        static void* LoadWaveDict(const char* pFilename);

        /**
         * @brief Yüklenmiş ses sözlüğünü bellekten serbest bırakır.
         * @param pWaveDict Yok edilecek Wave Dictionary.
         */
        static void UnloadWaveDict(void* pWaveDict);
    };
}

#endif // RWS_RWAUDRESOURCE_H

