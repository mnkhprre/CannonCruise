#ifndef RWS_MEMORYHANDLER_H
#define RWS_MEMORYHANDLER_H

// ============================================================================
// CannonCruise - RenderWare Studio Framework (memoryhandler.h)
// Original path: D:\Projects\CannonCruisePC\code\framework\core\memoryhandler\memoryhandler.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include <rwcore.h>

namespace RWS
{
    class CMemoryHandler
    {
    public:
        // RenderWare bellek yönetim fonksiyonları
        static void* RwAlloc(RwFreeList* fl, RwUInt32 size, RwUInt32 hint);
        static void* RwFree(RwFreeList* fl, void* mem);
        static void* RwRealloc(RwFreeList* fl, void* mem, RwUInt32 size, RwUInt32 hint);
        static void* RwCalloc(RwFreeList* fl, RwUInt32 numObj, RwUInt32 sizeObj, RwUInt32 hint);

        // Başlatma ve kapatma
        static RwBool Initialize();
        static void Shutdown();

        // RenderWare motoru için bellek fonksiyon tablosunu döndür
        static RwMemoryFunctions* GetMemoryFunctions();
    };
}

#endif // RWS_MEMORYHANDLER_H
