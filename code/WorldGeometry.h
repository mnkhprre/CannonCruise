#ifndef CANNONCRUISE_WORLDGEOMETRY_H
#define CANNONCRUISE_WORLDGEOMETRY_H

// ============================================================================
// CannonCruise - WorldGeometry decode + clump instantiate (WorldGeometry.h)
// Phase 4.6o: wbID_WORLDGEOMETRY asset payload layout (solved via VIKING01/
// IDUNA03/BEAVER01 hexdump + binary FUN_004c9b70 analysis):
//
//   [u32 prefixSectionLength][u32 libraryId=0x1803FFFF]
//   [prefix: STRUCT(16B: u32 master0,0,5,0) + GEOMETRY + EXTENSION]
//     — prefix length = offset to first ATOMIC chunk header
//   [ATOMIC × N]  — each: STRUCT(16B: u32 artifact,0,u32 flags=5,0)
//                   + GEOMETRY (embedded, NON-d3d8-native standard RW
//                   data — flags TEXTURED2|PRELIT|...) + EXTENSION
//   [u32 count2][ATOMIC × count2]  ← Phase 4.6s: the "trailing chunk" section
//                   we called is actually the SECOND ATOMIC group;
//                   0x22/0x1E/0x39 is NOT a chunk TYPE but group2 atomic COUNT
//                   (34/30/57). (Old code only read the type and never
//                   instantiated the group → VIKING01 had 34 atomics /
//                   8489 vertices not rendered.) End has 2-3 bytes asset
//                   padding.
//
// Binary: FUN_004c9b70 (atomic loop; header type==0x14 ATOMIC assert),
// FUN_004dbdb0 (sector/atomic reader). Embedded geometry resolved via
// librw Geometry::streamRead (version 0x36003 → NON-native standard stream;
// MATLIST d3d8 IMAGE's depend on raster create — failed atomics skipped).
// ============================================================================

#include "rwcore.h"

/// Decoded world geometry inventory (test fixture sources).
struct WorldGeometryInfo {
    RwUInt32 prefixSectionLength; ///< [0] — offset to first ATOMIC
    RwUInt32 libraryId;           ///< [4] — 0x1803FFFF (RW 3.6)
    RwUInt32 masterStruct[4];     ///< STRUCT@8 body (first: 25115 — info)
    RwUInt32 atomicCount;         ///< ATOMIC chunk count
    RwUInt32 totalVertices;       ///< embedded geometries total vertices
    RwUInt32 totalTriangles;      ///< embedded geometries total triangles
    RwUInt32 trailingChunkType;   ///< First u32 AFTER ATOMICs = GROUP2 COUNT
                                  ///< (0x22/0x1E/0x39 = 34/30/57 — was thought
                                  ///< to be "trailing chunk" in Phase 4.6s;
                                  ///< actually FUN_004c9b70's second
                                  ///< [count][ATOMIC×N])
    RwUInt32 trailingChunkOffset; ///< trailing header offset (payload start 0)
    RwUInt32 trailingChunkLength; ///< trailing body length (0 = overflow/bad)
    RwUInt32 trailingLibraryId;   ///< trailing header libid (RW 3.6 = 0x1803FFFF)
    RwUInt32 atomicGroup2Offset;  ///< [u32 count2] offset (group2 start)
    RwUInt32 atomicGroup2Count;   ///< second ATOMIC group count (0 = none)
    RwUInt32 group2Vertices;      ///< group2 total vertices (peek; not decoded)
    RwUInt32 group2Triangles;     ///< group2 total triangles (peek; not decoded)
};

/// Last CreateClump's world bbox (all atomic vertices — auto-framing)
struct WorldGeometryBounds {
    RwReal min[3];  ///< bbox corner (min)
    RwReal max[3];  ///< bbox corner (max)
    RwBool valid;   ///< TRUE if at least one vertex processed
};

/**
 * @brief Returns bbox computed by last WorldGeometry_CreateClump.
 * @param pOut Output (NULL-safe).
 * @return TRUE if bbox is valid.
 */
RwBool WorldGeometry_GetLastBounds(WorldGeometryBounds *pOut);

/**
 * @brief Diagnose clump geometries (root cause of colorlessness).
 *        First N atomic: flags (PRELIT/TEXTURED), vertex color presence +
 *        instance, material color + texture name + resolved. stdout.
 */
void WorldGeometry_Diagnose(const void *pClump, int maxAtomics);

/**
 * @brief Walks and validates payload structure (no read — traversal only).
 *
 * Each ATOMIC: STRUCT(16) + GEOMETRY (STRUCT header flags/tris/verts peek)
 * + EXTENSION; returns FALSE if chain corrupt/overflows.
 *
 * @param pBuf  Asset payload (RwStudioAssetReadResource output).
 * @param len   Payload length.
 * @param pInfo Output inventory (may be NULL — validation only).
 * @return TRUE structure sound; FALSE corrupt/unsupported.
 */
RwBool WorldGeometry_Inspect(const void *pBuf, RwUInt32 len,
                             WorldGeometryInfo *pInfo);

/**
 * @brief Instantiates atomics into clump (identity frames).
 *
 * Each ATOMIC's embedded GEOMETRY resolved via rw::Geometry::streamRead;
 * failed geometry (e.g. d3d8 native MATLIST) skips that atomic.
 * Returns clump if at least one atomic succeeded.
 *
 * @return rw::Clump* (ownership with caller — DestroyClump) or NULL.
 */
void *WorldGeometry_CreateClump(const void *pBuf, RwUInt32 len);

/**
 * @brief Destroys clump + frame hierarchy (NULL-safe).
 */
void WorldGeometry_DestroyClump(void *pClump);

#endif // CANNONCRUISE_WORLDGEOMETRY_H
