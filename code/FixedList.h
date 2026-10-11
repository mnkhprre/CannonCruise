#ifndef FIXEDLIST_H
#define FIXEDLIST_H

// ============================================================================
// CannonCruise - Engine FixedList Template (FixedList.h)
// Original path: D:\Projects\CannonCruisePC\code\FixedList.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include <cassert>
#include <rwcore.h>

/**
 * @class CFixedList
 * @brief Sabit kapasiteli, dinamik bellek kullanmayan liste şablonu.
 *
 * CFixedArray'den farklı olarak aktif eleman sayısını (`m_Count`) takip eder;
 * `PushBack`/`PopBack` ile elemanlar listeye eklenir/çıkarılır. Kapasite aşımı
 * assert ile kontrol edilir.
 *
 * **Kullanım örneği:**
 * ```cpp
 * CFixedList<RwV3d, 32> waypoints;
 * waypoints.PushBack({1.0f, 0.0f, 2.0f});
 * for (RwUInt32 i = 0; i < waypoints.GetCount(); ++i) { ... }
 * ```
 *
 * **Garantiler:**
 * - Kapasite aşımında: `assert(m_Count < CAPACITY)` (debug)
 * - Alt taşmada: `assert(m_Count > 0)` (debug)
 *
 * @tparam T        Listede tutulacak eleman türü.
 * @tparam CAPACITY Maksimum eleman sayısı (compile-time sabit).
 */
template <typename T, RwUInt32 CAPACITY>
class CFixedList
{
public:
    /**
     * @brief Yapıcı — boş liste başlatır (m_Count = 0).
     */
    CFixedList() : m_Count(0)
    {
    }

    /**
     * @brief Listenin sonuna yeni bir eleman ekler.
     * @param item Eklenecek eleman.
     * @pre m_Count < CAPACITY (kapasite aşımı assert ile korunur)
     */
    void PushBack(const T& item)
    {
        assert(m_Count < CAPACITY && "FixedList overflow!");
        m_Data[m_Count++] = item;
    }

    /**
     * @brief Listenin son elemanını çıkarır.
     * @pre m_Count > 0 (alt taşma assert ile korunur)
     */
    void PopBack()
    {
        assert(m_Count > 0 && "FixedList underflow!");
        --m_Count;
    }

    /**
     * @brief Listeyi temizler (m_Count = 0 yapar, bellek serbest bırakılmaz).
     */
    void Clear()
    {
        m_Count = 0;
    }

    /**
     * @brief Aktif eleman sayısını döndürür.
     * @return RwUInt32 — Listedeki geçerli eleman sayısı.
     */
    RwUInt32 GetCount() const
    {
        return m_Count;
    }

    /**
     * @brief Maksimum eleman kapasitesini döndürür.
     * @return RwUInt32 — CAPACITY şablon parametresi.
     */
    RwUInt32 GetCapacity() const
    {
        return CAPACITY;
    }

    /**
     * @brief Listenin boş olup olmadığını döndürür.
     * @return RwBool — TRUE ise m_Count == 0.
     */
    RwBool IsEmpty() const
    {
        return m_Count == 0;
    }

    /**
     * @brief Listenin dolu olup olmadığını döndürür.
     * @return RwBool — TRUE ise m_Count >= CAPACITY.
     */
    RwBool IsFull() const
    {
        return m_Count >= CAPACITY;
    }

    /**
     * @brief Belirtilen indeksteki elemana yazılabilir referans döndürür.
     * @param index Erişilecek eleman indeksi.
     * @return T& — İstenen elemanın referansı.
     * @pre index < m_Count
     */
    T& operator[](RwUInt32 index)
    {
        assert(index < m_Count && "FixedList index out of bounds!");
        return m_Data[index];
    }

    /**
     * @brief Belirtilen indeksteki elemana salt okunur referans döndürür.
     * @param index Erişilecek eleman indeksi.
     * @return const T& — İstenen elemanın sabit referansı.
     * @pre index < m_Count
     */
    const T& operator[](RwUInt32 index) const
    {
        assert(index < m_Count && "FixedList index out of bounds!");
        return m_Data[index];
    }

private:
    T m_Data[CAPACITY]; ///< Ham veri dizisi
    RwUInt32 m_Count;   ///< Aktif eleman sayısı
};

#endif // FIXEDLIST_H
