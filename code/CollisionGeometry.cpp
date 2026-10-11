// ============================================================================
// CannonCruise - Collision Geometry Generator (CollisionGeometry.cpp)
// Original path: D:\Projects\CannonCruisePC\code\CollisionGeometry.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_0046f040 — CCollisionGeometry::CreateShapeFromClump
//
// Backend: Bullet (PhysicsBackend). bConvex=TRUE → btConvexHullShape (dinamik
// gövde), FALSE → btBvhTriangleMeshShape (statik dünya). Tristrip bayrağı
// (rpGEOMETRYTRISTRIP) tekil üçgenlere açılır (alternating winding).
//
// SAPMA: vertex çıkarma RpGeometryGetVertices üzerinden — RW stub'larında
// NULL döner; librw entegrasyonunda gerçek veri akar. Atomik transform'u
// uygulamıyoruz (clump'ta noinstance/tek atomic varsayımı; ileride frame
// matrisiyle birleştirme).
// ============================================================================

#include "CollisionGeometry.h"
#include "PhysicsBackend.h"

#include <vector>

#ifndef rpGEOMETRYTRISTRIP
#define rpGEOMETRYTRISTRIP 0x00000001
#endif

namespace
{
    // RpClumpForAllAtomics toplayıcısı: tüm atomic'lerin vertex/triangle
    // listelerini tek akışta birleştirir (base offset'li).
    struct ClumpGeomAccum {
        std::vector<RwReal> verts;     // xyz paketli, stride 12
        std::vector<RwInt32> indices;  // 3'lü
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
            return pAtomic; // stub/boş geometri — atla
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
                // strip ters sarım — winding düzeltmesi
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

    // Hatırlatma: DFF çarpışma için kullanıldığında "noinstance" bayrağını kullanın
    ClumpGeomAccum acc;
    RpClumpForAllAtomics(pClump, CollectAtomic, &acc);
    if (acc.indices.empty() || acc.verts.empty())
    {
        return 0; // veri yok (RW stub durumu)
    }

    if (bConvex)
    {
        // dinamik gövde yolu — konveks hull (vertex akışı kopyalanır)
        return PhysicsShape_CreateConvexHull(&acc.verts[0],
                                             static_cast<RwInt32>(acc.verts.size() / 3),
                                             3 * static_cast<RwInt32>(sizeof(RwReal)));
    }

    // statik dünya yolu — üçgen mesh
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
    // RwV3d {x,y,z} ↔ RwReal[3] aynı düzen
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
