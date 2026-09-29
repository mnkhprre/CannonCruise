// ============================================================================
// CannonCruise - Pathfinding Resource Loader (PathFindingResource.cpp)
// Original path: D:\Projects\CannonCruisePC\code\PathFindingResource.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_0044c2f0 — CPathFindingResource::Load
//   FUN_0044c970 — CPathFindingResource::Unload
// ============================================================================

#include "PathFindingResource.h"
#include <cassert>

void* CPathFindingResource::Load(const char* pFilename, const char* pType)
{
    if (!pFilename) return 0;

    CPathFindingMap* pMap = new CPathFindingMap();
    if (pMap->Load(pFilename))
    {
        return pMap;
    }

    delete pMap;
    return 0;
}

void CPathFindingResource::Unload(void* pResource)
{
    if (pResource)
    {
        CPathFindingMap* pMap = (CPathFindingMap*)pResource;
        delete pMap;
    }
}
