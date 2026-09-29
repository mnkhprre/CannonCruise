// ============================================================================
// CannonCruise - Pathfinding Map (PathFindingMap.cpp)
// Original path: D:\Projects\CannonCruisePC\code\PathFindingMap.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_0044bf50 — CPathFindingMap::Load
// ============================================================================

#include "PathFindingMap.h"
#include <cassert>
#include <cstdio>

CPathFindingMap::CPathFindingMap()
    : m_iWidth(0),
      m_iHeight(0)
{
}

CPathFindingMap::~CPathFindingMap()
{
    Unload();
}

RwBool CPathFindingMap::Load(const char* pFilename)
{
    if (!pFilename) return FALSE;

    FILE* pFile = fopen(pFilename, "rb");
    if (!pFile)
    {
        assert(false && "Corrupt Pathfinding data file or unknown fileformat!");
        return FALSE;
    }

    fread(&m_iWidth, sizeof(RwInt32), 1, pFile);
    fread(&m_iHeight, sizeof(RwInt32), 1, pFile);

    if (m_iWidth < 3 || m_iHeight < 3)
    {
        assert(false && "Can't make a CPool with MaxElements = 0");
        fclose(pFile);
        return FALSE;
    }

    RwUInt32 totalCells = m_iWidth * m_iHeight;
    m_Grid.resize(totalCells);
    fread(&m_Grid[0], 1, totalCells, pFile);

    fclose(pFile);
    m_Astar.Initialize(m_iWidth, m_iHeight);
    return TRUE;
}

void CPathFindingMap::Unload()
{
    m_Grid.clear();
    m_iWidth = 0;
    m_iHeight = 0;
}

RwBool CPathFindingMap::IsCellBlocked(RwInt32 x, RwInt32 z) const
{
    if (x < 0 || x >= m_iWidth || z < 0 || z >= m_iHeight) return TRUE;
    return m_Grid[z * m_iWidth + x] != 0;
}
