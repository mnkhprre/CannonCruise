// ============================================================================
// CannonCruise - RenderWare Studio Framework (txtresource.cpp)
// Original path: D:\Projects\CannonCruisePC\code\framework\toolkits\txtresource\txtresource.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_00428890 — TxtResource::Load
//   FUN_00428b50 — TxtResource::Unload
// ============================================================================

#include "txtresource.h"
#include <cassert>
#include <cstdio>
#include <cstdlib>

namespace RWS
{
    // ========================================================================
    // TxtResource::Load (FUN_00428890)
    // ASM: 00428890 — TEXT dosyasını oku ve belleğe yükle
    // ========================================================================
    char* TxtResource::Load(const char* pFilename)
    {
        if (!pFilename) return 0;

        FILE* pFile = fopen(pFilename, "rb");
        if (!pFile) return 0;

        fseek(pFile, 0, SEEK_END);
        long fileSize = ftell(pFile);
        fseek(pFile, 0, SEEK_SET);

        char* pBuffer = (char*)malloc(fileSize + 1);
        if (pBuffer)
        {
            fread(pBuffer, 1, fileSize, pFile);
            pBuffer[fileSize] = '\0';
        }

        fclose(pFile);
        return pBuffer;
    }

    // ========================================================================
    // TxtResource::Unload (FUN_00428b50)
    // ========================================================================
    void TxtResource::Unload(char* pTextData)
    {
        if (pTextData)
        {
            free(pTextData);
        }
    }
}
