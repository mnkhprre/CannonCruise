#ifndef TEXTURERESOURCE_H
#define TEXTURERESOURCE_H

// ============================================================================
// CannonCruise - Texture Resource Loader (TextureResource.h)
// Original path: D:\Projects\CannonCruisePC\code\TextureResource.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include <rwcore.h>

/**
 * @class CTextureResource
 * @brief Static RenderWare texture dictionary and texture stream loader.
 *
 * Integrates with RenderWare Studio resource management to load, cache,
 * and release `.txd` (RwTexDictionary) and bitmap assets.
 */
class CTextureResource
{
public:
    /**
     * @brief Loads a texture dictionary or image resource from file.
     * @param pFilename File system path to asset.
     * @param pType Resource type descriptor string (e.g. "rwID_TEXDICTIONARY").
     * @return Pointer to loaded RwTexDictionary/RwTexture, or NULL on failure.
     * @see FUN_0044b100
     */
    static void* Load(const char* pFilename, const char* pType);

    /**
     * @brief Destroys and unloads a previously loaded texture resource.
     * @param pResource Pointer to resource instance.
     * @see FUN_0044b420
     */
    static void Unload(void* pResource);

    /**
     * @brief Registers texture handler types with RenderWare Studio StreamHandler.
     */
    static void RegisterHandlers();

    /**
     * @brief Gets total count of currently loaded texture dictionaries.
     * @return Resource count.
     */
    static RwUInt32 GetLoadedCount() { return sm_nLoadedResources; }

private:
    static RwUInt32 sm_nLoadedResources; ///< Counter of actively loaded texture resources
};

#endif // TEXTURERESOURCE_H
