#ifndef BINMINTREE_H
#define BINMINTREE_H

// ============================================================================
// CannonCruise - Binary Minimum Heap / Tree (BinMinTree.h)
// Original path: D:\Projects\CannonCruisePC\code\BinMinTree.h
// Reverse-engineered via Ghidra and CC.asm analysis.
// ============================================================================

#include <cassert>
#include <rwcore.h>
#include <vector>

/**
 * @class CBinMinTree
 * @brief Binary min-heap data structure template.
 *
 * Used as open list in A* pathfinding algorithm by `CAstar` class.
 * Each `Insert()` performs up-heap (UpHeap) to bring smallest element to root,
 * `ExtractMin()` performs down-heap (DownHeap) to re-sort after root removal.
 *
 * **Preconditions:**
 * - `T` type must define `operator<` (e.g. SPathNode).
 * - `ExtractMin()` must not be called on empty heap.
 *
 * **Complexity:**
 * - Insert: O(log n)
 * - ExtractMin: O(log n)
 * - IsEmpty / GetSize: O(1)
 *
 * @tparam T Element type stored in heap (requires operator<).
 */
template <typename T>
class CBinMinTree {
public:
    /**
     * @brief Default constructor — creates empty heap.
     */
    CBinMinTree() {}

    /**
     * @brief Adds new element to heap.
     * @param item Element to add.
     *
     * After appending to end of element vector, moved to correct position via `UpHeap()`.
     */
    void Insert(const T &item) {
        m_vTree.push_back(item);
        RwUInt32 n = (RwUInt32)m_vTree.size() - 1;
        UpHeap(n);
    }

    /**
     * @brief Extracts and returns minimum element from heap.
     * @return T — Element with smallest value (root).
     * @pre Heap must not be empty.
     *
     * Root swapped with last element, last element removed,
     * then heap property restored via `DownHeap()`.
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
     * @brief Removes all elements from heap.
     */
    void Clear() { m_vTree.clear(); }

    /**
     * @brief Returns whether heap is empty.
     * @return RwBool — TRUE if no elements in heap.
     */
    RwBool IsEmpty() const { return m_vTree.empty(); }

    /**
     * @brief Returns element count in heap.
     * @return RwUInt32 — Current element count.
     */
    RwUInt32 GetSize() const { return (RwUInt32)m_vTree.size(); }

    /**
     * @brief Provides read-only access to specific index in internal vector.
     * @param n Index to access.
     * @return const T& — Const reference to requested element.
     * @pre n < m_vTree.size()
     */
    const T &operator[](RwUInt32 n) const {
        assert(m_vTree.size() > (RwUInt32)n &&
               "m_vTree.size() > (RwUInt32)n Failed");
        return m_vTree[n];
    }

private:
    /**
     * @brief Moves element at index n upward maintaining heap property.
     * @param n Index of element to move up.
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
     * @brief Moves element at index n downward maintaining heap property.
     * @param n Index of element to move down.
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

    std::vector<T> m_vTree; ///< Dynamic vector holding heap elements
};

#endif // BINMINTREE_H
