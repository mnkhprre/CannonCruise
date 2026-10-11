#ifndef RWS_CLUMPHELPER_H
#define RWS_CLUMPHELPER_H

// ============================================================================
// CannonCruise - RenderWare Studio Framework (clumphelper.h)
// Original path: D:\Projects\CannonCruisePC\code\framework\toolkits\world\helpers\clumphelper.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// Binary ref: FUN_00408400 - FUN_00408680 (RWS::ClumpHelper)
// ============================================================================

#include <rwcore.h>
#include <rpworld.h>

namespace RWS
{
    /**
     * @class ClumpHelper
     * @brief RpClump nesneleri için yardımcı araç sınıfı.
     *
     * CannonCruise'da her 3D nesne (gemi, asker, top mermisi vb.) bir `RpClump`
     * içinde barındırılır. `ClumpHelper`, bu nesnelerin ortak işlemlerini (arama,
     * klonlama, yok etme, bounding sphere) merkezi bir API ile sunar.
     *
     * **Binary referansları:**
     * - FindFirstAtomic: `FUN_00408400`
     * - CalculateBoundingSphere: `FUN_00408480`
     * - Clone: `FUN_004084f0`
     * - Destroy: `FUN_00408560`
     */
    class ClumpHelper
    {
    public:
        /**
         * @brief Clump içindeki ilk `RpAtomic` nesnesini bulur.
         * @param pClump Arama yapılacak clump.
         * @return RpAtomic* — İlk atomic; bulunamazsa nullptr.
         */
        static RpAtomic* FindFirstAtomic(RpClump* pClump);

        /**
         * @brief Tüm atomiklere belirtilen render bayrağını uygular.
         * @param pClump Hedef clump.
         * @param flags  Uygulanacak `rpATOMICRENDER` vb. bayraklar.
         */
        static void SetAtomicFlags(RpClump* pClump, RwUInt32 flags);

        /**
         * @brief Clump'un tüm geometrisini kapsayan küreyi hesaplar.
         * @param pClump  Hedef clump.
         * @param pSphere Hesaplanan kürenin yazılacağı çıkış parametresi.
         */
        static void CalculateBoundingSphere(RpClump* pClump, RwSphere* pSphere);

        /**
         * @brief Clump'u klonlar (derin kopya).
         * @param pClump Klonlanacak kaynak clump.
         * @return RpClump* — Yeni klonlanmış clump; başarısızlıkta nullptr.
         */
        static RpClump* Clone(RpClump* pClump);

        /**
         * @brief Clump'u ve bağlı tüm kaynakları yok eder.
         * @param pClump Yok edilecek clump.
         */
        static void Destroy(RpClump* pClump);

        /// @brief `Destroy()` için uyumluluk sarmalayıcısı
        static void DestroyClump(RpClump* pClump) { Destroy(pClump); }

        /**
         * @brief Ham kaynak verisinden clump oluşturur (stub).
         * @return RpClump* — CannonCruise'da her zaman nullptr.
         */
        static RpClump* CreateClumpFromResource(const void* pResourceData) { return 0; }

        /**
         * @brief Clump'a özellik paketi uygular (stub).
         * @param pClump  Hedef clump.
         * @param packet  Uygulanacak özellik paketi.
         */
        static void HandleAttributes(RpClump* pClump, const class CAttributePacket& packet) {}
    };
}



#endif // RWS_CLUMPHELPER_H
