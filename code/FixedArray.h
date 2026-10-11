#ifndef FIXEDARRAY_H
#define FIXEDARRAY_H

// ============================================================================
// CannonCruise - Engine FixedArray Template (FixedArray.h)
// Original path: D:\Projects\CannonCruisePC\code\FixedArray.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include <cassert>
#include <cstring>
#include <rwcore.h>

/**
 * @class CFixedArray
 * @brief Sabit boyutlu, compile-time parametreli dizi şablonu.
 *
 * Standart C dizisini güvenli erişim için sarmalayan; boyut, indeks sınırı
 * ve veri işaretçisi kontrolleri sağlayan bir yardımcı şablon sınıf.
 * Dinamik bellek tahsisi gerektirmez ve heap'e ihtiyaç duymadan stack veya
 * üye değişken olarak kullanılabilir.
 *
 * **Kullanım örneği:**
 * ```cpp
 * CFixedArray<RwV3d, 16> pathWaypoints; // 16 waypoint tutan sabit dizi
 * pathWaypoints[0] = { 0.0f, 0.0f, 0.0f };
 * ```
 *
 * **Güvenlik özellikleri:**
 * - `operator[]` indeks sınır denetimi (assert ile)
 * - SIZE > 0 denetimi
 *
 * @tparam T    Dizide tutulacak eleman türü.
 * @tparam SIZE Dizi boyutu (compile-time sabit).
 */
template <typename T, RwUInt32 SIZE>
class CFixedArray
{
public:
    /**
     * @brief Varsayılan yapıcı — elemanları başlatmaz (ham bellek).
     */
    CFixedArray()
    {
    }

    /**
     * @brief Belirtilen indeksteki elemana yazılabilir referans döndürür.
     * @param index Erişilecek eleman indeksi (0 tabanlı).
     * @return T& — İstenen elemanın referansı.
     * @pre index < SIZE
     */
    T& operator[](RwUInt32 index)
    {
        assert(SIZE > 0 && "Can't access array with size 0 !");
        assert(index < SIZE && "CFixedArray() index out of bounds!");
        return m_Data[index];
    }

    /**
     * @brief Belirtilen indeksteki elemana salt okunur referans döndürür.
     * @param index Erişilecek eleman indeksi (0 tabanlı).
     * @return const T& — İstenen elemanın sabit referansı.
     * @pre index < SIZE
     */
    const T& operator[](RwUInt32 index) const
    {
        assert(SIZE > 0 && "Can't access array with size 0 !");
        assert(index < SIZE && "CFixedArray() index out of bounds!");
        return m_Data[index];
    }

    /**
     * @brief Dizinin sabit boyutunu döndürür.
     * @return RwUInt32 — SIZE şablon parametresi.
     */
    RwUInt32 GetSize() const
    {
        return SIZE;
    }

    /**
     * @brief Ham veri işaretçisini döndürür.
     * @return T* — İlk elemana işaretçi.
     */
    T* GetData()
    {
        return m_Data;
    }

    /**
     * @brief Ham veri işaretçisini salt okunur olarak döndürür.
     * @return const T* — İlk elemana sabit işaretçi.
     */
    const T* GetData() const
    {
        return m_Data;
    }

private:
    T m_Data[SIZE]; ///< Sabit boyutlu ham veri dizisi
};

#endif // FIXEDARRAY_H
