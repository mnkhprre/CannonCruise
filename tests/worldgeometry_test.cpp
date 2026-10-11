// ============================================================================
// CannonCruise - WorldGeometry decode + instantiate testi
// Faz 4.6o: üç gerçek dünya RWS (VIKING01/IDUNA03/BEAVER01) envanter
// sayıları (binary FUN_004c9b70 + hexdump çözümü) + VIKING01 clump
// instantiate (gömülü geometri streamRead) + bozuk girdi.
// Çalıştırma: worldgeometry_test <VIKING01> <IDUNA03> <BEAVER01>
// ============================================================================

#include "WorldGeometry.h"

#include "RwStudioStream.h"
#include "rwcore.h"

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
        pPayload = static_cast<unsigned char *>(
            std::malloc(asset.resourceLength));
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

struct Expected {
    const char *name;
    RwUInt32 preLen;
    RwUInt32 atomics;
    RwUInt32 verts;
    RwUInt32 tris;
    RwUInt32 trailing;
};

int main(int argc, char **argv) {
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
        {"VIKING01", 61405, 96, 69657, 70190, 0x22},
        {"IDUNA03", 4285, 31, 10074, 11226, 0x1E},
        {"BEAVER01", 2219, 119, 105358, 107669, 0x39},
    };

    unsigned char *pVikingPayload = nullptr;
    unsigned int vikingLen = 0;

    for (int i = 0; i < 3; ++i) {
        unsigned int len = 0;
        unsigned char *pPayload =
            LoadWorldGeometryPayload(argv[1 + i], &len);
        CHECK(pPayload != nullptr);
        if (!pPayload) {
            continue;
        }
        std::printf("%s: payload %u bytes\n", kExpected[i].name, len);

        WorldGeometryInfo info;
        CHECK(WorldGeometry_Inspect(pPayload, len, &info) == TRUE);
        CHECK(info.libraryId == 0x1803FFFF);
        CHECK(info.prefixSectionLength == kExpected[i].preLen);
        CHECK(info.atomicCount == kExpected[i].atomics);
        CHECK(info.totalVertices == kExpected[i].verts);
        CHECK(info.totalTriangles == kExpected[i].tris);
        CHECK(info.trailingChunkType == kExpected[i].trailing);
        CHECK(info.masterStruct[2] == 5); // STRUCT@8 üçüncü kelime

        if (i == 0) {
            pVikingPayload = pPayload; // clump testi için sakla
            vikingLen = len;
        } else {
            std::free(pPayload);
        }
    }

    // Clump instantiate — VIKING01 (en zengin dünya)
    if (pVikingPayload) {
        void *pClump = WorldGeometry_CreateClump(pVikingPayload, vikingLen);
        CHECK(pClump != nullptr);
        if (pClump) {
            // En az bir atomic (başarılı geometri) — sayı bilinmiyor
            // (d3d8 MATLIST bağımlı) → en az 1 yeterli kanıt.
            // 4.6q — bbox: VIKING01 bilinen aralık (min.y<0, max.y~124).
            WorldGeometryBounds bb;
            CHECK(WorldGeometry_GetLastBounds(&bb) == TRUE);
            CHECK(bb.min[0] < -400.0f && bb.max[0] > 400.0f);
            CHECK(bb.min[1] < 0.0f && bb.max[1] > 100.0f);
            CHECK(bb.min[2] < -300.0f && bb.max[2] > 300.0f);
            WorldGeometry_DestroyClump(pClump);
        }

        // Bozuk girdi: prefix libid yanlış → FALSE / NULL
        pVikingPayload[4] = 0x00;
        WorldGeometryInfo bad;
        CHECK(WorldGeometry_Inspect(pVikingPayload, vikingLen, &bad) == FALSE);
        CHECK(WorldGeometry_CreateClump(pVikingPayload, vikingLen) == nullptr);
        pVikingPayload[4] = 0xFF; // geri al

        // Truncated → FALSE
        CHECK(WorldGeometry_Inspect(pVikingPayload, 16, &bad) == FALSE);
        CHECK(WorldGeometry_Inspect(nullptr, vikingLen, &bad) == FALSE);

        std::free(pVikingPayload);
    }

    WorldGeometry_DestroyClump(nullptr); // NULL güvenli

    RwEngineStop();
    RwEngineClose();
    RwEngineTerm();

    std::printf("worldgeometry_test: %d checks, %d failures\n", s_checks,
                s_failures);
    return (s_failures == 0) ? 0 : 1;
}
