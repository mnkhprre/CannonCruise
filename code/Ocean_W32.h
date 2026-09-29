#ifndef OCEAN_W32_H
#define OCEAN_W32_H

// ============================================================================
// CannonCruise - Win32 Direct3D Ocean Renderer (Ocean_W32.h)
// Original path: D:\Projects\CannonCruisePC\code\Ocean_W32.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include <rwcore.h>

class COcean_W32
{
public:
    static RwBool Initialize();
    static void Shutdown();
    static void RenderWaterMesh(const RwV3d* pVertices, const RwTexCoords* pTexCoords, const RwRGBA* pColors, RwUInt32 numVertices);
};

#endif // OCEAN_W32_H
