#ifndef RWS_MATHS_H
#define RWS_MATHS_H

// ============================================================================
// CannonCruise - RenderWare Studio Framework (maths.h)
// Original path: D:\Projects\CannonCruisePC\code\framework\toolkits\math\maths.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include <rwcore.h>
#include <cmath>
#include <cassert>

namespace RWS
{
    namespace Maths
    {
        const RwReal PI = 3.14159265358979323846f;
        const RwReal TWO_PI = 6.28318530717958647692f;
        const RwReal HALF_PI = 1.57079632679489661923f;
        const RwReal DEG_TO_RAD = 0.01745329251994329576f;
        const RwReal RAD_TO_DEG = 57.2957795130823208767f;
        const RwReal EPSILON = 0.00001f;

        // Derece - Radyan dönüşümleri
        inline RwReal DegToRad(RwReal deg) { return deg * DEG_TO_RAD; }
        inline RwReal RadToDeg(RwReal rad) { return rad * RAD_TO_DEG; }

        // Sınırlandırma (Clamp)
        template <typename T>
        inline T Clamp(T val, T minVal, T maxVal)
        {
            assert(maxVal >= minVal && "Failed PRE-condition: max >= min");
            if (val < minVal) return minVal;
            if (val > maxVal) return maxVal;
            return val;
        }

        // Min & Max
        template <typename T>
        inline T Min(T a, T b) { return (a < b) ? a : b; }

        template <typename T>
        inline T Max(T a, T b) { return (a > b) ? a : b; }

        // Lineer İnterpolasyon (Lerp)
        inline RwReal Lerp(RwReal a, RwReal b, RwReal t)
        {
            return a + (b - a) * t;
        }

        inline void VectorLerp(RwV3d* pOut, const RwV3d* pA, const RwV3d* pB, RwReal t)
        {
            pOut->x = Lerp(pA->x, pB->x, t);
            pOut->y = Lerp(pA->y, pB->y, t);
            pOut->z = Lerp(pA->z, pB->z, t);
        }

        // Vektör İşlemleri
        inline RwReal VectorLength(const RwV3d* pV)
        {
            return std::sqrt(pV->x * pV->x + pV->y * pV->y + pV->z * pV->z);
        }

        inline RwReal VectorLengthSquared(const RwV3d* pV)
        {
            return pV->x * pV->x + pV->y * pV->y + pV->z * pV->z;
        }

        inline RwReal VectorNormalize(RwV3d* pV)
        {
            RwReal len = VectorLength(pV);
            if (len > EPSILON)
            {
                RwReal invLen = 1.0f / len;
                pV->x *= invLen;
                pV->y *= invLen;
                pV->z *= invLen;
            }
            return len;
        }

        inline RwReal VectorDotProduct(const RwV3d* pA, const RwV3d* pB)
        {
            return pA->x * pB->x + pA->y * pB->y + pA->z * pB->z;
        }

        inline void VectorCrossProduct(RwV3d* pOut, const RwV3d* pA, const RwV3d* pB)
        {
            pOut->x = pA->y * pB->z - pA->z * pB->y;
            pOut->y = pA->z * pB->x - pA->x * pB->z;
            pOut->z = pA->x * pB->y - pA->y * pB->x;
        }
    }
}

#endif // RWS_MATHS_H
