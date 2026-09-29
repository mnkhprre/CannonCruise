#ifndef RWS_FRAMEHELPER_H
#define RWS_FRAMEHELPER_H

// ============================================================================
// CannonCruise - RenderWare Studio Framework (framehelper.h)
// Original path: D:\Projects\CannonCruisePC\code\framework\toolkits\world\helpers\framehelper.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include <rwcore.h>

namespace RWS
{
    class FrameHelper
    {
    public:
        // Frame hiyerarşisinde isim veya ID ile frame arama
        static RwFrame* FindFrameByName(RwFrame* pRootFrame, const char* pName);
        static RwFrame* FindChildFrame(RwFrame* pRootFrame, RwFrame* pTarget);

        // Frame matris işlemleri
        static void SetPosition(RwFrame* pFrame, const RwV3d* pPos);
        static void GetPosition(const RwFrame* pFrame, RwV3d* pPos);
        static void Rotate(RwFrame* pFrame, const RwV3d* pAxis, RwReal angle, RwOpCombineType combineOp = rwCOMBINEPOSTCONCAT);
        static void Translate(RwFrame* pFrame, const RwV3d* pTranslation, RwOpCombineType combineOp = rwCOMBINEPOSTCONCAT);
    };
}

#endif // RWS_FRAMEHELPER_H
