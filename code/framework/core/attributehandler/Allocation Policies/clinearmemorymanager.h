#ifndef RWS_CLINEARMEMORYMANAGER_H
#define RWS_CLINEARMEMORYMANAGER_H

// ============================================================================
// CannonCruise - RenderWare Studio Framework (clinearmemorymanager.h)
// Original path: D:\Projects\CannonCruisePC\code\framework\core\attributehandler\Allocation Policies\..\clinearmemorymanager.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include <rwcore.h>
#include <vector>
#include "../cclassfactory.h"

namespace RWS
{
    class CLinearMemoryManager
    {
    public:
        struct MemoryPool
        {
            void* m_pBase;         // Temel bellek bloğu (FUN_004053f0 / malloc)
            void* m_pAlignedBase;  // 64-byte hizalanmış başlangıç
            void* m_pCurrent;      // Geçerli tahsis konumu
            void* m_pEnd;          // Havuz sonu (pAlignedBase + poolSize)
            RwUInt32 m_AllocCount; // Havuzdaki aktif tahsis sayısı
        };

        static CLinearMemoryManager* Instance();
        static void Delete();

        static void* Alloc(RwUInt32 size);
        static RwBool Free(void* pMem);
        static void CreatePool(RwUInt32 poolSize);
        static void DestroyPool(MemoryPool* pPool);

    private:
        CLinearMemoryManager();
        ~CLinearMemoryManager();

        static CLinearMemoryManager* sm_pInstance;
        static MemoryPool* sm_pActivePool;

        std::vector<MemoryPool*> m_StoragePools;
    };
}

#endif // RWS_CLINEARMEMORYMANAGER_H
