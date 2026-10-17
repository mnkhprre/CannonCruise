// ============================================================================
// CannonCruise - Collision Geometry Generator (CollisionGeometry.cpp)
// Original path: D:\Projects\CannonCruisePC\code\CollisionGeometry.cpp
// Reverse-engineered via Ghidra and CC.asm analysis.
//
// Binary functions:
//   FUN_0046f040 — CCollisionGeometry::CreateShapeFromClump
//
// Backend: Bullet (PhysicsBackend). bConvex=TRUE -> btConvexHullShape (dynamic
// body), FALSE -> btBvhTriangleMeshShape (static world). Tristrip flag
// (rpGEOMETRYTRISTRIP) expands to individual triangles (alternating winding).
//
// DEVIATION: vertex extraction via RpGeometryGetVertices — returns NULL in
// RW stubs; real data flows in librw integration. Atomic transform not
// applied (clump noinstance/single atomic assumption; merge with frame
// matrix later).
// ============================================================================

#include "CollisionGeometry.h"
#include "PhysicsBackend.h"

#include <vector>

#ifndef rpGEOMETRYTRISTRIP
#define rpGEOMETRYTRISTRIP 0x00000001
#endif

namespace
{
    // RpClumpForAllAtomics collector: merges all atomic vertex/triangle
    // lists into single stream (base offset).
    struct ClumpGeomAccum {
        std::vector<RwReal> verts;     // xyz packed, stride 12
        std::vector<RwInt32> indices;  // triples
    };

    RpAtomic *CollectAtomic(RpAtomic *pAtomic, void *pData)
    {
        ClumpGeomAccum *pAcc = static_cast<ClumpGeomAccum *>(pData);
        RpGeometry *pGeo = RpAtomicGetGeometry(pAtomic);
        if (!pGeo)
        {
            return pAtomic;
        }

        const RwInt32 vCount = RpGeometryGetNumVertices(pGeo);
        const RwInt32 tCount = RpGeometryGetNumTriangles(pGeo);
        RwV3d *pVerts = RpGeometryGetVertices(pGeo);
        RwUInt16 *pIdx = RpGeometryGetTriangleIndices(pGeo);
        if (!pVerts || !pIdx || vCount <= 0 || tCount <= 0)
        {
            return pAtomic; // stub/empty geometry — skip
        }

        const RwInt32 base = static_cast<RwInt32>(pAcc->verts.size() / 3);
        for (RwInt32 v = 0; v < vCount; ++v)
        {
            pAcc->verts.push_back(pVerts[v].x);
            pAcc->verts.push_back(pVerts[v].y);
            pAcc->verts.push_back(pVerts[v].z);
        }

        const RwBool bTristrip =
            (RpGeometryGetFlags(pGeo) & rpGEOMETRYTRISTRIP) != 0;
        for (RwInt32 t = 0; t < tCount; ++t)
        {
            const RwUInt16 i0 = pIdx[t * 3 + 0];
            const RwUInt16 i1 = pIdx[t * 3 + 1];
            const RwUInt16 i2 = pIdx[t * 3 + 2];
            pAcc->indices.push_back(base + i0);
            if (bTristrip && (t & 1))
            {
                // strip reverse wrap — winding correction
                pAcc->indices.push_back(base + i2);
                pAcc->indices.push_back(base + i1);
            }
            else
            {
                pAcc->indices.push_back(base + i1);
                pAcc->indices.push_back(base + i2);
            }
        }
        return pAtomic;
    }
} // namespace

void* CCollisionGeometry::CreateShapeFromClump(RpClump* pClump, RwBool bConvex)
{
    if (!pClump) return 0;

    // Reminder: when using DFF for collision, set "noinstance" flag
    ClumpGeomAccum acc;
    RpClumpForAllAtomics(pClump, CollectAtomic, &acc);
    if (acc.indices.empty() || acc.verts.empty())
    {
        return 0; // veri yok (RW stub durumu)
    }

    if (bConvex)
    {
        // dynamic body path — convex hull (vertex stream copied)
        return PhysicsShape_CreateConvexHull(&acc.verts[0],
                                             static_cast<RwInt32>(acc.verts.size() / 3),
                                             3 * static_cast<RwInt32>(sizeof(RwReal)));
    }

    // static world path — triangle mesh
    PhysicsTriMeshDesc desc;
    desc.pVertices = &acc.verts[0];
    desc.vertexCount = static_cast<RwInt32>(acc.verts.size() / 3);
    desc.vertexStride = 3 * static_cast<RwInt32>(sizeof(RwReal));
    desc.pIndices = &acc.indices[0];
    desc.indexCount = static_cast<RwInt32>(acc.indices.size());
    desc.pos[0] = desc.pos[1] = desc.pos[2] = 0.0f;
    return PhysicsShape_CreateTriMesh(&desc);
}

void* CCollisionGeometry::CreateBoxShape(const RwV3d* pHalfExtents)
{
    if (!pHalfExtents) return 0;
    // RwV3d {x,y,z} <-> RwReal[3] same layout
    return PhysicsShape_CreateBox(reinterpret_cast<const RwReal*>(pHalfExtents));
}

void* CCollisionGeometry::CreateSphereShape(RwReal radius)
{
    return PhysicsShape_CreateSphere(radius);
}

void CCollisionGeometry::DestroyShape(void* pShape)
{
    PhysicsShape_Destroy(pShape);
}
