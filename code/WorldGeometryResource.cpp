// ============================================================================
// CannonCruise - World Geometry Resource Loader (WorldGeometryResource.cpp)
// Original path: D:\Projects\CannonCruisePC\code\WorldGeometryResource.cpp
// Reverse-engineered via Ghidra and CC.asm analysis.
//
// Binary functions:
//   FUN_0044a800 — CWorldGeometryResource::Load
//   FUN_0044ab50 — CWorldGeometryResource::Unload
// ============================================================================

#include "WorldGeometryResource.h"
#include <cassert>

RwUInt32 CWorldGeometryResource::sm_nLoadedResources = 0;

/**
 * @brief Loads an RpWorld BSP sector geometry database from a RenderWare stream.
 *
 * @param pFilename Filesystem path to the world binary archive.
 * @param pType Resource type descriptor string (asserted non-null).
 * @return Pointer to initialized RpWorld, or NULL if stream read fails.
 * @see FUN_0044a800
 */
void* CWorldGeometryResource::Load(const char* pFilename, const char* pType)
{
    assert(pFilename != 0);
    assert(pType != 0 && "Failed PRE-condition: pacType");

    RwStream* pStream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, pFilename);
    if (!pStream)
    {
        return 0;
    }

    RpWorld* pWorld = 0;
    if (RwStreamFindChunk(pStream, rwID_WORLD, 0, 0))
    {
        pWorld = RpWorldStreamRead(pStream);
    }

    RwStreamClose(pStream, 0);

    if (pWorld)
    {
        sm_nLoadedResources++;
    }

    return pWorld;
}

/**
 * @brief Destroys and frees an RpWorld instance.
 *
 * @param pResource Pointer to RpWorld instance.
 * @see FUN_0044ab50
 */
void CWorldGeometryResource::Unload(void* pResource)
{
    if (pResource)
    {
        RpWorld* pWorld = (RpWorld*)pResource;
        RpWorldDestroy(pWorld);

        if (sm_nLoadedResources > 0)
        {
            sm_nLoadedResources--;
        }
    }
}

/**
 * @brief Registers world geometry loader with the stream handler system.
 */
void CWorldGeometryResource::RegisterHandlers()
{
}
