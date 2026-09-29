#ifndef RWS_CLUMPHELPER_H
#define RWS_CLUMPHELPER_H

// ============================================================================
// CannonCruise - RenderWare Studio Framework (clumphelper.h)
// Original path: D:\Projects\CannonCruisePC\code\framework\toolkits\world\helpers\clumphelper.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include <rwcore.h>
#include <rpworld.h>

namespace RWS
{
    class ClumpHelper
    {
    public:
        // Clump içindeki ilk Atomic'i bul
        static RpAtomic* FindFirstAtomic(RpClump* pClump);

        // Clump atomiklerine bayrak/özellik uygula
        static void SetAtomicFlags(RpClump* pClump, RwUInt32 flags);

        // Clump bounding sphere hesaplama
        static void CalculateBoundingSphere(RpClump* pClump, RwSphere* pSphere);

        // Clump'ı klonlama ve yok etme
        static RpClump* Clone(RpClump* pClump);
        static void Destroy(RpClump* pClump);
    };
}

#endif // RWS_CLUMPHELPER_H
