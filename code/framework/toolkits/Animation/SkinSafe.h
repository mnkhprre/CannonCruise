#ifndef RWS_SKINSAFE_H
#define RWS_SKINSAFE_H

// ============================================================================
// CannonCruise - RenderWare Studio Framework (SkinSafe.h)
// Original path: D:\Projects\CannonCruisePC\code\framework\toolkits\Animation\SkinSafe.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include <rwcore.h>
#include <rpworld.h>
#include <rpskin.h>
#include <rphanim.h>

namespace RWS
{
    class SkinSafe
    {
    public:
        // Güvenli skin matris kopyalama ve varsayılan poz atama
        static void CopyBoneMatrices(RwMatrix* pDstMatrices, const RwMatrix* pSrcMatrices, RwUInt32 numBones);
        static void SetDefaultPose(RpHAnimHierarchy* pHierarchy);
        static RwBool HierarchyCanAcceptDefaultPose(const RpHAnimHierarchy* pHierarchy);
    };
}

#endif // RWS_SKINSAFE_H
