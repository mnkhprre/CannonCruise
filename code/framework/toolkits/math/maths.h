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
    /**
     * @namespace RWS::Maths
     * @brief Matematik, vektör işlemleri ve trigonometri yardımcıları.
     *
     * Oyun motoru içerisindeki rotasyon hesaplamaları, interpolasyon (Lerp)
     * ve vektörel çarpım gibi temel lineer cebir işlemlerini barındırır.
     * Gemi fiziği, yapay zeka (PathFinding) ve mermi yörüngeleri bu
     * araçlara güvenir.
     */
    namespace Maths
    {
        /// Pi sayısı (3.1415...)
        const RwReal PI = 3.14159265358979323846f;
        /// 2 * Pi (360 derece)
        const RwReal TWO_PI = 6.28318530717958647692f;
        /// Pi / 2 (90 derece)
        const RwReal HALF_PI = 1.57079632679489661923f;
        /// Dereceden radyana çevrim çarpanı (Pi / 180)
        const RwReal DEG_TO_RAD = 0.01745329251994329576f;
        /// Radyandan dereceye çevrim çarpanı (180 / Pi)
        const RwReal RAD_TO_DEG = 57.2957795130823208767f;
        /// Kayar nokta (float) hata toleransı / sıfıra yakınlık sınırı
        const RwReal EPSILON = 0.00001f;

        /**
         * @brief Derece cinsinden açıyı radyana çevirir.
         * @param deg Derece (0-360)
         * @return RwReal — Radyan (0 - 2PI)
         */
        inline RwReal DegToRad(RwReal deg) { return deg * DEG_TO_RAD; }

        /**
         * @brief Radyan cinsinden açıyı dereceye çevirir.
         * @param rad Radyan (0 - 2PI)
         * @return RwReal — Derece (0-360)
         */
        inline RwReal RadToDeg(RwReal rad) { return rad * RAD_TO_DEG; }

        /**
         * @brief Değeri belirtilen minimum ve maksimum aralığına sıkıştırır (Clamp).
         * @param val    Sınanacak değer.
         * @param minVal Minimum sınır.
         * @param maxVal Maksimum sınır.
         * @return T — Sınırlar içerisindeki değer.
         */
        template <typename T>
        inline T Clamp(T val, T minVal, T maxVal)
        {
            assert(maxVal >= minVal && "Failed PRE-condition: max >= min");
            if (val < minVal) return minVal;
            if (val > maxVal) return maxVal;
            return val;
        }

        /**
         * @brief İki değerden küçüğünü döndürür.
         */
        template <typename T>
        inline T Min(T a, T b) { return (a < b) ? a : b; }

        /**
         * @brief İki değerden büyüğünü döndürür.
         */
        template <typename T>
        inline T Max(T a, T b) { return (a > b) ? a : b; }

        /**
         * @brief İki kayar nokta sayı arasında doğrusal interpolasyon yapar (Lerp).
         * @param a Başlangıç değeri.
         * @param b Bitiş değeri.
         * @param t Geçiş ağırlığı (0.0 - 1.0).
         * @return RwReal — Araya girilen (interpolated) değer.
         */
        inline RwReal Lerp(RwReal a, RwReal b, RwReal t)
        {
            return a + (b - a) * t;
        }

        /**
         * @brief İki vektör arasında doğrusal interpolasyon yapar.
         * @param pOut Çıkış vektörü (hesaplanan değer buraya yazılır).
         * @param pA   Başlangıç vektörü.
         * @param pB   Bitiş vektörü.
         * @param t    Geçiş ağırlığı (0.0 - 1.0).
         */
        inline void VectorLerp(RwV3d* pOut, const RwV3d* pA, const RwV3d* pB, RwReal t)
        {
            pOut->x = Lerp(pA->x, pB->x, t);
            pOut->y = Lerp(pA->y, pB->y, t);
            pOut->z = Lerp(pA->z, pB->z, t);
        }

        /**
         * @brief Vektörün büyüklüğünü (uzunluğunu) döndürür.
         * @param pV Hedef vektör.
         * @return RwReal — Uzunluk.
         */
        inline RwReal VectorLength(const RwV3d* pV)
        {
            return std::sqrt(pV->x * pV->x + pV->y * pV->y + pV->z * pV->z);
        }

        /**
         * @brief Vektörün karesel uzunluğunu (karekök almadan) döndürür.
         *
         * Sadece uzaklık karşılaştırmaları yapılacaksa daha hızlıdır.
         */
        inline RwReal VectorLengthSquared(const RwV3d* pV)
        {
            return pV->x * pV->x + pV->y * pV->y + pV->z * pV->z;
        }

        /**
         * @brief Vektörü birim vektöre çevirir (Normalize eder) ve eski uzunluğunu döndürür.
         * @param pV Normalize edilecek vektör.
         * @return RwReal — Vektörün normalize edilmeden önceki uzunluğu.
         */
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

        /**
         * @brief İki vektörün skaler (Nokta / Dot) çarpımını hesaplar.
         * @param pA İlk vektör.
         * @param pB İkinci vektör.
         * @return RwReal — Skaler çarpım sonucu (Kosinüs benzerliği).
         */
        inline RwReal VectorDotProduct(const RwV3d* pA, const RwV3d* pB)
        {
            return pA->x * pB->x + pA->y * pB->y + pA->z * pB->z;
        }

        /**
         * @brief İki vektörün vektörel (Çapraz / Cross) çarpımını hesaplar.
         * @param pOut Çıkış vektörü (iki vektöre de dik olan yeni vektör).
         * @param pA   İlk vektör.
         * @param pB   İkinci vektör.
         */
        inline void VectorCrossProduct(RwV3d* pOut, const RwV3d* pA, const RwV3d* pB)
        {
            pOut->x = pA->y * pB->z - pA->z * pB->y;
            pOut->y = pA->z * pB->x - pA->x * pB->z;
            pOut->z = pA->x * pB->y - pA->y * pB->x;
        }
    }
}

#endif // RWS_MATHS_H

