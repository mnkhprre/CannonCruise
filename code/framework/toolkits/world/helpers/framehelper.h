#ifndef RWS_FRAMEHELPER_H
#define RWS_FRAMEHELPER_H

// ============================================================================
// CannonCruise - RenderWare Studio Framework (framehelper.h)
// Original path: D:\Projects\CannonCruisePC\code\framework\toolkits\world\helpers\framehelper.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// Binary ref: FUN_00408900 - FUN_00408b40 (RWS::FrameHelper)
// ============================================================================

#include <rwcore.h>

namespace RWS
{
    /**
     * @class FrameHelper
     * @brief RwFrame hiyerarşisi üzerinde işlem yapmak için yardımcı sınıf.
     *
     * CannonCruise'daki her 3D nesnenin dönüşümü `RwFrame` hiyerarşisi
     * aracılığıyla yönetilir. `FrameHelper`, ad ile frame arama, konum ayarlama
     * ve döndürme gibi sık kullanılan işlemleri kapsüller.
     *
     * **Binary referansları:**
     * - FindFrameByName: `FUN_00408900`
     * - SetPosition: `FUN_004089a0`
     * - Rotate: `FUN_00408a10`
     */
    class FrameHelper
    {
    public:
        /**
         * @brief Frame hiyerarşisinde belirtilen adda bir frame arar.
         * @param pRootFrame Kök frame.
         * @param pName      Aranan frame adı.
         * @return RwFrame* — Bulunan frame; bulunamazsa nullptr.
         */
        static RwFrame* FindFrameByName(RwFrame* pRootFrame, const char* pName);

        /**
         * @brief Hiyerarşideki hedef frame'i arar.
         * @param pRootFrame Kök frame.
         * @param pTarget    Hedef frame.
         * @return RwFrame* — Bulunan frame; bulunamazsa nullptr.
         */
        static RwFrame* FindChildFrame(RwFrame* pRootFrame, RwFrame* pTarget);

        /**
         * @brief Frame'in dünya konumunu ayarlar.
         * @param pFrame Hedef frame.
         * @param pPos   Yeni dünya konumu.
         */
        static void SetPosition(RwFrame* pFrame, const RwV3d* pPos);

        /**
         * @brief Frame'in dünya konumunu alır.
         * @param pFrame Kaynak frame.
         * @param pPos   [out] Konum değerinin yazılacağı çıkış.
         */
        static void GetPosition(const RwFrame* pFrame, RwV3d* pPos);

        /**
         * @brief Frame'i belirtilen eksen etrafında döndürür.
         * @param pFrame     Hedef frame.
         * @param pAxis      Dönüş ekseni (normalize edilmiş).
         * @param angle      Dönüş açısı (radyan).
         * @param combineOp  Matris birleştirme yöntemi (varsayılan: POST_CONCAT).
         */
        static void Rotate(RwFrame* pFrame, const RwV3d* pAxis, RwReal angle,
                           RwOpCombineType combineOp = rwCOMBINEPOSTCONCAT);

        /**
         * @brief Frame'i belirtilen vektör kadar taşır.
         * @param pFrame       Hedef frame.
         * @param pTranslation Taşıma vektörü.
         * @param combineOp    Matris birleştirme yöntemi (varsayılan: POST_CONCAT).
         */
        static void Translate(RwFrame* pFrame, const RwV3d* pTranslation,
                              RwOpCombineType combineOp = rwCOMBINEPOSTCONCAT);
    };
}

#endif // RWS_FRAMEHELPER_H
