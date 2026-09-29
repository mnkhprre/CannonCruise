#ifndef PATHFINDINGMAP_H
#define PATHFINDINGMAP_H

// ============================================================================
// CannonCruise - Pathfinding Map (PathFindingMap.h)
// Original path: D:\Projects\CannonCruisePC\code\PathFindingMap.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include "Astar.h"
#include <rwcore.h>
#include <vector>

class CPathFindingMap
{
public:
    CPathFindingMap();
    ~CPathFindingMap();

    RwBool Load(const char* pFilename);
    void Unload();

    RwBool IsCellBlocked(RwInt32 x, RwInt32 z) const;
    RwInt32 GetWidth() const { return m_iWidth; }
    RwInt32 GetHeight() const { return m_iHeight; }

private:
    RwInt32 m_iWidth;
    RwInt32 m_iHeight;
    std::vector<RwUInt8> m_Grid;
    CAstar m_Astar;
};

#endif // PATHFINDINGMAP_H
