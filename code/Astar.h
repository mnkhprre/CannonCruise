#ifndef ASTAR_H
#define ASTAR_H

// ============================================================================
// CannonCruise - A* Grid Pathfinding (Astar.h)
// Original path: D:\Projects\CannonCruisePC\code\Astar.h
// Reverse-engineered via Ghidra and CC.asm analysis.
// Binary ref: FUN_00473800 - FUN_00473f00 (CAstar methods)
// ============================================================================

#include "BinMinTree.h"
#include <rwcore.h>
#include <vector>

/**
 * @struct SPathNode
 * @brief Represents a single grid node for A* algorithm.
 *
 * Contains grid coordinates (x, z), g/h/f costs, and parent node pointer.
 * Sorted by priority in CBinMinTree<SPathNode> open list.
 */
struct SPathNode {
  RwInt32 x;          ///< X coordinate in grid (column)
  RwInt32 z;          ///< Z coordinate in grid (row)
  RwReal g;           ///< Actual cost from start to this node
  RwReal h;           ///< Estimated cost from this node to target (heuristic)
  RwReal f;           ///< Total cost: f = g + h
  SPathNode *pParent; ///< Parent node pointer for path tracing

  /**
   * @brief Comparison operator for min-heap ordering.
   * @param other Other node to compare.
   * @return True if this node's f value is smaller.
   */
  bool operator<(const SPathNode &other) const { return f < other.f; }
};

/**
 * @class CAstar
 * @brief A* pathfinding algorithm on 2D grid.
 *
 * Used by CPathFindingMap; calculates path allowing NPC AIs (Soldier, Bird)
 * to reach target by overcoming obstacles.
 * Grid dimensions set via Initialize(); each FindPath() call performs
 * independent path calculation.
 *
 * **Algorithm:**
 * - Open list: CBinMinTree<SPathNode> (min-heap)
 * - Heuristic: Manhattan distance
 * - Neighbors: 4-directional (north, south, east, west)
 *
 * **Binary references:**
 * - Ctor: `FUN_00473800`
 * - FindPath: `FUN_004738a0`
 * - Initialize: `FUN_00473850`
 */
class CAstar {
public:
  /**
   * @brief Default constructor — initializes empty grid.
   */
  CAstar();

  /**
   * @brief Destructor — frees open list memory.
   */
  ~CAstar();

  /**
   * @brief Sets grid dimensions.
   * @param width  Number of columns (X axis).
   * @param height Number of rows (Z axis).
   */
  void Initialize(RwInt32 width, RwInt32 height);

  /**
   * @brief Searches path from start to target node.
   * @param startX Start X coordinate.
   * @param startZ Start Z coordinate.
   * @param endX   Target X coordinate.
   * @param endZ   Target Z coordinate.
   * @param outPath Output vector filled with path in 3D world coordinates.
   * @return RwBool — TRUE if valid path found; FALSE if unreachable.
   */
  RwBool FindPath(RwInt32 startX, RwInt32 startZ, RwInt32 endX, RwInt32 endZ,
                  std::vector<RwV3d> &outPath);

private:
  RwInt32 m_iWidth;             ///< Grid width (column count)
  RwInt32 m_iHeight;            ///< Grid height (row count)
  CBinMinTree<SPathNode> m_OpenList; ///< A* open list (min-heap)
};

#endif // ASTAR_H
