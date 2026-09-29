// ============================================================================
// CannonCruise - RenderWare Studio Framework (framehelper.cpp)
// Original path: D:\Projects\CannonCruisePC\code\framework\toolkits\world\helpers\framehelper.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include "framehelper.h"
#include <cassert>
#include <cstring>

namespace RWS
{
    struct FrameSearchData
    {
        const char* pTargetName;
        RwFrame* pFoundFrame;
    };

    static RwFrame* FrameFindCallback(RwFrame* pFrame, void* pData)
    {
        FrameSearchData* pSearch = (FrameSearchData*)pData;
        // Frame kullanıcı verisi veya node adı kontrolü
        // (RwFrameForAllChildren hiyerarşik tarama)
        pSearch->pFoundFrame = pFrame;
        return 0; // Dur
    }

    RwFrame* FrameHelper::FindFrameByName(RwFrame* pRootFrame, const char* pName)
    {
        if (!pRootFrame || !pName) return 0;

        FrameSearchData search;
        search.pTargetName = pName;
        search.pFoundFrame = 0;

        RwFrameForAllChildren(pRootFrame, FrameFindCallback, &search);
        return search.pFoundFrame;
    }

    RwFrame* FrameHelper::FindChildFrame(RwFrame* pRootFrame, RwFrame* pTarget)
    {
        if (!pRootFrame || !pTarget) return 0;
        if (pRootFrame == pTarget) return pRootFrame;

        // Recursive child search
        RwFrame* pChild = pRootFrame->child;
        while (pChild)
        {
            RwFrame* pFound = FindChildFrame(pChild, pTarget);
            if (pFound) return pFound;
            pChild = pChild->next;
        }

        return 0;
    }

    void FrameHelper::SetPosition(RwFrame* pFrame, const RwV3d* pPos)
    {
        assert(pFrame != 0);
        assert(pPos != 0);

        RwMatrix* pMatrix = RwFrameGetMatrix(pFrame);
        if (pMatrix)
        {
            pMatrix->pos = *pPos;
            RwFrameUpdateObjects(pFrame);
        }
    }

    void FrameHelper::GetPosition(const RwFrame* pFrame, RwV3d* pPos)
    {
        assert(pFrame != 0);
        assert(pPos != 0);

        const RwMatrix* pMatrix = RwFrameGetLTM(const_cast<RwFrame*>(pFrame));
        if (pMatrix)
        {
            *pPos = pMatrix->pos;
        }
    }

    void FrameHelper::Rotate(RwFrame* pFrame, const RwV3d* pAxis, RwReal angle, RwOpCombineType combineOp)
    {
        assert(pFrame != 0);
        assert(pAxis != 0);

        RwFrameRotate(pFrame, pAxis, angle, combineOp);
    }

    void FrameHelper::Translate(RwFrame* pFrame, const RwV3d* pTranslation, RwOpCombineType combineOp)
    {
        assert(pFrame != 0);
        assert(pTranslation != 0);

        RwFrameTranslate(pFrame, pTranslation, combineOp);
    }
}
