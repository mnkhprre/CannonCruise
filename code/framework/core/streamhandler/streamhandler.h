#ifndef RWS_STREAMHANDLER_H
#define RWS_STREAMHANDLER_H

// ============================================================================
// CannonCruise - RenderWare Studio Framework (streamhandler.h)
// Original path: D:\Projects\CannonCruisePC\code\framework\core\streamhandler\streamhandler.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// Binary ref: FUN_00404000 - FUN_004042e0 (RWS::CStreamHandler)
// ============================================================================

#include <rwcore.h>
#include <map>

namespace RWS
{
    /**
     * @typedef ChunkHandlerFunc
     * @brief RenderWare akış yığınını işleyen geri çağrı fonksiyon işaretçisi.
     *
     * `CStreamHandler::RegisterChunkHandler()` ile belirli bir yığın kimliğine
     * (chunkId) bağlanır. Akış işlenirken ilgili yığınla karşılaşıldığında çağrılır.
     *
     * @param pStream   İşlenecek RenderWare akışı.
     * @param chunkId   Yığın kimliği.
     * @param chunkSize Yığın boyutu (bayt).
     * @return TRUE ise yığın başarıyla işlendi.
     */
    typedef RwBool (*ChunkHandlerFunc)(RwStream* pStream, RwUInt32 chunkId, RwUInt32 chunkSize);

    /**
     * @class CStreamHandler
     * @brief RenderWare akış (stream) sistemi için yığın (chunk) yöneticisi.
     *
     * RenderWare binary dosyaları (`.dff`, `.bsp`, `.txd` vb.) yığın tabanlı
     * bir formata sahiptir. `CStreamHandler`, her yığın türü için kayıtlı
     * bir işleyici çağırarak dosyaları ayrıştırır.
     *
     * **Kullanım:**
     * ```cpp
     * CStreamHandler::RegisterChunkHandler(rwID_CLUMP, &OnClumpChunk);
     * CStreamHandler::ProcessFile("levels/level01.bsp");
     * ```
     *
     * **Binary referansları:**
     * - Instance: `FUN_00404000`
     * - ProcessStream: `FUN_00404080`
     * - ProcessFile: `FUN_004040f0`
     * - RegisterChunkHandler: `FUN_00404180`
     */
    class CStreamHandler
    {
    public:
        /// @return Tekil CStreamHandler örneğine işaretçi
        static CStreamHandler* Instance();
        /// @brief Akış sistemini başlatır
        static void Initialize();
        /// @brief Akış sistemini kapatır, tüm yığın işleyicilerini kaldırır
        static void Shutdown();

        /**
         * @brief Belirtilen yığın kimliği için işleyici kaydeder.
         * @param chunkId  Yığın kimliği (örn. `rwID_CLUMP`, `rwID_WORLD`).
         * @param pHandler İşleyici fonksiyon işaretçisi.
         * @return TRUE ise kayıt başarılı.
         */
        static RwBool RegisterChunkHandler(RwUInt32 chunkId, ChunkHandlerFunc pHandler);

        /**
         * @brief Belirtilen yığın kimliğinin işleyicisini kaldırır.
         * @param chunkId Kaldırılacak yığın kimliği.
         */
        static void UnRegisterChunkHandler(RwUInt32 chunkId);

        /**
         * @brief Açık bir RenderWare akışını işler.
         * @param pStream İşlenecek akış.
         * @return TRUE ise akış başarıyla işlendi.
         */
        static RwBool ProcessStream(RwStream* pStream);

        /**
         * @brief Disk dosyasını okuyarak içindeki yığınları işler.
         * @param pFilename Okunacak dosyanın yolu.
         * @return TRUE ise dosya başarıyla işlendi.
         */
        static RwBool ProcessFile(const char* pFilename);

    private:
        CStreamHandler();
        ~CStreamHandler();

        static CStreamHandler* sm_pInstance; ///< Tekil örnek işaretçisi

        std::map<RwUInt32, ChunkHandlerFunc> m_ChunkHandlers; ///< ChunkId → işleyici eşleştirme tablosu
    };
}

#endif // RWS_STREAMHANDLER_H
