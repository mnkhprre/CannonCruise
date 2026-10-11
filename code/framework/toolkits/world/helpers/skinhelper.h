#ifndef RWS_SKINHELPER_H
#define RWS_SKINHELPER_H

// ============================================================================
// CannonCruise - RenderWare Studio Framework (skinhelper.h)
// Original path: D:\Projects\CannonCruisePC\code\framework\toolkits\world\helpers\skinhelper.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// Binary ref: FUN_00408c00 - FUN_00408d40 (RWS::SkinHelper)
// ============================================================================

#include <rwcore.h>
#include <rpworld.h>
#include <rpskin.h>
#include <rphanim.h>

namespace RWS
{
    /**
     * @class SkinHelper
     * @brief RpSkin ve HAnim hiyerarşisi için yardımcı sınıf.
     *
     * Asker, kuş gibi iskelet animasyonlu karakterler `RpSkin` plugin'ini
     * kullanan atomikler içerir. `SkinHelper`, bu atomikleri bulur ve
     * `RpHAnimHierarchy` animasyon hiyerarşisini bağlar.
     *
     * **Kullanım örneği:**
     * ```cpp
     * RpAtomic* pSkinAtomic = SkinHelper::FindSkinAtomic(pClump);
     * RpHAnimHierarchy* pHier = SkinHelper::GetHierarchy(pSkinAtomic);
     * SkinHelper::SetHierarchy(pSkinAtomic, pHier);
     * ```
     *
     * **Binary referansları:**
     * - FindSkinAtomic: `FUN_00408c00`
     * - GetHierarchy: `FUN_00408c60`
     * - SetHierarchy: `FUN_00408cc0`
     */
    class SkinHelper
    {
    public:
        /**
         * @brief Clump içindeki RpSkin plugin'ine sahip atomici bulur.
         * @param pClump Arama yapılacak clump.
         * @return RpAtomic* — Skin atomici; bulunamazsa nullptr.
         */
        static RpAtomic* FindSkinAtomic(RpClump* pClump);

        /**
         * @brief Atomice bağlı HAnim hiyerarşisini döndürür.
         * @param pAtomic Hedef skin atomici.
         * @return RpHAnimHierarchy* — Hiyerarşi; yoksa nullptr.
         */
        static RpHAnimHierarchy* GetHierarchy(RpAtomic* pAtomic);

        /**
         * @brief HAnim hiyerarşisini atomice bağlar.
         * @param pAtomic     Hedef skin atomici.
         * @param pHierarchy  Bağlanacak hiyerarşi.
         * @return TRUE ise bağlama başarılı.
         */
        static RwBool SetHierarchy(RpAtomic* pAtomic, RpHAnimHierarchy* pHierarchy);
    };
}

#endif // RWS_SKINHELPER_H
