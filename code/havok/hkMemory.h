#ifndef HK_MEMORY_H
#define HK_MEMORY_H

// ============================================================================
// CannonCruise - Havok Physics 2.2.1 Shim (hkMemory.h)
// Binary: "Havok Allocation of " assert dizesi (HavokMemoryManager.cpp / FUN_00435adf)
// Havok 2.x memory interface. Game CHavokMemoryManager does its own allocation;
// SHIM: init/quit/getInstance in havokimpl.cpp bind to malloc/realloc/free
// (game calls them; hkMemory::getInstance() returns singleton).
// ============================================================================

#include "hkBaseTypes.h"

class hkMemory
{
public:
    virtual ~hkMemory() {}

    /// Allocate (Havok 2.x: allocate)
    virtual void* HK_CALL allocate(size_t size) = 0;

    /// Deallocate
    virtual void HK_CALL deallocate(void* p) = 0;

    /// Reallocate
    virtual void* HK_CALL reallocate(void* p, size_t size) = 0;

    /// Initialize global memory system (before HavokInit)
    static void init();

    /// Shut down global memory system
    static void quit();

    /// Active memory system singleton
    static hkMemory* getInstance();
};

#endif // HK_MEMORY_H
