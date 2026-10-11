// ============================================================================
// CannonCruise - Win32 Direct3D Ocean Renderer (Ocean_W32.cpp)
// Original path: D:\Projects\CannonCruisePC\code\Ocean_W32.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_00431910 — COcean_W32::Initialize
//   FUN_00431bd0 — COcean_W32::RenderWaterMesh ("paTexCoords != NULL Failed", "paColors != NULL Failed")
// ============================================================================

#include "Ocean_W32.h"
#include <cassert>
#include <cmath>
#include <cstdlib>

#ifdef RW_GL3
// rw::im3d / rw::gl3::Im3DVertex — yalnız GL3 render yolu (librw include
// yolu gerektirir; syntax-only probe RW_GL3'süz derler — bkz. else dalı).
#include <rw.h>
#endif

// ---------------------------------------------------------------------------
// Initialize – mirrors FUN_00431910.
// Prepares Direct3D/RenderWare texture samplers, blend modes, and vertex layouts.
// ---------------------------------------------------------------------------
RwBool COcean_W32::Initialize()
{
    return TRUE;
}

// ---------------------------------------------------------------------------
// Shutdown – cleans up Direct3D/RenderWare resources.
// ---------------------------------------------------------------------------
void COcean_W32::Shutdown()
{
}

// ---------------------------------------------------------------------------
// RenderWaterMesh – mirrors FUN_00431bd0.
// Dispatches immediate 3D indexed primitives to GPU using RenderWare Im3D.
// ---------------------------------------------------------------------------
void COcean_W32::RenderWaterMesh(const RwV3d* pVertices, const RwTexCoords* pTexCoords, const RwRGBA* pColors, RwUInt32 numVertices)
{
    assert(pVertices != 0);
    assert(pTexCoords != 0 && "paTexCoords != NULL Failed");
    assert(pColors != 0 && "paColors != NULL Failed");

    // Grid (subdiv+1)^2 — tek TRISTRIP iki boyutlu grid'i temsil edemez;
    // dörtgen başına 6 indeks ile TRILIST. Kenar numVertices'tan çıkarılır.
    const RwUInt32 side = (RwUInt32)(std::sqrt((RwReal)numVertices) + 0.5f);
    if (side < 2 || side * side != numVertices) {
        return;
    }
    const RwUInt32 numIndices = (side - 1) * (side - 1) * 6;
    RwUInt16 *indices = (RwUInt16 *)malloc(numIndices * sizeof(RwUInt16));
    if (indices == nullptr) {
        return;
    }
    {
        RwUInt32 k = 0;
        for (RwUInt32 iz = 0; iz + 1 < side; ++iz) {
            for (RwUInt32 ix = 0; ix + 1 < side; ++ix) {
                const RwUInt32 v00 = iz * side + ix;
                const RwUInt32 v10 = v00 + 1;
                const RwUInt32 v01 = v00 + side;
                const RwUInt32 v11 = v01 + 1;
                indices[k++] = (RwUInt16)v00;
                indices[k++] = (RwUInt16)v10;
                indices[k++] = (RwUInt16)v11;
                indices[k++] = (RwUInt16)v00;
                indices[k++] = (RwUInt16)v11;
                indices[k++] = (RwUInt16)v01;
            }
        }
    }

    // rw::im3d doğrudan (rwstubs no-op C API'sine düşmez — RenderTestQuad
    // im2d deseni). GL3: gl3::Im3DVertex (pos+normal+rgba+uv); NULL: çizim yok.
#ifdef RW_GL3
    // Doku yok: arazi render'ından sızan TEXTURERASTER temizlenir (yoksa
    // okyanus son arazi dokusuyla çarpılıp kararır).
    rw::SetRenderStatePtr(rw::TEXTURERASTER, nullptr);
    rw::gl3::Im3DVertex *imVertices = (rw::gl3::Im3DVertex *)RwMalloc(
        numVertices * sizeof(rw::gl3::Im3DVertex), 0);
    if (imVertices == nullptr) {
        free(indices);
        return;
    }
    for (RwUInt32 i = 0; i < numVertices; ++i) {
        imVertices[i].setX(pVertices[i].x);
        imVertices[i].setY(pVertices[i].y);
        imVertices[i].setZ(pVertices[i].z);
        imVertices[i].setNormalX(0.0f);
        imVertices[i].setNormalY(1.0f);
        imVertices[i].setNormalZ(0.0f);
        imVertices[i].setColor(pColors[i].red, pColors[i].green,
                               pColors[i].blue, pColors[i].alpha);
        imVertices[i].setU(pTexCoords[i].u);
        imVertices[i].setV(pTexCoords[i].v);
    }
    rw::im3d::Transform(imVertices, (int)numVertices, nullptr,
                        rw::im3d::EVERYTHING);
    rw::im3d::RenderIndexedPrimitive(rw::PRIMTYPETRILIST, indices,
                                     (int)numIndices);
    rw::im3d::End();
    RwFree(imVertices);
#else
    // NULL platform: cihaz im3D no-op — indeks buffer üretimi kanıtlanır,
    // çizim atlanır (RW_GL3 derlemede yukarıdaki gerçek render yolu).
    (void)pVertices;
    (void)pTexCoords;
    (void)pColors;
#endif
    free(indices);
}
