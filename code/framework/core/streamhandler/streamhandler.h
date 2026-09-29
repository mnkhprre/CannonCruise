#ifndef RWS_STREAMHANDLER_H
#define RWS_STREAMHANDLER_H

// ============================================================================
// CannonCruise - RenderWare Studio Framework (streamhandler.h)
// Original path: D:\Projects\CannonCruisePC\code\framework\core\streamhandler\streamhandler.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include <rwcore.h>
#include <map>

namespace RWS
{
    typedef RwBool (*ChunkHandlerFunc)(RwStream* pStream, RwUInt32 chunkId, RwUInt32 chunkSize);

    class CStreamHandler
    {
    public:
        static CStreamHandler* Instance();
        static void Initialize();
        static void Shutdown();

        // Chunk işleyicisi kaydı
        static RwBool RegisterChunkHandler(RwUInt32 chunkId, ChunkHandlerFunc pHandler);
        static void UnRegisterChunkHandler(RwUInt32 chunkId);

        // Akış okuma & işleme
        static RwBool ProcessStream(RwStream* pStream);
        static RwBool ProcessFile(const char* pFilename);

    private:
        CStreamHandler();
        ~CStreamHandler();

        static CStreamHandler* sm_pInstance;

        std::map<RwUInt32, ChunkHandlerFunc> m_ChunkHandlers;
    };
}

#endif // RWS_STREAMHANDLER_H
