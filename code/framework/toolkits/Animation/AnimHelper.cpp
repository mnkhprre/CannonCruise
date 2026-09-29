// ============================================================================
// CannonCruise - RenderWare Studio Framework (AnimHelper.cpp)
// Original path: D:\Projects\CannonCruisePC\code\framework\toolkits\Animation\AnimHelper.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include "AnimHelper.h"
#include <cassert>

namespace RWS
{
    void AnimHelper::UpdateHierarchy(RpHAnimHierarchy* pHierarchy, RwReal deltaTime)
    {
        if (!pHierarchy) return;

        RpHAnimHierarchyAddAnimTime(pHierarchy, deltaTime);
        RpHAnimHierarchyUpdateMatrices(pHierarchy);
    }

    void AnimHelper::SetCurrentAnim(RpHAnimHierarchy* pHierarchy, RtAnimAnimation* pAnim)
    {
        assert(pHierarchy != 0);

        if (pAnim)
        {
            RpHAnimHierarchySetCurrentAnim(pHierarchy, pAnim);
        }
    }

    void AnimHelper::BlendHierarchy(RpHAnimHierarchy* pDestHierarchy, RpHAnimHierarchy* pSrcHierarchy, RwReal blendAlpha)
    {
        if (!pDestHierarchy || !pSrcHierarchy) return;

        RpHAnimHierarchyBlendHierarchy(pDestHierarchy, pSrcHierarchy, blendAlpha);
    }

    void AnimHelper::UpdateHierarchyMatrices(RpHAnimHierarchy* pHierarchy)
    {
        if (pHierarchy)
        {
            RpHAnimHierarchyUpdateMatrices(pHierarchy);
        }
    }
}
