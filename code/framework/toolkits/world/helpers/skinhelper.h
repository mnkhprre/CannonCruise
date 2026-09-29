#ifndef RWS_SKINHELPER_H
#define RWS_SKINHELPER_H

// ============================================================================
// CannonCruise - RenderWare Studio Framework (skinhelper.h)
// Original path: D:\Projects\CannonCruisePC\code\framework\toolkits\world\helpers\skinhelper.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include <rwcore.h>
#include <rpworld.h>
#include <rpskin.h>
#include <rphanim.h>

namespace RWS
{
    class SkinHelper
    {
    public:
        // Clump içindeki skin atomic ve animasyon hiyerarşisini bul
        static RpAtomic* FindSkinAtomic(RpClump* pClump);
        static RpHAnimHierarchy* GetHierarchy(RpAtomic* pAtomic);

        // HAnim hiyerarşisini atomic'e bağla
        static RwBool SetHierarchy(RpAtomic* pAtomic, RpHAnimHierarchy* pHierarchy);
    };
}

#endif // RWS_SKINHELPER_H
