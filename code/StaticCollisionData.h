// ============================================================================
// CannonCruise - Static World Collision Data (.col / STATICWORLDCOLLISION)
//
// Binary functions:
//   FUN_00438eb0 — CPhysicsResource loader (vtable 0x5EF908 slot[3];
//                  type comparison wbID_STATICWORLDCOLLISION)
//   FUN_0043bfa0 — collision object ctor (48B header + section + arrays)
//   sub_58BEF0   — section reader (5×f32 header + N-byte blob)
//   FUN_00439530 — IsType (slot[2]); FUN_00439460 — Unload (slot[4])
//
// Ctor flow (asm, esp shift cleared):
//   Read 48B header; count1=[+08] (vertexCount), count2=[+18] (triCount),
//   flag byte=[+10] (2 -> u32 index + f32 vert); count1/count2==0 -> empty.
//   sub_58BEF0 -> section; flag==2 means count2×3 u32 index (this+0xC),
//   then count1×3 f32 vert (this+0x14). Then friction 0.1 /
//   sequence material object set up (not needed for trimesh port).
//
// Ctor header starts at embA field of 0x0716 asset payload (payload−8);
// this module reads equivalent layout directly from payload:
//
//   +00 u32 vertexCount          (+04 unknown; varies per world)
//   +08 u8  flag (=2; +09..0B unknown)
//   +0C u32 (always 12)
//   +10 u32 triangleCount
//   +14..+27 zero
//   +28 f32 bboxMin.xyz, f32 unknown, u32 sectionBytes (f32 bit pattern)
//   +3C section blob (sectionBytes; material/primitive — opaque for now)
//   Then: u32 indices[triangleCount×3]  (vertex indices, < vertexCount)
//          f32 verts[vertexCount×3]      (world coord, float3)
//   Tail: 0–4 byte alignment padding (3 real-world slack 0/1/2).
//
// Verified real data (Streams/*.RWS):
//   VIKING01  vc=38391 tc=71303 sect=753474  bboxMin≈(-475.77,-9.08,-333.67)
//   IDUNA03   vc=6726  tc=11592 sect=113955  v0≈(56.2,6.5,4.6)
//   BEAVER01  vc=55902 tc=102833 sect=1064352 v0≈(-144.4,11.4,2.9)
//   Empty world (MAINMENU/GLOBAL/...): countA=0 -> decode FALSE.
// ============================================================================

#ifndef CANNONCRUISE_STATICCOLLISIONDATA_H
#define CANNONCRUISE_STATICCOLLISIONDATA_H

#include "rwcore.h"

/**
 * @struct StaticCollisionMesh
 * @brief Decoded static world collision mesh.
 *
 * Pointers are into payload buffer; buffer lifetime must exceed mesh
 * lifetime (PhysicsShape_CreateTriMesh copies data).
 */
struct StaticCollisionMesh {
    RwUInt32 vertexCount;  ///< [+00]
    RwUInt32 triangleCount; ///< [+10]
    RwUInt32 headerUnknown; ///< [+04] unknown, varies per world
    RwReal bboxMin[3];      ///< [+28] world bounds bottom corner
    const RwUInt8 *pSectionBlob; ///< [+3C] opaque section data
    RwUInt32 sectionBytes;       ///< section blob size
    const RwInt32 *pIndices; ///< triangleCount×3 vertex index (u32)
    const RwReal *pVertices; ///< vertexCount×3 float3 position
    RwUInt32 totalBytes;     ///< header+section+indices+verts (+pad)
};

/**
 * @brief Decodes .col payload (FUN_0043bfa0 layout).
 *
 * Header/section/counts validated; flag!=2 or bounds exceeded returns
 * FALSE (empty world path: vertexCount==0 included).
 */
RwBool StaticCollisionData_Decode(const RwUInt8 *pPayload, RwUInt32 len,
                                  StaticCollisionMesh *pOut);

/**
 * @brief Mesh'ten bağımsız çarpışma shape'i üretir
 * (PhysicsShape_CreateTriMesh — btBvhTriangleMeshShape).
 * shape çağırının malıdır; PhysicsShape_Destroy ile serbest bırakılır.
 */
void *StaticCollisionData_CreateShape(const StaticCollisionMesh *pMesh);

#endif // CANNONCRUISE_STATICCOLLISIONDATA_H
