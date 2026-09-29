// ============================================================================
// CannonCruise - Havok Physics Memory Manager (HavokMemoryManager.cpp)
// Original path: D:\Projects\CannonCruisePC\code\HavokMemoryManager.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_00435adf — CHavokMemoryManager::Allocate
//   FUN_00435eaf — CHavokMemoryManager::Free
// ============================================================================

#include "HavokMemoryManager.h"
#include <cassert>
#include <cstdlib>

void* CHavokMemoryManager::Allocate(RwUInt32 size, RwUInt32 alignment)
{
    assert(alignment == 16 && "Unalligned alloc");

    // 16-byte hizalama ile tahsis
    void* pRaw = malloc(size + alignment + sizeof(void*));
    if (!pRaw)
    {
        assert(false && "Havok Allocation of bytes failed");
        return 0;
    }

    size_t rawAddr = (size_t)pRaw + sizeof(void*);
    size_t alignedAddr = (rawAddr + (alignment - 1)) & ~(alignment - 1);
    void** pStoredRaw = (void**)(alignedAddr - sizeof(void*));
    *pStoredRaw = pRaw;

    return (void*)alignedAddr;
}

void CHavokMemoryManager::Free(void* pMem)
{
    if (!pMem) return;

    void** pStoredRaw = (void**)((size_t)pMem - sizeof(void*));
    void* pRaw = *pStoredRaw;
    free(pRaw);
}

void CHavokMemoryManager::Initialize()
{
}

void CHavokMemoryManager::Shutdown()
{
}
