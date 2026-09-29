#ifndef RWS_ANIMHELPER_H
#define RWS_ANIMHELPER_H

// ============================================================================
// CannonCruise - RenderWare Studio Framework (AnimHelper.h)
// Original path: D:\Projects\CannonCruisePC\code\framework\toolkits\Animation\AnimHelper.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include <rwcore.h>
#include <rpworld.h>
#include <rphanim.h>
#include <rtanim.h>

namespace RWS
{
    class AnimHelper
    {
    public:
        // HAnim animasyonu oynatma ve güncelleme
        static void UpdateHierarchy(RpHAnimHierarchy* pHierarchy, RwReal deltaTime);
        static void SetCurrentAnim(RpHAnimHierarchy* pHierarchy, RtAnimAnimation* pAnim);
        static void BlendHierarchy(RpHAnimHierarchy* pDestHierarchy, RpHAnimHierarchy* pSrcHierarchy, RwReal blendAlpha);

        // Hiyerarşi matrislerini güncelleme
        static void UpdateHierarchyMatrices(RpHAnimHierarchy* pHierarchy);
    };
}

#endif // RWS_ANIMHELPER_H
