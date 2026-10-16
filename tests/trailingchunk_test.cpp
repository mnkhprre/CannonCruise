// ============================================================================
// CannonCruise - WorldGeometry ATOMIC grup decode (trailingchunk_test.cpp)
//
// Faz 4.6s: WorldGeometry payload'unun atomikler SONRASI kalan bölümünün
// çözümü. Faz 4.6o bu bölümü "[trailing chunk — tipe göre dünya varyantı
// 0x22/0x1E/0x39]" olarak kaydetmiş ve yalnızca TİPİ okumuştu; içerik
// çözülmemişti. Ayrıca bu bölüm hiç instantiate edilmiyordu (96 atomic
// render edilirken 34'ü atlanıyordu).
//
// BULGU (dosya verisi + binary FUN_004c9b70 modeli):
//   WorldGeometry payload =
//     [u32 prefixSectionLength][u32 libid=0x1803FFFF]
//     [prefix: STRUCT(16) + GEOMETRY + EXTENSION]
//     [ATOMIC × N]                       ← count YOK; tip!=0x14 olana kadar
//     [u32 count2][ATOMIC × count2]      ← İKİNCİ GRUP (eski "trailing chunk")
//     [2-3 bayt asset padding]
//   FUN_004c9b70: "u32 count → tekrar: chunk header (assert type==0x14) →
//   sektör/atomic oku (FUN_004dbdb0) → dünyaya ekle".
//   Yani 0x22/0x1E/0x39 bir "tip" DEĞİL, grup2 atomic SAYISI (34/30/57).
//
// Üç dünya envanteri sabittir ve birebir doğrulanır.
// ============================================================================

#include "WorldGeometry.h"

#include "RwStudioStream.h"
#include "rwcore.h"

#include <rw.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>

static int s_checks = 0;
static int s_failures = 0;

#define CHECK(cond)                                                            \
    do {                                                                       \
        ++s_checks;                                                            \
        if (!(cond)) {                                                         \
            ++s_failures;                                                      \
            std::printf("FAIL line %d: %s\n", __LINE__, #cond);                \
        }                                                                      \
    } while (0)

/// RWS'ten WorldGeometry payload'unu okur (malloc; çağırıcı free).
static unsigned char *LoadWorldGeometryPayload(const char *pPath,
                                               unsigned int *pLen) {
    RwStudioFile *pFile = RwStudioFileOpen(pPath);
    if (!pFile) {
        std::fprintf(stderr, "open failed: %s\n", pPath);
        return nullptr;
    }
    unsigned char *pPayload = nullptr;
    RwStudioAsset asset;
    while (RwStudioFileNextAsset(pFile, &asset)) {
        if (std::strcmp(asset.typeName, "WorldGeometry") != 0) {
            continue;
        }
        if (asset.resourceLength == 0) {
            break;
        }
        pPayload =
            static_cast<unsigned char *>(std::malloc(asset.resourceLength));
        if (!pPayload) {
            break;
        }
        const RwUInt32 got = RwStudioAssetReadResource(
            pFile, &asset, pPayload, asset.resourceLength);
        if (got != asset.resourceLength) {
            std::free(pPayload);
            pPayload = nullptr;
        }
        *pLen = asset.resourceLength;
        break;
    }
    RwStudioFileClose(pFile);
    return pPayload;
}

static unsigned int U32(const unsigned char *p) {
    unsigned int v;
    std::memcpy(&v, p, 4);
    return v;
}

/// Clump içindeki atomic sayısı (grup1 + grup2 gerçekten eklendi mi?).
static int CountClumpAtomics(void *pClumpPtr) {
    if (pClumpPtr == nullptr) {
        return -1;
    }
    rw::Clump *pClump = static_cast<rw::Clump *>(pClumpPtr);
    return static_cast<int>(pClump->countAtomics());
}

struct Expected {
    const char *pName;
    unsigned int group1;   ///< ilk ATOMIC grubu (count yok)
    unsigned int group2;   ///< [u32 count2] — eski "trailing chunk type"
    unsigned int group2V;  ///< grup2 vertice
    unsigned int group2T;  ///< grup2 üçgen
};

int main(int argc, char **argv) {
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    if (argc < 4) {
        std::fprintf(stderr,
                     "usage: %s <VIKING01.RWS> <IDUNA03.RWS> <BEAVER01.RWS>\n",
                     argv[0]);
        return 2;
    }

    if (RwEngineInit(0, 0, 0) != TRUE || RwEngineOpen() != TRUE ||
        RwEngineStart() != TRUE) {
        std::fprintf(stderr, "engine init failed\n");
        return 2;
    }

    static const Expected kExpected[3] = {
        {"VIKING01", 96, 34, 8489, 12297},
        {"IDUNA03", 31, 30, 432, 258},
        {"BEAVER01", 119, 57, 10422, 6806},
    };

    for (int i = 0; i < 3; ++i) {
        unsigned int len = 0;
        unsigned char *pPayload = LoadWorldGeometryPayload(argv[1 + i], &len);
        CHECK(pPayload != nullptr);
        if (!pPayload) {
            continue;
        }

        WorldGeometryInfo info;
        std::memset(&info, 0, sizeof(info));
        CHECK(WorldGeometry_Inspect(pPayload, len, &info) == TRUE);

        // grup1: Faz 4.6o'dan beri bilinen değer (değişmedi)
        CHECK(info.atomicCount == kExpected[i].group1);
        // grup2: Faz 4.6s çözümü
        CHECK(info.atomicGroup2Count == kExpected[i].group2);
        CHECK(info.group2Vertices == kExpected[i].group2V);
        CHECK(info.group2Triangles == kExpected[i].group2T);

        // 0x22/0x1E/0x39 bir "tip" değil, grup2 SAYISI → aynı u32
        CHECK(info.trailingChunkType == kExpected[i].group2);
        CHECK(info.trailingChunkType == info.atomicGroup2Count);
        CHECK(info.atomicGroup2Offset == info.trailingChunkOffset);
        // grup2, grup1'in hemen ardından gelir
        CHECK(info.atomicGroup2Offset > 8 + info.prefixSectionLength);

        std::printf("%s: group1=%u group2=%u (+%u verts/+%u tris) "
                    "group2Offset=%u\n",
                    kExpected[i].pName, info.atomicCount,
                    info.atomicGroup2Count, info.group2Vertices,
                    info.group2Triangles, info.atomicGroup2Offset);

        // ham u32 ile bağımsız doğrulama (Inspect'a güvenmeden)
        CHECK(U32(pPayload + info.atomicGroup2Offset) ==
              kExpected[i].group2);
        CHECK(U32(pPayload + info.atomicGroup2Offset + 4) == 0x14);

        // CreateClump: İKİ grubu da instantiate etmeli
        void *pClump = WorldGeometry_CreateClump(pPayload, len);
        CHECK(pClump != nullptr);
        const int nAtomics = CountClumpAtomics(pClump);
        CHECK(nAtomics == static_cast<int>(kExpected[i].group1 +
                                           kExpected[i].group2));
        WorldGeometryBounds bounds;
        std::memset(&bounds, 0, sizeof(bounds));
        CHECK(WorldGeometry_GetLastBounds(&bounds) == TRUE);
        CHECK(bounds.valid == TRUE);
        std::printf("  clump atomics=%d (beklenen %u) bounds "
                    "(%.0f..%.0f, %.0f..%.0f, %.0f..%.0f)\n",
                    nAtomics, kExpected[i].group1 + kExpected[i].group2,
                    (double)bounds.min[0], (double)bounds.max[0],
                    (double)bounds.min[1], (double)bounds.max[1],
                    (double)bounds.min[2], (double)bounds.max[2]);
        WorldGeometry_DestroyClump(pClump);

        std::free(pPayload);
    }

    std::printf("trailingchunk_test: %d checks, %d failures\n", s_checks,
                s_failures);
    if (s_failures != 0) {
        return 1;
    }
    std::printf("trailingchunk_test: OK\n");
    return 0;
}
