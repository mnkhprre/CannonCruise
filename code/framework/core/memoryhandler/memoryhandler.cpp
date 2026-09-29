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
#endif

namespace RWS
{
    static RwMemoryFunctions s_RwMemoryFunctions;
    static HANDLE s_hCustomHeap = 0;
    static void* s_piCurrentMemoryTracker = 0;

    // ========================================================================
    // CMemoryHandler::RwAlloc (FUN_004049f0)
    // ASM: 004049f0 — RenderWare bellek tahsisi
    // ========================================================================
    void* CMemoryHandler::RwAlloc(RwFreeList* fl, RwUInt32 size, RwUInt32 hint)
    {
        void* pMem = 0;
#ifdef _WIN32
        if (s_hCustomHeap)
        {
            pMem = HeapAlloc(s_hCustomHeap, 0, size);
        }
        else
        {
            pMem = malloc(size);
        }
#else
        pMem = malloc(size);
#endif
        assert(pMem != 0 && "HeapAlloc failed");
        return pMem;
    }

    // ========================================================================
    // CMemoryHandler::RwFree (FUN_00404c80)
    // ASM: 00404c80 — RenderWare bellek serbest bırakma
    // ========================================================================
    void* CMemoryHandler::RwFree(RwFreeList* fl, void* mem)
    {
        if (mem != 0)
        {
#ifdef _WIN32
            if (s_hCustomHeap)
            {
                HeapFree(s_hCustomHeap, 0, mem);
            }
            else
            {
                free(mem);
            }
#else
            free(mem);
#endif
        }
        return 0;
    }

    // ========================================================================
    // CMemoryHandler::RwRealloc (FUN_00404f80)
    // ASM: 00404f80 — RenderWare bellek yeniden boyutlandırma
    // ========================================================================
    void* CMemoryHandler::RwRealloc(RwFreeList* fl, void* mem, RwUInt32 size, RwUInt32 hint)
    {
        assert(mem != 0 && "pMem != NULL Failed");
        void* pNewMem = 0;
#ifdef _WIN32
        if (s_hCustomHeap)
        {
            pNewMem = HeapReAlloc(s_hCustomHeap, 0, mem, size);
        }
        else
        {
            pNewMem = realloc(mem, size);
        }
#else
        pNewMem = realloc(mem, size);
#endif
        assert(pNewMem != 0 && "HeapAlloc failed");
        return pNewMem;
    }

    // ========================================================================
    // CMemoryHandler::RwCalloc (FUN_00405140)
    // ASM: 00405140 — Sıfırlanmış blok tahsisi
    // ========================================================================
    void* CMemoryHandler::RwCalloc(RwFreeList* fl, RwUInt32 numObj, RwUInt32 sizeObj, RwUInt32 hint)
    {
        RwUInt32 totalSize = numObj * sizeObj;
        void* pMem = RwAlloc(fl, totalSize, hint);
        if (pMem)
        {
            memset(pMem, 0, totalSize);
        }
        return pMem;
    }

    // ========================================================================
    // CMemoryHandler::Initialize / Shutdown
    // ========================================================================
    RwBool CMemoryHandler::Initialize()
    {
        s_RwMemoryFunctions.rwmalloc = RwAlloc;
        s_RwMemoryFunctions.rwfree = RwFree;
        s_RwMemoryFunctions.rwrealloc = RwRealloc;
        s_RwMemoryFunctions.rwcalloc = RwCalloc;

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
