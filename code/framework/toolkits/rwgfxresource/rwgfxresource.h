#ifndef RWS_RWGFXRESOURCE_H
#define RWS_RWGFXRESOURCE_H

// ============================================================================
// CannonCruise - RenderWare Studio Framework (rwgfxresource.h)
// Original path: D:\Projects\CannonCruisePC\code\framework\toolkits\rwgfxresource\rwgfxresource.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include <rwcore.h>
#include <rpworld.h>

namespace RWS
{
    class RwGfxResource
    {
    public:
        // RenderWare grafik kaynak yükleyici ve kayıt
        static void* Load(const char* pFilename, const char* pType);
        static void Unload(void* pResource, const char* pType);

        // Grafik kaynak işleyicilerini CStreamHandler / CResourceManager'a bağla
        static void RegisterHandlers();
    };
}

#endif // RWS_RWGFXRESOURCE_H
