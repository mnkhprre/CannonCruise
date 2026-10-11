#ifndef OCEAN_W32_H
#define OCEAN_W32_H

// ============================================================================
// CannonCruise - Win32 Direct3D Ocean Renderer (Ocean_W32.h)
// Original path: D:\Projects\CannonCruisePC\code\Ocean_W32.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include <rwcore.h>

/**
 * @class COcean_W32
 * @brief Platform-specific Win32 / Direct3D immediate-mode renderer for ocean surfaces.
 *
 * Provides static drawing helpers that interface directly with the RenderWare
 * Immediate Mode 3D pipeline (RwIm3D) or Direct3D vertex streams to draw water tiles
 * with custom multi-pass alpha blending and specular water reflection effects.
 */
class COcean_W32
{
public:
    /**
     * @brief Initializes Direct3D render states and shaders for water rendering.
     * @return TRUE on successful initialization.
     */
    static RwBool Initialize();

    /**
     * @brief Restores default graphics states and shuts down water renderer.
     */
    static void Shutdown();

    /**
     * @brief Submits dynamic water vertex arrays to the GPU via immediate 3D mode.
     * @param pVertices    Pointer to array of vertex coordinates.
     * @param pTexCoords   Pointer to array of texture coordinates.
     * @param pColors      Pointer to array of vertex colors / alpha.
     * @param numVertices  Total number of vertices in stream.
     */
    static void RenderWaterMesh(const RwV3d* pVertices, const RwTexCoords* pTexCoords, const RwRGBA* pColors, RwUInt32 numVertices);
};

#endif // OCEAN_W32_H
