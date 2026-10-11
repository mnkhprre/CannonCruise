#ifndef RWS_CLINEARMEMORYMANAGER_H
#define RWS_CLINEARMEMORYMANAGER_H

// ============================================================================
// CannonCruise - RenderWare Studio Framework (clinearmemorymanager.h)
// Original path: D:\Projects\CannonCruisePC\code\framework\core\attributehandler\Allocation Policies\..\clinearmemorymanager.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// Binary ref: FUN_004053f0 - FUN_00405600 (RWS::CLinearMemoryManager)
// ============================================================================

#include <rwcore.h>
#include <vector>
#include "../cclassfactory.h"

namespace RWS
{
    /**
     * @class CLinearMemoryManager
     * @brief Lineer (blok) tabanlı özel bellek yöneticisi.
     *
     * Oyun süresince sık tahsis edilen geçici veya aynı yaşam döngüsüne sahip
     * nesnelerin tek bir bloktan ayrılması (allocation) ve tüm havuzun bir defada
     * serbest bırakılması (deallocation) amacıyla kullanılır. Bu sayede
     * bellek parçalanması önlenir ve hız artar.
     *
     * **Binary referansları:**
     * - Alloc: `FUN_004054a0`
     * - Free: `FUN_00405520`
     * - CreatePool: `FUN_00405580`
     * - DestroyPool: `FUN_004055e0`
     */
    class CLinearMemoryManager
    {
    public:
        /**
         * @struct MemoryPool
         * @brief Tahsis edilmiş bir bellek havuzunu tanımlayan yapı.
         */
        struct MemoryPool
        {
            void* m_pBase;         ///< Temel bellek bloğu (malloc çıkışı)
            void* m_pAlignedBase;  ///< 64-byte hizalanmış başlangıç (alignment)
            void* m_pCurrent;      ///< Bir sonraki boş konum (geçerli tahsis sınırı)
            void* m_pEnd;          ///< Havuz sonu (pAlignedBase + poolSize)
            RwUInt32 m_AllocCount; ///< Havuzdaki aktif tahsis sayısı
        };

        /// @return Tekil CLinearMemoryManager örneğine işaretçi
        static CLinearMemoryManager* Instance();
        /// @brief Yöneticiyi ve tüm havuzları yok eder
        static void Delete();

        /**
         * @brief Aktif bellek havuzundan blok tahsis eder.
         * @param size Tahsis edilecek bayt sayısı.
         * @return void* — Tahsis edilen bloğa işaretçi (havuz doluysa nullptr).
         */
        static void* Alloc(RwUInt32 size);

        /**
         * @brief Havuzdan tahsis edilmiş belleği serbest bırakır (sayaç düşürür).
         * @param pMem Serbest bırakılacak bellek bloğu.
         * @return TRUE ise başarıyla bırakıldı.
         *
         * Not: Gerçek bellek serbest bırakma işlemi havuz yok edilirken gerçekleşir.
         */
        static RwBool Free(void* pMem);

        /**
         * @brief Yeni bir bellek havuzu oluşturur ve aktif havuz yapar.
         * @param poolSize Havuzun toplam boyutu (bayt).
         */
        static void CreatePool(RwUInt32 poolSize);

        /**
         * @brief Belirtilen bellek havuzunu tamamen yok eder.
         * @param pPool Yok edilecek havuz.
         */
        static void DestroyPool(MemoryPool* pPool);

    private:
        CLinearMemoryManager();
        ~CLinearMemoryManager();

        static CLinearMemoryManager* sm_pInstance; ///< Tekil örnek işaretçisi
        static MemoryPool* sm_pActivePool;         ///< Şu anda tahsis yapılan havuz

        std::vector<MemoryPool*> m_StoragePools;   ///< Tüm havuzların listesi
    };
}

#endif // RWS_CLINEARMEMORYMANAGER_H

