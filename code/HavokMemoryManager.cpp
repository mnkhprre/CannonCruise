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
#include <cstdint>
#include <limits>

void* CHavokMemoryManager::Allocate(RwUInt32 size, RwUInt32 alignment)
{
    assert(alignment == 16 && "Unalligned alloc");

    // Only the existing 16-byte contract is supported, including release builds.
    if (alignment != 16) return 0;
    const size_t padding = static_cast<size_t>(alignment) - 1 + sizeof(void*);
    if (static_cast<size_t>(size) > std::numeric_limits<size_t>::max() - padding)
        return 0;

    void* pRaw = malloc(static_cast<size_t>(size) + padding);
    if (!pRaw)
    {
        assert(false && "Havok Allocation of bytes failed");
        return 0;
    }

    // Widen BEFORE complementing: a 32-bit mask clears high pointer bits on x64.
    const std::uintptr_t mask = static_cast<std::uintptr_t>(alignment) - 1;
    const std::uintptr_t rawAddr = reinterpret_cast<std::uintptr_t>(pRaw) + sizeof(void*);
    const std::uintptr_t alignedAddr = (rawAddr + mask) & ~mask;
    void** pStoredRaw = reinterpret_cast<void**>(alignedAddr - sizeof(void*));
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
