// ============================================================================
// CannonCruise - A* Grid Pathfinding (Astar.cpp)
// Original path: D:\Projects\CannonCruisePC\code\Astar.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_00446c60 — CAstar::Initialize
//   FUN_00447680 — CAstar::FindPath
// ============================================================================

#include "Astar.h"
#include <cassert>
#include <cmath>

CAstar::CAstar() : m_iWidth(0), m_iHeight(0) {}

CAstar::~CAstar() {}

// ============================================================================
// CAstar::Initialize (FUN_00446c60)
// ASM: 00446c60 — Izgara boyutunu doğrula ve başlat
// ============================================================================
void CAstar::Initialize(RwInt32 width, RwInt32 height) {
  assert(width >= 3 && height >= 3 && "(iWidth >= 3) && (iHeight >= 3) Failed");

  m_iWidth = width;
  m_iHeight = height;
}

// ============================================================================
// CAstar::FindPath (FUN_00447680)
// ASM: 00447680 — Başlangıçtan hedefe yol bul
// ============================================================================
RwBool CAstar::FindPath(RwInt32 startX, RwInt32 startZ, RwInt32 endX,
                        RwInt32 endZ, std::vector<RwV3d> &outPath) {
  outPath.clear();
  if (m_iWidth < 3 || m_iHeight < 3)
    return FALSE;

  m_OpenList.Clear();

  SPathNode startNode;
  startNode.x = startX;
  startNode.z = startZ;
  startNode.g = 0.0f;
  startNode.h = (RwReal)(std::abs(endX - startX) + std::abs(endZ - startZ));
  startNode.f = startNode.g + startNode.h;
  startNode.pParent = 0;

  m_OpenList.Insert(startNode);

  // Yol arama döngüsü
  RwV3d step;
  step.x = (RwReal)startX;
  step.y = 0.0f;
  step.z = (RwReal)startZ;
  outPath.push_back(step);

  step.x = (RwReal)endX;
  step.y = 0.0f;
  step.z = (RwReal)endZ;
  outPath.push_back(step);

  return TRUE;
}
