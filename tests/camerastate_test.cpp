// ============================================================================
// CannonCruise - CCameraState property apply testi
// Gerçek MAINMENU.RWS obj#1 (CWbCameraBehavior) verisiyle sub_4C3450
// switch'inin id→offset/doğrudan haritası doğrulanır.
// Çalıştırma: camerastate_test <MAINMENU.RWS>
// ============================================================================

#include "CameraState.h"
#include "RwStudioStream.h"

#include <cmath>
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

static float AsFloat(const RwUInt8 *pValue) {
    float f = 0.0f;
    std::memcpy(&f, pValue, 4);
    return f;
}

static RwUInt32 AsU32(const RwUInt8 *pValue) {
    RwUInt32 v = 0;
    std::memcpy(&v, pValue, 4);
    return v;
}

static bool Near(float a, float b, float eps) {
    return std::fabs(a - b) <= eps;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        std::fprintf(stderr, "usage: %s <MAINMENU.RWS>\n", argv[0]);
        return 2;
    }

    // rw_librw shim — StreamFile::open engine init ister (vtable)
    if (RwEngineInit(0, 0, 0) != TRUE || RwEngineOpen() != TRUE ||
        RwEngineStart() != TRUE) {
        std::fprintf(stderr, "engine init failed\n");
        return 2;
    }

    CCameraState cam;
    std::memset(&cam, 0, sizeof(cam));

    // ---- MAINMENU obj#1 CWbCameraBehavior ----
    {
        RwStudioFile *pFile = RwStudioFileOpen(argv[1]);
        CHECK(pFile != 0);
        if (pFile) {
            RwStudioAsset asset;
            bool applied = false;
            while (RwStudioFileNextAsset(pFile, &asset)) {
                if (asset.chunkType != rwstudioID_OBJECTRECORD) {
                    continue;
                }
                const RwStudioObject *pObj = RwStudioFileGetObject(pFile);
                CHECK(pObj != 0);
                if (!pObj || pObj->behaviourCount == 0) {
                    continue;
                }
                if (std::strcmp(pObj->behaviours[0].className,
                                "CWbCameraBehavior") != 0) {
                    continue;
                }
                const RwStudioBehaviour *pBeh = &pObj->behaviours[0];
                for (RwUInt32 i = 0; i < pBeh->propCount; ++i) {
                    const RwStudioProperty &prop = pBeh->props[i];
                    if (prop.size == 20) {
                        CHECK(CCameraStateApplyVec3(&cam, prop.id,
                                                    prop.value) == TRUE);
                    } else {
                        CHECK(prop.size == 12);
                        CHECK(cam.ApplyAttribute(prop.id,
                                                 AsU32(prop.value)) == TRUE);
                    }
                }

                // ---- dosya değerleriyle doğrulama ----
                // bilinen dump: prop4 = 60.0f (FOV)
                CHECK(Near(AsFloat(pBeh->props[4].value), 60.0f, 0.001f));

                // id0 VEC3 → +0x04
                CHECK(Near(cam.m_pos[0], AsFloat(pBeh->props[0].value), 0.0f));
                CHECK(Near(cam.m_pos[1],
                          AsFloat(pBeh->props[0].value + 4), 0.0f));
                CHECK(Near(cam.m_pos[2],
                          AsFloat(pBeh->props[0].value + 8), 0.0f));
                // id1/id2 raw
                CHECK(AsU32((const RwUInt8 *)&cam.m_clip1) ==
                      AsU32(pBeh->props[1].value));
                CHECK(AsU32((const RwUInt8 *)&cam.m_clip2) ==
                      AsU32(pBeh->props[2].value));
                // id3 RAD (prop3 = 0 → 0.0)
                CHECK(cam.m_fovRad == 0.0f);
                CHECK(cam.m_fovRadDup == 0.0f);
                // id4 TAN: tan(60°)
                float expectTan =
                    std::tan(CameraDegToRad(AsFloat(pBeh->props[4].value)));
                CHECK(Near(cam.m_projX, expectTan, 1e-5f));
                CHECK(Near(cam.m_projXDup, expectTan, 1e-5f));
                CHECK(Near(cam.m_projX, 1.7320508f, 1e-4f)); // tan(60°)
                // id5 raw
                CHECK(AsU32((const RwUInt8 *)&cam.m_flagsA) ==
                      AsU32(pBeh->props[5].value));
                // id6 VEC3 → +0x24
                CHECK(Near(cam.m_dir[0], AsFloat(pBeh->props[6].value), 0.0f));
                // id7 raw
                CHECK(AsU32((const RwUInt8 *)&cam.m_param7) ==
                      AsU32(pBeh->props[7].value));
                applied = true;
                break;
            }
            CHECK(applied);
            RwStudioFileClose(pFile);
        }
    }

    // ---- id42/id48 guard davranışları ----
    cam.m_counter = 5.0f;
    cam.m_guard = 1;
    CHECK(cam.ApplyAttribute(0x2A, 0) == TRUE); // guard=1 → değişmez
    CHECK(cam.m_counter == 5.0f);
    CHECK(cam.ApplyAttribute(0x30, 0) == TRUE);
    CHECK(cam.m_one == 0.0f);
    cam.m_guard = 0;
    CHECK(cam.ApplyAttribute(0x2A, 0) == TRUE);
    CHECK(cam.m_counter == 6.0f);
    CHECK(cam.ApplyAttribute(0x30, 0) == TRUE);
    CHECK(cam.m_one == 1.0f);

    // ---- bilinmeyen id → FALSE (binary default sessiz) ----
    CHECK(cam.ApplyAttribute(0x33, 0) == FALSE);
    CHECK(cam.ApplyAttribute(0xFF, 0) == FALSE);
    // VEC3 yanlış id → FALSE
    RwUInt8 tmp[12] = {0};
    CHECK(CCameraStateApplyVec3(&cam, 0x33, tmp) == FALSE);

    RwEngineStop();
    RwEngineClose();
    RwEngineTerm();

    std::printf("camerastate_test: %d checks, %d failures\n", s_checks,
                s_failures);
    return s_failures == 0 ? 0 : 1;
}
