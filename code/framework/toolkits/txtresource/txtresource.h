#ifndef RWS_TXTRESOURCE_H
#define RWS_TXTRESOURCE_H

// ============================================================================
// CannonCruise - RenderWare Studio Framework (txtresource.h)
// Original path: D:\Projects\CannonCruisePC\code\framework\toolkits\txtresource\txtresource.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include <rwcore.h>

namespace RWS
{
    class TxtResource
    {
    public:
        // Metin kaynağı yükleme & serbest bırakma
        static char* Load(const char* pFilename);
        static void Unload(char* pTextData);
    };
}

#endif // RWS_TXTRESOURCE_H
