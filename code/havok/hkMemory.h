#ifndef HK_MEMORY_H
#define HK_MEMORY_H

// ============================================================================
// CannonCruise - Havok Physics 2.2.1 Stub (hkMemory.h)
// Binary: "Havok Allocation of " assert dizesi (HavokMemoryManager.cpp / FUN_00435adf)
// Havok 2.x bellek arayüzü. Oyun CHavokMemoryManager ile kendi tahsisini yapar;
// gerçek SDK entegrasyonunda bu arayüz hkMemory singleton'ına bağlanacak.
// ============================================================================

#include "hkBaseTypes.h"

class hkMemory
{
public:
    virtual ~hkMemory() {}

    /// Tahsis et (Havok 2.x: allocate)
    virtual void* HK_CALL allocate(size_t size) = 0;

    /// Serbest bırak
    virtual void HK_CALL deallocate(void* p) = 0;

    /// Yeniden boyutlandır
    virtual void* HK_CALL reallocate(void* p, size_t size) = 0;

    /// Global bellek sistemi başlat (HavokInit çağrılmadan önce)
    static void init();

    /// Global bellek sistemini kapat
    static void quit();

    /// Aktif bellek sistemi singleton'ı
    static hkMemory* getInstance();
};

#endif // HK_MEMORY_H
