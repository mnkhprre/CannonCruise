// ============================================================================
// CannonCruise - Texture Resource Loader (TextureResource.cpp)
// Original path: D:\Projects\CannonCruisePC\code\TextureResource.cpp
// Reverse-engineered via Ghidra and CC.asm analysis.
//
// Binary functions:
//   FUN_0044b100 — CTextureResource::Load
//   FUN_0044b420 — CTextureResource::Unload
// ============================================================================

#include "TextureResource.h"
#include <cassert>
#include <cstring>

RwUInt32 CTextureResource::sm_nLoadedResources = 0;

/**
 * @brief Loads a texture dictionary (`.txd`) or texture stream into memory.
 *
 * Opens a RenderWare stream and reads an RwTexDictionary object.
 *
 * @param pFilename Filesystem path to the texture dictionary.
 * @param pType Resource type descriptor string (asserted non-null).
 * @return Pointer to loaded RwTexDictionary, or NULL if loading fails.
 * @see FUN_0044b100
 */
void* CTextureResource::Load(const char* pFilename, const char* pType)
{
    assert(pFilename != 0);
    assert(pType != 0 && "Failed PRE-condition: pacType");

    RwStream* pStream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, pFilename);
    if (!pStream)
    {
        return 0;
    }

    RwTexDictionary* pTexDict = 0;
    if (RwStreamFindChunk(pStream, rwID_TEXDICTIONARY, 0, 0))
    {
        pTexDict = RwTexDictionaryStreamRead(pStream);
    }

    RwStreamClose(pStream, 0);

    if (pTexDict)
    {
        RwTexDictionarySetCurrent(pTexDict);
        sm_nLoadedResources++;
    }

    return pTexDict;
}

/**
 * @brief Unloads and destroys a previously allocated RwTexDictionary.
 *
 * @param pResource Pointer to the texture dictionary.
 * @see FUN_0044b420
 */
void CTextureResource::Unload(void* pResource)
{
    if (pResource)
    {
        RwTexDictionary* pTexDict = (RwTexDictionary*)pResource;
        RwTexDictionaryDestroy(pTexDict);

        if (sm_nLoadedResources > 0)
        {
            sm_nLoadedResources--;
        }
    }
}

/**
 * @brief Registers the texture dictionary resource type with the framework.
 */
void CTextureResource::RegisterHandlers()
{
}
