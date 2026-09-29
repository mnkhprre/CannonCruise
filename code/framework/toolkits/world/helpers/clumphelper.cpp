// ============================================================================
// CannonCruise - RenderWare Studio Framework (clumphelper.cpp)
// Original path: D:\Projects\CannonCruisePC\code\framework\toolkits\world\helpers\clumphelper.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include "clumphelper.h"
#include <cassert>

namespace RWS
{
    static RpAtomic* GetFirstAtomicCallback(RpAtomic* pAtomic, void* pData)
    {
        *(RpAtomic**)pData = pAtomic;
        return 0; // İlkinde dur
    }

    static RpAtomic* SetAtomicFlagsCallback(RpAtomic* pAtomic, void* pData)
    {
        RwUInt32 flags = *(RwUInt32*)pData;
        RpAtomicSetFlags(pAtomic, flags);
        return pAtomic;
    }

    RpAtomic* ClumpHelper::FindFirstAtomic(RpClump* pClump)
    {
        if (!pClump) return 0;

        RpAtomic* pFoundAtomic = 0;
        RpClumpForAllAtomics(pClump, GetFirstAtomicCallback, &pFoundAtomic);
        return pFoundAtomic;
    }

    void ClumpHelper::SetAtomicFlags(RpClump* pClump, RwUInt32 flags)
    {
        if (!pClump) return;
        RpClumpForAllAtomics(pClump, SetAtomicFlagsCallback, &flags);
    }

    void ClumpHelper::CalculateBoundingSphere(RpClump* pClump, RwSphere* pSphere)
    {
        assert(pClump != 0);
        assert(pSphere != 0);

        RpAtomic* pAtomic = FindFirstAtomic(pClump);
        if (pAtomic && RpAtomicGetGeometry(pAtomic))
        {
            const RwSphere* pGeomSphere = RpGeometryGetBoundingSphere(RpAtomicGetGeometry(pAtomic));
            if (pGeomSphere)
            {
                *pSphere = *pGeomSphere;
            }
        }
    }

    RpClump* ClumpHelper::Clone(RpClump* pClump)
    {
        if (!pClump) return 0;
        return RpClumpClone(pClump);
    }

    void ClumpHelper::Destroy(RpClump* pClump)
    {
        if (pClump)
        {
            RpClumpDestroy(pClump);
        }
    }
}
