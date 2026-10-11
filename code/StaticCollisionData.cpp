// ============================================================================
// CannonCruise - Statik Dünya Çarpışma Verisi (StaticCollisionData.cpp)
// Binary düzeni: FUN_0043bfa0 ctor + sub_58BEF0 section (bkz. başlık).
// ============================================================================

#include "StaticCollisionData.h"

#include "PhysicsBackend.h"

#include <cstring>

namespace {

inline RwUInt32 ReadU32(const RwUInt8 *p) {
    RwUInt32 v;
    std::memcpy(&v, p, 4);
    return v;
}

inline RwReal ReadF32(const RwUInt8 *p) {
    RwReal v;
    std::memcpy(&v, p, 4);
    return v;
}

} // namespace

RwBool StaticCollisionData_Decode(const RwUInt8 *pPayload, RwUInt32 len,
                                  StaticCollisionMesh *pOut) {
    if (!pPayload || !pOut) {
        return FALSE;
    }
    std::memset(pOut, 0, sizeof(*pOut));

    // ctor 48B başlık + section başlığı 5×f32 (en az 0x3C bayt gerekli)
    if (len < 0x3C) {
        return FALSE;
    }

    const RwUInt32 vertexCount = ReadU32(pPayload + 0x00);
    const RwUInt32 headerUnknown = ReadU32(pPayload + 0x04);
    const RwUInt8 flag = pPayload[0x08];
    const RwUInt32 midTwelve = ReadU32(pPayload + 0x0C);
    const RwUInt32 triangleCount = ReadU32(pPayload + 0x10);

    // boş dünya yolu: countA/count2 == 0 → ctor arraysiz skip
    if (vertexCount == 0 || triangleCount == 0) {
        return FALSE;
    }
    // flag==2 yolu: u32 index + f32 vert (üç gerçek dünyada doğrulandı)
    if (flag != 2 || midTwelve != 12) {
        return FALSE;
    }
    // sayısal taşma koruması (count*12 32-bit aralıkta)
    if (vertexCount > 0x14000000u / 12u || triangleCount > 0x14000000u / 12u) {
        return FALSE;
    }

    const RwUInt32 sectionBytes = ReadU32(pPayload + 0x38);
    const RwUInt32 sectionOff = 0x3C;
    if (sectionBytes > len - sectionOff) {
        return FALSE;
    }
    const RwUInt32 indexOff = sectionOff + sectionBytes;
    const RwUInt32 indexBytes = triangleCount * 3u * 4u;
    if (indexBytes > len - indexOff) {
        return FALSE;
    }
    const RwUInt32 vertOff = indexOff + indexBytes;
    const RwUInt32 vertBytes = vertexCount * 3u * 4u;
    if (vertBytes > len - vertOff) {
        return FALSE;
    }
    const RwUInt32 totalBytes = vertOff + vertBytes;
    // kuyruk 0–4 bayt hizalama dolgusu (gerçek verilerde slack 0/1/2)
    if (len - totalBytes > 4) {
        return FALSE;
    }

    pOut->vertexCount = vertexCount;
    pOut->triangleCount = triangleCount;
    pOut->headerUnknown = headerUnknown;
    pOut->bboxMin[0] = ReadF32(pPayload + 0x28);
    pOut->bboxMin[1] = ReadF32(pPayload + 0x2C);
    pOut->bboxMin[2] = ReadF32(pPayload + 0x30);
    pOut->pSectionBlob = pPayload + sectionOff;
    pOut->sectionBytes = sectionBytes;
    pOut->pIndices = reinterpret_cast<const RwInt32 *>(pPayload + indexOff);
    pOut->pVertices = reinterpret_cast<const RwReal *>(pPayload + vertOff);
    pOut->totalBytes = totalBytes;

    // index sınırları (ctor veriye güvener; decode doğrular)
    for (RwUInt32 i = 0; i < triangleCount * 3u; ++i) {
        if (pOut->pIndices[i] < 0 ||
            static_cast<RwUInt32>(pOut->pIndices[i]) >= vertexCount) {
            std::memset(pOut, 0, sizeof(*pOut));
            return FALSE;
        }
    }

    return TRUE;
}

void *StaticCollisionData_CreateShape(const StaticCollisionMesh *pMesh) {
    if (!pMesh || !pMesh->pVertices || !pMesh->pIndices) {
        return 0;
    }
    PhysicsTriMeshDesc desc;
    desc.pVertices = pMesh->pVertices;
    desc.vertexCount = static_cast<RwInt32>(pMesh->vertexCount);
    desc.vertexStride = 12; // 3×RwReal
    desc.pIndices = pMesh->pIndices;
    desc.indexCount = static_cast<RwInt32>(pMesh->triangleCount * 3u);
    desc.pos[0] = desc.pos[1] = desc.pos[2] = 0.0f;
    return PhysicsShape_CreateTriMesh(&desc);
}
