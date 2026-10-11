#ifndef RWS_SKINSAFE_H
#define RWS_SKINSAFE_H

// ============================================================================
// CannonCruise - RenderWare Studio Framework (SkinSafe.h)
// Original path: D:\Projects\CannonCruisePC\code\framework\toolkits\Animation\SkinSafe.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// Binary ref: FUN_00409200 - FUN_004092f0 (RWS::SkinSafe)
// ============================================================================

#include <rwcore.h>
#include <rpworld.h>
#include <rpskin.h>
#include <rphanim.h>

namespace RWS
{
    /**
     * @class SkinSafe
     * @brief İskelet animasyonlarında güvenli matris işlemleri sağlayan sınıf.
     *
     * Animasyon verisinin geçerli olmadığı veya varsayılan T-poza
     * dönülmesi gerektiği durumlarda (hata toleransı) kullanılır.
     *
     * **Binary referansları:**
     * - CopyBoneMatrices: `FUN_00409200`
     * - SetDefaultPose: `FUN_00409260`
     * - HierarchyCanAcceptDefaultPose: `FUN_004092b0`
     */
    class SkinSafe
    {
    public:
        /**
         * @brief Kemik matrislerini güvenli bir şekilde kopyalar.
         * @param pDstMatrices Hedef matris dizisi.
         * @param pSrcMatrices Kaynak matris dizisi.
         * @param numBones     Kopyalanacak kemik (matris) sayısı.
         */
        static void CopyBoneMatrices(RwMatrix* pDstMatrices, const RwMatrix* pSrcMatrices, RwUInt32 numBones);

        /**
         * @brief Hiyerarşiyi varsayılan başlangıç pozuna (genellikle T-poz) getirir.
         * @param pHierarchy Hedef HAnim hiyerarşisi.
         */
        static void SetDefaultPose(RpHAnimHierarchy* pHierarchy);

        /**
         * @brief Hiyerarşinin varsayılan pozu kabul edip edemeyeceğini kontrol eder.
         * @param pHierarchy Kontrol edilecek HAnim hiyerarşisi.
         * @return TRUE ise varsayılan poz uygulanabilir.
         */
        static RwBool HierarchyCanAcceptDefaultPose(const RpHAnimHierarchy* pHierarchy);
    };
}

#endif // RWS_SKINSAFE_H

