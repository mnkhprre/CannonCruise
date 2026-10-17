#ifndef WORLDGEOMETRYRESOURCE_H
#define WORLDGEOMETRYRESOURCE_H

// ============================================================================
// CannonCruise - World Geometry Resource Loader (WorldGeometryResource.h)
// Original path: D:\Projects\CannonCruisePC\code\WorldGeometryResource.h
// Reverse-engineered via Ghidra and CC.asm analysis.
// ============================================================================

#include <rwcore.h>
#include <rpworld.h>

/**
 * @class CWorldGeometryResource
 * @brief Static RenderWare RpWorld BSP scene geometry stream resource loader.
 *
 * Loads pre-compiled BSP world binary chunks (`.bsp` / `"rwID_WORLD"`) from stream
 * archives and manages RpWorld instance lifecycles.
 */
class CWorldGeometryResource
{
public:
    /**
     * @brief Loads an RpWorld BSP sector geometry database from file stream.
     * @param pFilename Filesystem path to world stream file.
     * @param pType Resource type descriptor string (e.g. "rwID_WORLD").
     * @return Pointer to loaded RpWorld, or NULL on failure.
     * @see FUN_0044a800
     */
    static void* Load(const char* pFilename, const char* pType);

    /**
     * @brief Destroys and unloads an RpWorld BSP resource.
     * @param pResource Pointer to RpWorld instance.
     * @see FUN_0044ab50
     */
    static void Unload(void* pResource);

    /**
     * @brief Registers RpWorld resource handler with framework resource manager.
     */
    static void RegisterHandlers();

    /**
     * @brief Retrieves total count of loaded BSP world resources.
     * @return Count of active RpWorld objects.
     */
    static RwUInt32 GetLoadedCount() { return sm_nLoadedResources; }

private:
    static RwUInt32 sm_nLoadedResources; ///< Counter of active world resources
};

#endif // WORLDGEOMETRYRESOURCE_H
