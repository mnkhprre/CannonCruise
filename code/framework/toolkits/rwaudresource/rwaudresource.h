#ifndef RWS_RWAUDRESOURCE_H
#define RWS_RWAUDRESOURCE_H

// ============================================================================
// CannonCruise - RenderWare Studio Framework (rwaudresource.h)
// Original path: D:\Projects\CannonCruisePC\code\framework\toolkits\rwaudresource\rwaudresource.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include <rwcore.h>

namespace RWS
{
    class RwAudResource
    {
    public:
        // Ses dalga sözlüğü (Wave Dictionary) yükleme ve temizleme
        static void* LoadWaveDict(const char* pFilename);
        static void UnloadWaveDict(void* pWaveDict);
    };
}

#endif // RWS_RWAUDRESOURCE_H
