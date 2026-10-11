// ============================================================================
// CannonCruise - RenderBehaviourWorld property apply testi
// Gerçek MAINMENU.RWS obj#0 (CWbRenderBehavior, 13 property) verisiyle
// FUN_004c3e40 switch'inin id→offset haritası doğrulanır.
// Çalıştırma: renderworld_test <MAINMENU.RWS>
// ============================================================================

#include "RenderBehaviourWorld.h"
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

    // ---- ctor default'ları (FUN_004c4410) ----
    CRenderBehaviourWorld *pWorld = CRenderBehaviourWorld::Instance();
    CHECK(pWorld != 0);
    if (pWorld) {
        CHECK(pWorld->m_Render.m_param0 == 0.0f);
        CHECK(pWorld->m_Render.m_param1 == 125.0f);
        CHECK(pWorld->m_param2 == 1.0f);
        CHECK(pWorld->m_pad20 == 0);
        CHECK(pWorld->m_pad24 == 1);
        CHECK(pWorld->m_param8 == 1000.0f);
        CHECK(pWorld->m_param9 == 300.0f);
        CHECK(pWorld->m_pad58 == 0);
    }

    // ---- MAINMENU obj#0 CWbRenderBehavior property'lerini uygula ----
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
                                "CWbRenderBehavior") != 0) {
                    continue;
                }
                const RwStudioBehaviour *pBeh = &pObj->behaviours[0];
                CHECK(pBeh->propCount == 13);
                for (RwUInt32 i = 0; i < pBeh->propCount; ++i) {
                    const RwStudioProperty &prop = pBeh->props[i];
                    CHECK(prop.size == 12);
                    RwBool ok = pWorld->ApplyWbRenderAttribute(
                        prop.id, AsU32(prop.value));
                    CHECK(ok == TRUE);
                }
                applied = true;
                break;
            }
            CHECK(applied);
            RwStudioFileClose(pFile);
        }
    }

    // ---- obj#0 değerleri → singleton alanları (FUN_004c3e40 haritası) ----
    if (pWorld) {
        // prop0=0xFFFFFFFF (raw u32 — NaN float'a bit kopyası)
        CHECK(AsU32((const RwUInt8 *)&pWorld->m_Render.m_param0) ==
              0xFFFFFFFFu);
        CHECK(pWorld->m_Render.m_param1 == 1000.0f); // prop1
        CHECK(pWorld->m_param2 == 0.5f);             // prop2
        CHECK(pWorld->m_param3 == 0);                // prop3
        CHECK(pWorld->m_param4 == 0);                // prop4
        CHECK(pWorld->m_param5 == 0.0f);             // prop5
        // prop6 = 0xFF000000 → RGB(0,0,0)
        CHECK(pWorld->m_color1R == 0.0f);
        CHECK(pWorld->m_color1G == 0.0f);
        CHECK(pWorld->m_color1B == 0.0f);
        CHECK(pWorld->m_param7 == 0.1f); // prop7
        CHECK(pWorld->m_param8 == 1000.0f); // prop8 (default ile aynı)
        CHECK(pWorld->m_param9 == 300.0f);  // prop9 (default ile aynı)
        // propA = 0x00FFD0FF → RGB(255,208,255)
        CHECK(pWorld->m_color2R == 255.0f);
        CHECK(pWorld->m_color2G == 208.0f);
        CHECK(pWorld->m_color2B == 255.0f);
        CHECK(pWorld->m_paramB == 0.1f); // propB
        CHECK(pWorld->m_paramC == 30.0f); // propC
    }

    // ---- bilinmeyen id → UNHANDLED (binary: assert) ----
    if (pWorld) {
        CHECK(pWorld->ApplyWbRenderAttribute(0xD, 0) == FALSE);
        CHECK(pWorld->ApplyWbRenderAttribute(0xFF, 0x1234) == FALSE);
        // alanlar son apply'dan etkilenmedi
        CHECK(pWorld->m_paramC == 30.0f);
    }

    CRenderBehaviourWorld::Destroy();

    RwEngineStop();
    RwEngineClose();
    RwEngineTerm();

    std::printf("renderworld_test: %d checks, %d failures\n", s_checks,
                s_failures);
    return s_failures == 0 ? 0 : 1;
}
