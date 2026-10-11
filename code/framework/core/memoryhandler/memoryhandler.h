#ifndef RWS_MEMORYHANDLER_H
#define RWS_MEMORYHANDLER_H

// ============================================================================
// CannonCruise - RenderWare Studio Framework (memoryhandler.h)
// Original path: D:\Projects\CannonCruisePC\code\framework\core\memoryhandler\memoryhandler.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// Binary ref: FUN_00406000 - FUN_00406280 (RWS::CMemoryHandler)
// ============================================================================

#include <rwcore.h>

namespace RWS
{
    /**
     * @class CMemoryHandler
     * @brief RenderWare motorunun bellek tahsis/serbest bırakma işlemlerini yöneten sınıf.
     *
     * RenderWare, kendi bellek işlevlerini (`rwFreeList` tabanlı) dışarıdan
     * enjekte edilebilir fonksiyon işaretçileri aracılığıyla kullanır.
     * `CMemoryHandler`, bu işaretçileri sistem heap'ine veya özel bir havuza
     * yönlendirerek RenderWare'in bellek tahsisini kontrol altında tutar.
     *
     * **Başlatma:**
     * ```cpp
     * CMemoryHandler::Initialize();  // RwEngineInit'ten ÖNCE çağrılmalıdır
     * RwEngineInit(nullptr, 0, CMemoryHandler::GetMemoryFunctions());
     * ```
     *
     * **Binary referansları:**
     * - Initialize: `FUN_00406000`
     * - Shutdown: `FUN_00406060`
     * - RwAlloc: `FUN_004060c0`
     * - RwFree: `FUN_00406100`
     * - RwRealloc: `FUN_00406140`
     * - RwCalloc: `FUN_00406190`
     * - GetMemoryFunctions: `FUN_004061f0`
     */
    class CMemoryHandler
    {
    public:
        /**
         * @brief RenderWare serbest liste havuzundan bellek tahsis eder.
         * @param fl   RenderWare serbest liste (nullptr ise system malloc kullanılır).
         * @param size Tahsis edilecek bayt sayısı.
         * @param hint Hizalama veya tahsis ipucu bayrakları.
         * @return void* — Tahsis edilen bellek bloğuna işaretçi.
         */
        static void* RwAlloc(RwFreeList* fl, RwUInt32 size, RwUInt32 hint);

        /**
         * @brief `RwAlloc()` ile tahsis edilmiş belleği serbest bırakır.
         * @param fl  RenderWare serbest liste.
         * @param mem Serbest bırakılacak bellek bloğu.
         * @return void* — Her zaman nullptr.
         */
        static void* RwFree(RwFreeList* fl, void* mem);

        /**
         * @brief Mevcut bellek bloğunu yeniden boyutlandırır.
         * @param fl   RenderWare serbest liste.
         * @param mem  Yeniden boyutlandırılacak blok.
         * @param size Yeni boyut (bayt).
         * @param hint Tahsis ipucu bayrakları.
         * @return void* — Yeniden boyutlandırılmış blok işaretçisi.
         */
        static void* RwRealloc(RwFreeList* fl, void* mem, RwUInt32 size, RwUInt32 hint);

        /**
         * @brief Sıfırlanmış bellek bloğu tahsis eder.
         * @param fl      RenderWare serbest liste.
         * @param numObj  Nesne sayısı.
         * @param sizeObj Her nesnenin boyutu (bayt).
         * @param hint    Tahsis ipucu bayrakları.
         * @return void* — Sıfırlanmış bellek bloğuna işaretçi.
         */
        static void* RwCalloc(RwFreeList* fl, RwUInt32 numObj, RwUInt32 sizeObj, RwUInt32 hint);

        /**
         * @brief Bellek işleyicisini başlatır ve RenderWare bellek fonksiyonlarını kaydeder.
         * @return TRUE ise başlatma başarılı.
         */
        static RwBool Initialize();

        /**
         * @brief Bellek işleyicisini kapatır.
         */
        static void Shutdown();

        /**
         * @brief RenderWare motoru için bellek fonksiyon tablosunu döndürür.
         * @return RwMemoryFunctions* — `RwEngineInit()` çağrısına geçirilecek yapıya işaretçi.
         */
        static RwMemoryFunctions* GetMemoryFunctions();
    };
}

#endif // RWS_MEMORYHANDLER_H
