#ifndef CANNONCRUISE_WORLDGEOMETRY_H
#define CANNONCRUISE_WORLDGEOMETRY_H

// ============================================================================
// CannonCruise - WorldGeometry decode + clump instantiate (WorldGeometry.h)
// Faz 4.6o: wbID_WORLDGEOMETRY asset payload düzeni (VIKING01/IDUNA03/
// BEAVER01 hexdump + binary FUN_004c9b70 analizi ile çözüldü):
//
//   [u32 prefixSectionLength][u32 libraryId=0x1803FFFF]
//   [prefix: STRUCT(16B: u32 master0,0,5,0) + GEOMETRY + EXTENSION]
//     — prefix uzunluğu = ilk ATOMIC chunk header'ına ofset
//   [ATOMIC × N]  — her biri: STRUCT(16B: u32 artifact,0,u32 flags=5,0)
//                   + GEOMETRY (gömülü, d3d8-native OLMAYAN standart RW
//                   verisi — flags TEXTURED2|PRELIT|...) + EXTENSION
//   [trailing chunk — tipe göre dünya varyantı: 0x22/0x1E/0x39]
//
// Binary: FUN_004c9b70 (atomic döngüsü; header type==0x14 ATOMIC assert),
// FUN_004dbdb0 (sektör/atomic okuyucu). Gömülü geometri librw
// Geometry::streamRead ile çözülür (version 0x36003 → native OLMAYAN
// standart akış; MATLIST d3d8 IMAGE'leri raster create'e bağlı —
// başarısız atomic'ler atlanır).
// ============================================================================

#include "rwcore.h"

/// Decode edilmiş world geometry envanteri (test fixture kaynakları).
struct WorldGeometryInfo {
    RwUInt32 prefixSectionLength; ///< [0] — ilk ATOMIC ofseti
    RwUInt32 libraryId;           ///< [4] — 0x1803FFFF (RW 3.6)
    RwUInt32 masterStruct[4];     ///< STRUCT@8 body (ilk: 25115 — bilgi)
    RwUInt32 atomicCount;         ///< ATOMIC chunk sayısı
    RwUInt32 totalVertices;       ///< gömülü geometriler toplam vertice
    RwUInt32 totalTriangles;      ///< gömülü geometriler toplam üçgen
    RwUInt32 trailingChunkType;   ///< atomikler sonrası ilk chunk (0 = yok)
};

/// Son CreateClump'un dünya bbox'ı (tüm atomic vertice'leri — auto-framing)
struct WorldGeometryBounds {
    RwReal min[3];  ///< bbox köşesi (en küçük)
    RwReal max[3];  ///< bbox köşesi (en büyük)
    RwBool valid;   ///< TRUE ise en az bir vertice işlendi
};

/**
 * @brief Son WorldGeometry_CreateClump'un hesapladığı bbox'ı döndürür.
 * @param pOut Çıktı (NULL güvenli).
 * @return TRUE ise bbox geçerli.
 */
RwBool WorldGeometry_GetLastBounds(WorldGeometryBounds *pOut);

/**
 * @brief Clump geometrilerini teşhis eder (renksizlik kök nedeni).
 *        İlk N atomic: flags (PRELIT/TEXTURED), vertex rengi varlığı +
 *        örneği, malzeme rengi + doku adı + çözüldü mü. stdout.
 */
void WorldGeometry_Diagnose(const void *pClump, int maxAtomics);

/**
 * @brief Payload yapısını yürür ve doğrular (okuma yok — gezinme).
 *
 * Her ATOMIC: STRUCT(16) + GEOMETRY(STRUCT başlığından flags/tris/verts
 * peek) + EXTENSION; zincir bozulursa/taşarsa FALSE.
 *
 * @param pBuf  Asset payload (RwStudioAssetReadResource çıktısı).
 * @param len   Payload uzunluğu.
 * @param pInfo Çıktı envanter (NULL olabilir — yalnız doğrulama).
 * @return TRUE yapı sağlam; FALSE bozuk/desteklenmeyen.
 */
RwBool WorldGeometry_Inspect(const void *pBuf, RwUInt32 len,
                             WorldGeometryInfo *pInfo);

/**
 * @brief Atomic'leri clump'a instantiate eder (identity frame'ler).
 *
 * Her ATOMIC gömülü GEOMETRY'sini rw::Geometry::streamRead ile çözer;
 * başarısız geometri (örn. d3d8 native MATLIST) o atomic'i ATLANIR.
 * En az bir atomic başarılıysa clump döner.
 *
 * @return rw::Clump* (sahiplik çağırıcıda — DestroyClump) veya NULL.
 */
void *WorldGeometry_CreateClump(const void *pBuf, RwUInt32 len);

/**
 * @brief Clump + frame hiyerarşisini yok eder (NULL güvenli).
 */
void WorldGeometry_DestroyClump(void *pClump);

#endif // CANNONCRUISE_WORLDGEOMETRY_H
