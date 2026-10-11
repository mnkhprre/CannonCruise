// ============================================================================
// CannonCruise - Statik dünya çarpışma (.col) decode + trimesh shape testi
// Gerçek Streams/*.RWS wbID_STATICWORLDCOLLISION payload'ları FUN_0043bfa0
// düzenine göre decode edilip sayı/bbox/shape doğrulanır.
// Çalıştırma: coltrimesh_test <VIKING01.RWS> <IDUNA03.RWS> <BEAVER01.RWS>
// ============================================================================

#include "StaticCollisionData.h"

#include "PhysicsBackend.h"
#include "RwStudioStream.h"

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

static bool NearEq(float a, float b, float eps) {
    float d = a - b;
    if (d < 0) {
        d = -d;
    }
    return d <= eps;
}

// Tek bir RWS'ten STATICWORLDCOLLISION payload'unu bulur; decode + shape.
static void CheckWorld(const char *pPath, RwUInt32 expectVc, RwUInt32 expectTc,
                       RwUInt32 expectSect, float expectMinX,
                       float expectMinY, float expectMinZ) {
    RwStudioFile *pFile = RwStudioFileOpen(pPath);
    CHECK(pFile != 0);
    if (!pFile) {
        return;
    }

    bool found = false;
    RwStudioAsset asset;
    while (RwStudioFileNextAsset(pFile, &asset)) {
        if (asset.resourceLength == 0) {
            continue;
        }
        if (std::strcmp(asset.rwIdName, "wbID_STATICWORLDCOLLISION") != 0) {
            continue;
        }
        found = true;
        CHECK(asset.resourceLength > 0x3C);

        RwUInt8 *pBuf =
            static_cast<RwUInt8 *>(std::malloc(asset.resourceLength));
        CHECK(pBuf != 0);
        if (!pBuf) {
            break;
        }
        RwUInt32 got = RwStudioAssetReadResource(pFile, &asset, pBuf,
                                                 asset.resourceLength);
        CHECK(got == asset.resourceLength);

        StaticCollisionMesh mesh;
        CHECK(StaticCollisionData_Decode(pBuf, got, &mesh) == TRUE);
        CHECK(mesh.vertexCount == expectVc);
        CHECK(mesh.triangleCount == expectTc);
        CHECK(mesh.sectionBytes == expectSect);
        CHECK(NearEq(mesh.bboxMin[0], expectMinX, 0.5f));
        CHECK(NearEq(mesh.bboxMin[1], expectMinY, 0.5f));
        CHECK(NearEq(mesh.bboxMin[2], expectMinZ, 0.5f));
        CHECK(mesh.pVertices != 0 && mesh.pIndices != 0);
        CHECK(mesh.totalBytes <= got && got - mesh.totalBytes <= 4);
        CHECK(mesh.pIndices[0] == 0 && mesh.pIndices[1] == 1 &&
              mesh.pIndices[2] == 2);

        // shape üretimi (Bullet btBvhTriangleMeshShape) + ilk vertex okunabilir
        void *pShape = StaticCollisionData_CreateShape(&mesh);
        CHECK(pShape != 0);
        if (pShape) {
            CHECK(mesh.pVertices[0] > -2000.0f &&
                  mesh.pVertices[0] < 2000.0f);
            PhysicsShape_Destroy(pShape);
        }

        std::free(pBuf);
        break;
    }
    CHECK(found);
    RwStudioFileClose(pFile);
}

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

    // VIKING01: 38391 vert / 71303 tri, section 753474 (hexdump forensics)
    CheckWorld(argv[1], 38391, 71303, 753474, -475.77f, -9.08f, -333.67f);
    // IDUNA03: 6726 / 11592, section 113955
    CheckWorld(argv[2], 6726, 11592, 113955, -102.24f, -6.53f, -74.36f);
    // BEAVER01: 55902 / 102833, section 1064352
    CheckWorld(argv[3], 55902, 102833, 1064352, -478.53f, -8.53f, -596.25f);

    // bozuk girdi: truncation + flag bozulması FALSE vermeli
    {
        StaticCollisionMesh mesh;
        RwUInt8 junk[64];
        std::memset(junk, 0, sizeof(junk));
        CHECK(StaticCollisionData_Decode(junk, sizeof(junk), &mesh) == FALSE);
        junk[0] = 0x10; // vertexCount>0 ama gerisi sıfır
        CHECK(StaticCollisionData_Decode(junk, sizeof(junk), &mesh) == FALSE);
    }

    std::printf("%s: %d checks, %d failures\n", argv[0], s_checks,
                s_failures);
    RwEngineStop();
    RwEngineClose();
    RwEngineTerm();
    return s_failures == 0 ? 0 : 1;
}
