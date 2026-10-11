// ============================================================================
// CannonCruise - RenderWare Studio Framework (memoryhandler.cpp)
// Original path: D:\Projects\CannonCruisePC\code\framework\core\memoryhandler\memoryhandler.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_004049f0 — CMemoryHandler::RwAlloc
//   FUN_00404c80 — CMemoryHandler::RwFree
//   FUN_00404f80 — CMemoryHandler::RwRealloc
//   FUN_00405140 — CMemoryHandler::RwCalloc
// ============================================================================

#include "memoryhandler.h"
#include <cassert>
#include <cstdlib>
#include <cstring>

#ifdef _WIN32
#include <windows.h>
#else
typedef void* HANDLE;
#endif

namespace RWS
{
    static RwMemoryFunctions s_RwMemoryFunctions;
    static HANDLE s_hCustomHeap = 0;
    static void* s_piCurrentMemoryTracker = 0;

    // Internal wrapper functions matching RwMemoryFunctions signature
    static void* InternalMalloc(size_t size)
    {
        void* pMem = malloc(size);
        assert(pMem != 0 && "malloc failed");
        return pMem;
    }

    static void InternalFree(void* mem)
    {
        if (mem != 0)
        {
            free(mem);
        }
    }

    static void* InternalRealloc(void* mem, size_t size)
    {
        assert(mem != 0 && "pMem != NULL Failed");
        void* pNewMem = realloc(mem, size);
        assert(pNewMem != 0 && "realloc failed");
        return pNewMem;
    }

    static void* InternalCalloc(size_t numObj, size_t sizeObj)
    {
        size_t totalSize = numObj * sizeObj;
        void* pMem = InternalMalloc(totalSize);
        if (pMem)
        {
            memset(pMem, 0, totalSize);
        }
        return pMem;
    }

    // ========================================================================
    // CMemoryHandler::RwAlloc (FUN_004049f0)
    // ASM: 004049f0 — RenderWare bellek tahsisi
    // ========================================================================
    void* CMemoryHandler::RwAlloc(RwFreeList* fl, RwUInt32 size, RwUInt32 hint)
    {
        (void)fl; (void)hint; // unused in stub
        return InternalMalloc(size);
    }

    // ========================================================================
    // CMemoryHandler::RwFree (FUN_00404c80)
    // ASM: 00404c80 — RenderWare bellek serbest bırakma
    // ========================================================================
    void* CMemoryHandler::RwFree(RwFreeList* fl, void* mem)
    {
        (void)fl; // unused in stub
        InternalFree(mem);
        return 0;
    }

    // ========================================================================
    // CMemoryHandler::RwRealloc (FUN_00404f80)
    // ASM: 00404f80 — RenderWare bellek yeniden boyutlandırma
    // ========================================================================
    void* CMemoryHandler::RwRealloc(RwFreeList* fl, void* mem, RwUInt32 size, RwUInt32 hint)
    {
        (void)fl; (void)hint; // unused in stub
        return InternalRealloc(mem, size);
    }

    // ========================================================================
    // CMemoryHandler::RwCalloc (FUN_00405140)
    // ASM: 00405140 — Sıfırlanmış blok tahsisi
    // ========================================================================
    void* CMemoryHandler::RwCalloc(RwFreeList* fl, RwUInt32 numObj, RwUInt32 sizeObj, RwUInt32 hint)
    {
        (void)fl; (void)hint; // unused in stub
        return InternalCalloc(numObj, sizeObj);
    }

    // ========================================================================
    // CMemoryHandler::Initialize / Shutdown
    // ========================================================================
    RwBool CMemoryHandler::Initialize()
    {
        s_RwMemoryFunctions.rwmalloc = InternalMalloc;
        s_RwMemoryFunctions.rwfree = InternalFree;
        s_RwMemoryFunctions.rwrealloc = InternalRealloc;
        s_RwMemoryFunctions.rwcalloc = InternalCalloc;

#ifdef _WIN32
        s_hCustomHeap = GetProcessHeap();
#endif
        return TRUE;
    }

    void CMemoryHandler::Shutdown()
    {
        s_hCustomHeap = 0;
    }

    RwMemoryFunctions* CMemoryHandler::GetMemoryFunctions()
    {
        return &s_RwMemoryFunctions;
    }
}
