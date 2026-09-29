#ifndef ASTAR_H
#define ASTAR_H

// ============================================================================
// CannonCruise - A* Grid Pathfinding (Astar.h)
// Original path: D:\Projects\CannonCruisePC\code\Astar.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include "BinMinTree.h"
#include <rwcore.h>
#include <vector>

struct SPathNode {
  RwInt32 x, z;
  RwReal g, h, f;
  SPathNode *pParent;
  bool operator<(const SPathNode &other) const { return f < other.f; }
};

class CAstar {
public:
  CAstar();
  ~CAstar();

  void Initialize(RwInt32 width, RwInt32 height);
  RwBool FindPath(RwInt32 startX, RwInt32 startZ, RwInt32 endX, RwInt32 endZ,
                  std::vector<RwV3d> &outPath);

private:
  RwInt32 m_iWidth;
  RwInt32 m_iHeight;
  CBinMinTree<SPathNode> m_OpenList;
};

#endif // ASTAR_H
