// ============================================================================
// CannonCruise - RenderWare Studio Framework (SkinSafe.cpp)
// Original path: D:\Projects\CannonCruisePC\code\framework\toolkits\Animation\SkinSafe.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include "SkinSafe.h"
#include <cassert>
#include <cstring>

namespace RWS
{
    void SkinSafe::CopyBoneMatrices(RwMatrix* pDstMatrices, const RwMatrix* pSrcMatrices, RwUInt32 numBones)
    {
        assert(pDstMatrices != 0 && "Failed PRE-condition: pDstMatrices");
        assert(pSrcMatrices != 0 && "Failed PRE-condition: pSrcMatrices");

        memcpy(pDstMatrices, pSrcMatrices, numBones * sizeof(RwMatrix));
    }

    RwBool SkinSafe::HierarchyCanAcceptDefaultPose(const RpHAnimHierarchy* pHierarchy)
    {
        if (!pHierarchy) return FALSE;
        return (pHierarchy->numNodes > 0 && pHierarchy->pNodeInfo != 0);
    }

    void SkinSafe::SetDefaultPose(RpHAnimHierarchy* pHierarchy)
    {
        if (!pHierarchy) return;
        assert(HierarchyCanAcceptDefaultPose(pHierarchy) && "Failed PRE-condition: HierarchyCanAcceptDefaultPose( hierarchy )");

        // Hiyerarşi matrislerini varsayılan (kimlik) poza sıfırla
        RpHAnimHierarchyUpdateMatrices(pHierarchy);
    }
}
