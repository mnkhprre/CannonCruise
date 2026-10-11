// ============================================================================
// CannonCruise - Statik Dünya Çarpışma Verisi (.col / STATICWORLDCOLLISION)
//
// Binary fonksiyonları:
//   FUN_00438eb0 — CPhysicsResource loader (vtable 0x5EF908 slot[3];
//                  tip karşılaştırması wbID_STATICWORLDCOLLISION)
//   FUN_0043bfa0 — çarpışma nesnesi ctor'ı (48B başlık + section + arrays)
//   sub_58BEF0   — section okuyucu (5×f32 başlık + N bayt blob)
//   FUN_00439530 — IsType (slot[2]); FUN_00439460 — Unload (slot[4])
//
// Ctor akışı (asm, esp kayması giderilmiş):
//   48B başlık oku; count1=[+08] (vertexCount), count2=[+18] (triCount),
//   flag bayt=[+10] (2 → u32 index + f32 vert); count1/count2==0 → boş.
//   sub_58BEF0 → section; flag==2 ise count2×3 u32 index (this+0xC),
//   sonra count1×3 f32 vert (this+0x14). Ardından sürtünme 0.1 /
//   sekans materyal nesnesi kurulur (trimesh portu için gerekmez).
//
// Ctor başlığı 0x0716 asset payload'ının embA alanından (payload−8)
// başlar; bu modül doğrudan payload üzerinde eşdeğer düzeni okur:
//
//   +00 u32 vertexCount          (+04 bilinmeyen; dünya başına değişir)
//   +08 u8  flag (=2; +09..0B bilinmeyen)
//   +0C u32 (hep 12)
//   +10 u32 triangleCount
//   +14..+27 sıfır
//   +28 f32 bboxMin.xyz, f32 bilinmeyen, u32 sectionBytes (f32 bit deseni)
//   +3C section blob (sectionBytes; materyal/primitif — şimdilik opak)
//   Sonra: u32 indices[triangleCount×3]  (vertex indeksleri, < vertexCount)
//          f32 verts[vertexCount×3]      (dünya koordinat, float3)
//   Kuyruk: 0–4 bayt hizalama dolgusu (3 gerçek dünyada slack 0/1/2).
//
// Doğrulanan gerçek veriler (Streams/*.RWS):
//   VIKING01  vc=38391 tc=71303 sect=753474  bboxMin≈(-475.77,-9.08,-333.67)
//   IDUNA03   vc=6726  tc=11592 sect=113955  v0≈(56.2,6.5,4.6)
//   BEAVER01  vc=55902 tc=102833 sect=1064352 v0≈(-144.4,11.4,2.9)
//   Boş dünya (MAINMENU/GLOBAL/...): countA=0 → decode FALSE.
// ============================================================================

#ifndef CANNONCRUISE_STATICCOLLISIONDATA_H
#define CANNONCRUISE_STATICCOLLISIONDATA_H

#include "rwcore.h"

/**
 * @struct StaticCollisionMesh
 * @brief Decode edilmiş statik dünya çarpışma mesh'i.
 *
 * İşaretçiler payload tamponu içinedir; tampon ömrü mesh ömründen uzun
 * olmalıdır (PhysicsShape_CreateTriMesh veriyi kopyalar).
 */
struct StaticCollisionMesh {
    RwUInt32 vertexCount;  ///< [+00]
    RwUInt32 triangleCount; ///< [+10]
    RwUInt32 headerUnknown; ///< [+04] dünya başına değişen bilinmeyen
    RwReal bboxMin[3];      ///< [+28] świat sınırı alt köşe
    const RwUInt8 *pSectionBlob; ///< [+3C] opak section verisi
    RwUInt32 sectionBytes;       ///< section blob boyu
    const RwInt32 *pIndices; ///< triangleCount×3 vertex indeksi (u32)
    const RwReal *pVertices; ///< vertexCount×3 float3 konum
    RwUInt32 totalBytes;     ///< header+section+indices+verts (+pad)
};

/**
 * @brief .col payload'ını decode eder (FUN_0043bfa0 düzeni).
 *
 * Başlık/section/sayılar doğrulanır; flag!=2 veya sınırlar aşılırsa
 * FALSE (boş dünya yolu: vertexCount==0 dahil).
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
