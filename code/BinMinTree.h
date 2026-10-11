#ifndef BINMINTREE_H
#define BINMINTREE_H

// ============================================================================
// CannonCruise - Binary Minimum Heap / Tree (BinMinTree.h)
// Original path: D:\Projects\CannonCruisePC\code\BinMinTree.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include <cassert>
#include <rwcore.h>
#include <vector>

/**
 * @class CBinMinTree
 * @brief İkili minimum yığın (binary min-heap) veri yapısı şablonu.
 *
 * A* yol bulma algoritmasında `CAstar` sınıfının açık listesi olarak kullanılır.
 * Her `Insert()` en küçük elemanı kök konumuna getirmek için yukarı yığınlama
 * (UpHeap), `ExtractMin()` ise kaldırılan kökü yeniden sıralamak için aşağı
 * yığınlama (DownHeap) uygular.
 *
 * **Kullanım koşulları:**
 * - `T` türü `operator<` tanımlamalıdır (SPathNode gibi).
 * - `ExtractMin()` boş yığında çağrılmamalıdır.
 *
 * **Karmaşıklık:**
 * - Insert: O(log n)
 * - ExtractMin: O(log n)
 * - IsEmpty / GetSize: O(1)
 *
 * @tparam T Yığında tutulacak eleman türü (operator< gerektir).
 */
template <typename T>
class CBinMinTree {
public:
    /**
     * @brief Varsayılan yapıcı — boş yığın oluşturur.
     */
    CBinMinTree() {}

    /**
     * @brief Yığına yeni bir eleman ekler.
     * @param item Eklenecek eleman.
     *
     * Eleman vektörün sonuna eklendikten sonra `UpHeap()` ile
     * uygun konuma taşınır.
     */
    void Insert(const T &item) {
        m_vTree.push_back(item);
        RwUInt32 n = (RwUInt32)m_vTree.size() - 1;
        UpHeap(n);
    }

    /**
     * @brief Minimum elemanı yığından çıkarır ve döndürür.
     * @return T — En küçük değere sahip eleman (kök).
     * @pre Yığın boş olmamalıdır.
     *
     * Kök, son elemanla yer değiştirir, son eleman çıkarılır,
     * ardından `DownHeap()` ile yığın özelliği yeniden sağlanır.
     */
    T ExtractMin() {
        assert(m_vTree.size() > 0 && "m_vTree.size() > 0 Failed");
        T minItem = m_vTree[0];
        m_vTree[0] = m_vTree.back();
        m_vTree.pop_back();
        if (!m_vTree.empty()) {
            DownHeap(0);
        }
        return minItem;
    }

    /**
     * @brief Tüm elemanları yığından temizler.
     */
    void Clear() { m_vTree.clear(); }

    /**
     * @brief Yığının boş olup olmadığını döndürür.
     * @return RwBool — TRUE ise yığında eleman yoktur.
     */
    RwBool IsEmpty() const { return m_vTree.empty(); }

    /**
     * @brief Yığındaki eleman sayısını döndürür.
     * @return RwUInt32 — Mevcut eleman sayısı.
     */
    RwUInt32 GetSize() const { return (RwUInt32)m_vTree.size(); }

    /**
     * @brief İç vektördeki belirli bir indekse salt okunur erişim sağlar.
     * @param n Erişilecek indeks.
     * @return const T& — İstenen elemanın sabit referansı.
     * @pre n < m_vTree.size()
     */
    const T &operator[](RwUInt32 n) const {
        assert(m_vTree.size() > (RwUInt32)n &&
               "m_vTree.size() > (RwUInt32)n Failed");
        return m_vTree[n];
    }

private:
    /**
     * @brief n indeksindeki elemanı yukarı doğru yığın özelliğini sağlayarak taşır.
     * @param n Yukarı taşınacak elemanın indeksi.
     */
    void UpHeap(RwUInt32 n) {
        while (n > 0) {
            RwUInt32 parent = (n - 1) / 2;
            if (m_vTree[n] < m_vTree[parent]) {
                T temp = m_vTree[n];
                m_vTree[n] = m_vTree[parent];
                m_vTree[parent] = temp;
                n = parent;
            } else {
                break;
            }
        }
    }

    /**
     * @brief n indeksindeki elemanı aşağı doğru yığın özelliğini sağlayarak taşır.
     * @param n Aşağı taşınacak elemanın indeksi.
     */
    void DownHeap(RwUInt32 n) {
        RwUInt32 nTreeSize = (RwUInt32)m_vTree.size();
        assert(nTreeSize > 0 && "nTreeSize > 0 Failed");

        while (true) {
            RwUInt32 left = 2 * n + 1;
            RwUInt32 right = 2 * n + 2;
            RwUInt32 smallest = n;

            if (left < nTreeSize && m_vTree[left] < m_vTree[smallest]) {
                smallest = left;
            }
            if (right < nTreeSize && m_vTree[right] < m_vTree[smallest]) {
                smallest = right;
            }

            if (smallest != n) {
                T temp = m_vTree[n];
                m_vTree[n] = m_vTree[smallest];
                m_vTree[smallest] = temp;
                n = smallest;
            } else {
                break;
            }
        }
    }

    std::vector<T> m_vTree; ///< Yığın elemanlarını tutan dinamik vektör
};

#endif // BINMINTREE_H
