// ============================================================================
// CannonCruise - RenderWare Studio Framework (clinearmemorymanager.cpp)
// Original path: D:\Projects\CannonCruisePC\code\framework\core\attributehandler\clinearmemorymanager.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_00414850 — CLinearMemoryManager::Instance()
//   FUN_00414930 — CLinearMemoryManager::~CLinearMemoryManager()
//   FUN_00414e60 — CLinearMemoryManager::Delete()
//   FUN_00414e80 — CLinearMemoryManager::Alloc(RwUInt32 size)
//   FUN_00415160 — CLinearMemoryManager::Free(void* pMem)
//   FUN_00415200 — CLinearMemoryManager::DestroyPool(MemoryPool* pPool)
//   FUN_004154a0 — CLinearMemoryManager::CreatePool(RwUInt32 poolSize)
// ============================================================================

#include "Allocation Policies/clinearmemorymanager.h"
#include <cassert>
#include <cstdio>
#include <cstdlib>

namespace RWS
{
    CLinearMemoryManager* CLinearMemoryManager::sm_pInstance = 0;
    CLinearMemoryManager::MemoryPool* CLinearMemoryManager::sm_pActivePool = 0;

    // ========================================================================
    // CLinearMemoryManager Constructor
    // ========================================================================
    CLinearMemoryManager::CLinearMemoryManager()
    {
    }

    // ========================================================================
    // CLinearMemoryManager Destructor (FUN_00414930)
    // ASM: 00414930 — Havuzları sil, serbest bırakılmamış bellek varsa uyar
    // ========================================================================
    CLinearMemoryManager::~CLinearMemoryManager()
    {
        for (std::vector<MemoryPool*>::iterator it = m_StoragePools.begin();
             it != m_StoragePools.end(); ++it)
        {
            MemoryPool* pPool = *it;
            if (pPool)
            {
                if (pPool->m_AllocCount > 0)
                {
                    // Assert/Warning: Deleting a memory pool with %d allocations not freed.
                    assert(pPool->m_AllocCount == 0 && "Deleting a memory pool with allocations not freed.");
                }
                DestroyPool(pPool);
            }
        }
        m_StoragePools.clear();
        sm_pActivePool = 0;
    }

    // ========================================================================
    // CLinearMemoryManager::Instance (FUN_00414850)
    // ASM: 00414850 — Singleton al / yoksa oluştur (DAT_0063567c)
    // ========================================================================
    CLinearMemoryManager* CLinearMemoryManager::Instance()
    {
        if (sm_pInstance == 0)
        {
            sm_pInstance = new CLinearMemoryManager();
        }
        return sm_pInstance;
    }

    // ========================================================================
    // CLinearMemoryManager::Delete (FUN_00414e60)
    // ASM: 00414e60 — Singleton nesnesini serbest bırak
    // ========================================================================
    void CLinearMemoryManager::Delete()
    {
        if (sm_pInstance != 0)
        {
            delete sm_pInstance;
            sm_pInstance = 0;
        }
    }

    // ========================================================================
    // CLinearMemoryManager::Alloc (FUN_00414e80)
    // ASM: 00414e80 — Doğrusal bellek havuzundan blok tahsis et
    // ========================================================================
    void* CLinearMemoryManager::Alloc(RwUInt32 size)
    {
        if (sm_pActivePool == 0)
        {
            return 0;
        }

        RwUInt8* pCurrent = (RwUInt8*)sm_pActivePool->m_pCurrent;
        RwUInt8* pEnd = (RwUInt8*)sm_pActivePool->m_pEnd;

        // Havuzda yeterli alan var mı?
        if (pCurrent + size > pEnd)
        {
            return 0;
        }

        // 64-byte hizalama kontrolü
        assert((size & (CClassFactory::m_AlignmentSize - 1)) == 0 &&
               "Size should always be a multiple of CClassFactory::m_AlignmentSize to ensure alignment. check the scope of the new operation ::RWS_NEW vs RWS_NEW.");

        void* pResult = pCurrent;
        sm_pActivePool->m_pCurrent = (void*)(pCurrent + size);
        sm_pActivePool->m_AllocCount++;

        return pResult;
    }

    // ========================================================================
    // CLinearMemoryManager::Free (FUN_00415160)
    // ASM: 00415160 — Blok serbest bırak, havuz boşalırsa yok et
    // ========================================================================
    RwBool CLinearMemoryManager::Free(void* pMem)
    {
        CLinearMemoryManager* pManager = Instance();

        for (std::vector<MemoryPool*>::iterator it = pManager->m_StoragePools.begin();
             it != pManager->m_StoragePools.end(); ++it)
        {
            MemoryPool* pPool = *it;
            if (pPool && pMem >= pPool->m_pAlignedBase && pMem < pPool->m_pEnd)
            {
                pPool->m_AllocCount--;
                if (pPool->m_AllocCount == 0)
                {
                    if (sm_pActivePool == pPool)
                    {
                        sm_pActivePool = 0;
                    }
                    DestroyPool(pPool);
                    pManager->m_StoragePools.erase(it);
                }
                return TRUE;
            }
        }

        return FALSE;
    }

    // ========================================================================
    // CLinearMemoryManager::DestroyPool (FUN_00415200)
    // ASM: 00415200 — Havuz belleğini serbest bırak
    // ========================================================================
    void CLinearMemoryManager::DestroyPool(MemoryPool* pPool)
    {
        if (pPool)
        {
            if (pPool->m_pBase)
            {
                free(pPool->m_pBase);
                pPool->m_pBase = 0;
            }
            delete pPool;
        }
    }

    // ========================================================================
    // CLinearMemoryManager::CreatePool (FUN_004154a0)
    // ASM: 004154a0 — Yeni bir bellek havuzu oluştur (64-byte hizalanmış)
    // ========================================================================
    void CLinearMemoryManager::CreatePool(RwUInt32 poolSize)
    {
        MemoryPool* pPool = new MemoryPool();
        
        // Hizalama için ekstra 64 byte tahsis et
        pPool->m_pBase = malloc(poolSize + CClassFactory::m_AlignmentSize);
        
        // 64-byte hizalama hesapla
        RwUInt32 baseAddr = (RwUInt32)pPool->m_pBase;
        RwUInt32 alignedAddr = baseAddr;
        RwUInt32 misaligned = baseAddr & (CClassFactory::m_AlignmentSize - 1);
        if (misaligned != 0)
        {
            alignedAddr = (baseAddr - misaligned) + CClassFactory::m_AlignmentSize;
        }

        assert((alignedAddr & (CClassFactory::m_AlignmentSize - 1)) == 0 &&
               "CLinearMemoryManager: Storage not aligned to CClassFactory::m_AlignmentSize.");

        pPool->m_pAlignedBase = (void*)alignedAddr;
        pPool->m_pCurrent = (void*)alignedAddr;
        pPool->m_pEnd = (void*)(alignedAddr + poolSize);
        pPool->m_AllocCount = 0;

        CLinearMemoryManager* pManager = Instance();
        pManager->m_StoragePools.push_back(pPool);

        assert(pManager->m_StoragePools.size() <= 2 && "Instance()->m_StoragePools.size() <= 2 Failed");

        sm_pActivePool = pPool;
    }
}
