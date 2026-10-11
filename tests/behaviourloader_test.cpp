// ============================================================================
// CannonCruise - BehaviourLoader entegrasyon testi (Faz 4.6k)
// 0x0704 create-by-name dispatch: sınıf adı → CClassFactory → create →
// apply virtual (render/camera/audio portları) MAINMENU nesnelerinde
// uçtan uca doğrulanır.
// Çalıştırma: behaviourloader_test <MAINMENU.RWS>
// ============================================================================

#include "BehaviourLoader.h"

#include "AudioStreamEntity.h"
#include "CameraState.h"
#include "RenderBehaviourWorld.h"
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

// ---- fabrika + apply adaptörleri (binary: factory + vtable slot1) ----

static void *CreateWbRender() {
    CRenderBehaviourSub *pSub = new CRenderBehaviourSub;
    std::memset(pSub, 0, sizeof(*pSub));
    return pSub; // apply hedefi singleton (binary: apply DAT_00635648)
}

static RwBool ApplyWbRender(void *, RwUInt32 id, const RwUInt8 *pValue) {
    RwUInt32 bits = 0;
    std::memcpy(&bits, pValue, 4);
    return CRenderBehaviourWorld::Instance()->ApplyWbRenderAttribute(id, bits);
}

static void *CreateWbCamera() {
    CCameraState *pCam = new CCameraState;
    std::memset(pCam, 0, sizeof(*pCam));
    return pCam;
}

static RwBool ApplyWbCamera(void *pObject, RwUInt32 id,
                            const RwUInt8 *pValue) {
    CCameraState *pCam = static_cast<CCameraState *>(pObject);
    if (CCameraStateApplyVec3(pCam, id, pValue) == TRUE) {
        return TRUE; // VEC3 kaydı (12B kopya)
    }
    RwUInt32 bits = 0;
    std::memcpy(&bits, pValue, 4);
    return pCam->ApplyAttribute(id, bits);
}

static void *CreateAudio() {
    CAudioStreamEntity *pAudio = new CAudioStreamEntity;
    CAudioStreamEntityInit(pAudio);
    return pAudio;
}

static RwBool ApplyAudio(void *pObject, RwUInt32 id, const RwUInt8 *pValue) {
    return CAudioStreamEntityApplyAttribute(
        static_cast<CAudioStreamEntity *>(pObject), id, pValue);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        std::fprintf(stderr, "usage: %s <MAINMENU.RWS>\n", argv[0]);
        return 2;
    }

    if (RwEngineInit(0, 0, 0) != TRUE || RwEngineOpen() != TRUE ||
        RwEngineStart() != TRUE) {
        std::fprintf(stderr, "engine init failed\n");
        return 2;
    }

    CRenderBehaviourWorld::Instance()->Init();

    // binary Register boyutları: 0x18 / 0x34 / 0x54
    static const BehaviourClassEntry kRender = {
        "CWbRenderBehavior", 0x18, CreateWbRender, ApplyWbRender};
    static const BehaviourClassEntry kCamera = {
        "CWbCameraBehavior", 0x34, CreateWbCamera, ApplyWbCamera};
    static const BehaviourClassEntry kAudio = {
        "CAudioStreamEntity", 0x54, CreateAudio, ApplyAudio};
    CHECK(BehaviourLoader::Register(&kRender) == TRUE);
    CHECK(BehaviourLoader::Register(&kCamera) == TRUE);
    CHECK(BehaviourLoader::Register(&kAudio) == TRUE);

    {
        RwStudioFile *pFile = RwStudioFileOpen(argv[1]);
        CHECK(pFile != 0);
        if (pFile) {
            RwStudioAsset asset;
            int objectIndex = 0;
            int totalApplied = 0;
            void *instances[rwstudioMAX_BEHAVIOURS];

            while (RwStudioFileNextAsset(pFile, &asset)) {
                if (asset.chunkType != rwstudioID_OBJECTRECORD) {
                    continue;
                }
                const RwStudioObject *pObj = RwStudioFileGetObject(pFile);
                CHECK(pObj != 0);
                if (!pObj) {
                    ++objectIndex;
                    continue;
                }
                RwInt32 applied = BehaviourLoader::ProcessObject(
                    pObj, instances, rwstudioMAX_BEHAVIOURS);
                CHECK(applied >= 0);
                totalApplied += applied;

                if (objectIndex == 0) {
                    // CWbRenderBehavior — 13 prop apply → singleton
                    CHECK(applied == 1);
                    CRenderBehaviourWorld *pWorld =
                        CRenderBehaviourWorld::Instance();
                    // prop id0 → +0x14 bit kopyası (dosya değeriyle)
                    RwUInt32 param0Bits = 0;
                    std::memcpy(&param0Bits, &pWorld->m_Render.m_param0, 4);
                    CHECK(param0Bits ==
                          *reinterpret_cast<const RwUInt32 *>(
                              pObj->behaviours[0].props[0].value));
                    // prop idC = 30.0f → +0x40
                    CHECK(pWorld->m_paramC == 30.0f);
                    // prop idA = 0x00FFD0FF → RGB(255,208,255) float
                    CHECK(pWorld->m_color2R == 255.0f);
                    CHECK(pWorld->m_color2G == 208.0f);
                    CHECK(pWorld->m_color2B == 255.0f);
                    delete static_cast<CRenderBehaviourSub *>(instances[0]);
                } else if (objectIndex == 1) {
                    // CWbCameraBehavior — 9 prop apply → CCameraState
                    CHECK(applied == 1);
                    const CCameraState *pCam =
                        static_cast<const CCameraState *>(instances[0]);
                    // prop0 VEC3 → m_pos (dosya değeriyle)
                    const RwUInt8 *pVec = pObj->behaviours[0].props[0].value;
                    CHECK(pCam->m_pos[0] == AsFloat(pVec));
                    CHECK(pCam->m_pos[1] == AsFloat(pVec + 4));
                    CHECK(pCam->m_pos[2] == AsFloat(pVec + 8));
                    // tan(60°) ≈ 1.7320508 (prop4 = 60.0f)
                    CHECK(std::fabs(pCam->m_projX - 1.7320508f) < 1e-4f);
                    delete pCam;
                } else if (objectIndex == 2) {
                    // CAudioStreamEntity — 5 prop apply → entity
                    CHECK(applied == 1);
                    const CAudioStreamEntity *pAudio =
                        static_cast<const CAudioStreamEntity *>(instances[0]);
                    CHECK(pAudio->loopCount == 0xFFFFFFFF);
                    CHECK(pAudio->float0 == 5.0f);
                    CHECK(pAudio->float1 == 0.0f);
                    delete pAudio;
                } else {
                    // bilinmeyen sınıf → skip (factory NULL)
                    CHECK(applied == 0);
                }
                ++objectIndex;
            }
            // 4 OBJECTRECORD: 3 bilinen + 1 bilinmeyen
            CHECK(objectIndex == 4);
            CHECK(totalApplied == 3);
            RwStudioFileClose(pFile);
        }
    }

    // bilinmeyen sınıf adı → NULL
    CHECK(BehaviourLoader::Create("CNoSuchClass") == 0);

    BehaviourLoader::UnregisterAll();
    CRenderBehaviourWorld::Destroy();
    RwEngineStop();
    RwEngineClose();
    RwEngineTerm();

    std::printf("behaviourloader_test: %d checks, %d failures\n", s_checks,
                s_failures);
    return s_failures == 0 ? 0 : 1;
}
