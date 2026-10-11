#ifndef RWS_ANIMHELPER_H
#define RWS_ANIMHELPER_H

// ============================================================================
// CannonCruise - RenderWare Studio Framework (AnimHelper.h)
// Original path: D:\Projects\CannonCruisePC\code\framework\toolkits\Animation\AnimHelper.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// Binary ref: FUN_00409000 - FUN_004091a0 (RWS::AnimHelper)
// ============================================================================

#include <rwcore.h>
#include <rpworld.h>
#include <rphanim.h>
#include <rtanim.h>

namespace RWS
{
    /**
     * @class AnimHelper
     * @brief RenderWare HAnim animasyon hiyerarşileri için yardımcı araç sınıfı.
     *
     * `CAnimController` tarafından kullanılarak 3D karakter animasyonlarının
     * (yürüme, ateş etme vb.) oynatılması, güncellenmesi ve geçişlerinin (blend)
     * yönetilmesini sağlar.
     *
     * **Binary referansları:**
     * - UpdateHierarchy: `FUN_00409000`
     * - SetCurrentAnim: `FUN_00409060`
     * - BlendHierarchy: `FUN_004090d0`
     * - UpdateHierarchyMatrices: `FUN_00409140`
     */
    class AnimHelper
    {
    public:
        /**
         * @brief Animasyon hiyerarşisinin süresini ilerletir.
         * @param pHierarchy Güncellenecek HAnim hiyerarşisi.
         * @param deltaTime  Geçen zaman (saniye).
         */
        static void UpdateHierarchy(RpHAnimHierarchy* pHierarchy, RwReal deltaTime);

        /**
         * @brief Hiyerarşi için geçerli animasyonu ayarlar.
         * @param pHierarchy Hedef HAnim hiyerarşisi.
         * @param pAnim      Oynatılacak RenderWare animasyon nesnesi.
         */
        static void SetCurrentAnim(RpHAnimHierarchy* pHierarchy, RtAnimAnimation* pAnim);

        /**
         * @brief İki animasyon hiyerarşisi arasında geçiş (blend) yapar.
         * @param pDestHierarchy Hedef (çıkış) hiyerarşi.
         * @param pSrcHierarchy  Kaynak (geçiş yapılacak) hiyerarşi.
         * @param blendAlpha     Geçiş ağırlığı (0.0 - 1.0 aralığında).
         */
        static void BlendHierarchy(RpHAnimHierarchy* pDestHierarchy, RpHAnimHierarchy* pSrcHierarchy, RwReal blendAlpha);

        /**
         * @brief Hiyerarşideki kemik matrislerini günceller.
         * @param pHierarchy Hedef HAnim hiyerarşisi.
         *
         * Bu işlem genellikle `UpdateHierarchy` veya `BlendHierarchy`
         * sonrasında, matrislerin dünya koordinatlarına çevrilmesi için çağrılır.
         */
        static void UpdateHierarchyMatrices(RpHAnimHierarchy* pHierarchy);
    };
}

#endif // RWS_ANIMHELPER_H

