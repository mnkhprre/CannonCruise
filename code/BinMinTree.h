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

template <typename T> class CBinMinTree {
public:
  CBinMinTree() {}

  void Insert(const T &item) {
    m_vTree.push_back(item);
    RwUInt32 n = (RwUInt32)m_vTree.size() - 1;
    UpHeap(n);
  }

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

  void Clear() { m_vTree.clear(); }

  RwBool IsEmpty() const { return m_vTree.empty(); }

  RwUInt32 GetSize() const { return (RwUInt32)m_vTree.size(); }

  const T &operator[](RwUInt32 n) const {
    assert(m_vTree.size() > (RwUInt32)n &&
           "m_vTree.size() > (RwUInt32)n Failed");
    return m_vTree[n];
  }

private:
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

  std::vector<T> m_vTree;
};

#endif // BINMINTREE_H
