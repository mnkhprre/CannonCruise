// ============================================================================
// CannonCruise - RenderWare Studio Framework (rwgfxresource.cpp)
// Original path: D:\Projects\CannonCruisePC\code\framework\toolkits\rwgfxresource\rwgfxresource.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_00424e30 — RwGfxResource::Load
//   FUN_00425360 — RwGfxResource::Unload
// ============================================================================

#include "rwgfxresource.h"
#include <cassert>
#include <cstring>

namespace RWS
{
    // ========================================================================
    // RwGfxResource::Load (FUN_00424e30)
    // ASM: 00424e30 — Türüne göre (CLUMP, TEXDICTIONARY vb.) grafik kaynağı yükle
    // ========================================================================
    void* RwGfxResource::Load(const char* pFilename, const char* pType)
    {
        assert(pFilename != 0 && "Failed PRE-condition: psResourcePath");
        assert(pType != 0 && "Failed PRE-condition: psType");

        RwStream* pStream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, pFilename);
        if (!pStream) return 0;

        void* pResource = 0;

        if (strcmp(pType, "CLUMP") == 0)
        {
            if (RwStreamFindChunk(pStream, rwID_CLUMP, 0, 0))
            {
                pResource = RpClumpStreamRead(pStream);
            }
        }
        else if (strcmp(pType, "TEXDICTIONARY") == 0 || strcmp(pType, "PITEXDICTIONARY") == 0)
        {
            if (RwStreamFindChunk(pStream, rwID_TEXDICTIONARY, 0, 0))
            {
                pResource = RwTexDictionaryStreamRead(pStream);
                assert(pResource != 0 && "Failed POST-condition: tex_dictionary");
            }
        }
        else if (strcmp(pType, "WORLD") == 0)
        {
            if (RwStreamFindChunk(pStream, rwID_WORLD, 0, 0))
            {
                pResource = RpWorldStreamRead(pStream);
            }
        }
        else
        {
            // Desteklenmeyen RenderWare türü uyarısı
            assert(false && "CRenderwareResource::Load doesn't support this RenderWare type");
        }

        RwStreamClose(pStream, 0);
        return pResource;
    }

    // ========================================================================
    // RwGfxResource::Unload (FUN_00425360)
    // ASM: 00425360 — Grafik kaynağını serbest bırak
    // ========================================================================
    void RwGfxResource::Unload(void* pResource, const char* pType)
    {
        if (!pResource || !pType) return;

        if (strcmp(pType, "CLUMP") == 0)
        {
            RpClumpDestroy((RpClump*)pResource);
        }
        else if (strcmp(pType, "TEXDICTIONARY") == 0 || strcmp(pType, "PITEXDICTIONARY") == 0)
        {
            RwTexDictionaryDestroy((RwTexDictionary*)pResource);
        }
        else if (strcmp(pType, "WORLD") == 0)
        {
            RpWorldDestroy((RpWorld*)pResource);
        }
    }

    void RwGfxResource::RegisterHandlers()
    {
        // Gfx stream chunk işleyicilerini kaydet
    }
}
