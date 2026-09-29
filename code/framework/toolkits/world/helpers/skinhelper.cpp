// ============================================================================
// CannonCruise - RenderWare Studio Framework (skinhelper.cpp)
// Original path: D:\Projects\CannonCruisePC\code\framework\toolkits\world\helpers\skinhelper.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include "skinhelper.h"
#include <cassert>

namespace RWS
{
    static RpAtomic* FindSkinAtomicCallback(RpAtomic* pAtomic, void* pData)
    {
        if (RpSkinAtomicGetSkin(pAtomic) != 0)
        {
            *(RpAtomic**)pData = pAtomic;
            return 0; // Bulundu, dur
        }
        return pAtomic;
    }

    RpAtomic* SkinHelper::FindSkinAtomic(RpClump* pClump)
    {
        if (!pClump) return 0;

        RpAtomic* pFoundAtomic = 0;
        RpClumpForAllAtomics(pClump, FindSkinAtomicCallback, &pFoundAtomic);
        return pFoundAtomic;
    }

    RpHAnimHierarchy* SkinHelper::GetHierarchy(RpAtomic* pAtomic)
    {
        if (!pAtomic) return 0;

        RpSkin* pSkin = RpSkinAtomicGetSkin(pAtomic);
        assert(pSkin != 0 && "Failed PRE-condition: pSkin");

        return RpSkinAtomicGetHAnimHierarchy(pAtomic);
    }

    RwBool SkinHelper::SetHierarchy(RpAtomic* pAtomic, RpHAnimHierarchy* pHierarchy)
    {
        if (!pAtomic || !pHierarchy) return FALSE;

        RpSkinAtomicSetHAnimHierarchy(pAtomic, pHierarchy);
        return TRUE;
    }
}
