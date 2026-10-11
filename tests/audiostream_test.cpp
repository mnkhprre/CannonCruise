// ============================================================================
// CannonCruise - CAudioStreamEntity property apply testi
// Gerçek MAINMENU.RWS obj#2 (CAudioStreamEntity "Audio_MenuMusic") verisiyle
// sub_458BD0 switch'inin id→alan haritası doğrulanır.
// Çalıştırma: audiostream_test <MAINMENU.RWS>
// ============================================================================

#include "AudioStreamEntity.h"
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

    CAudioStreamEntity audio;
    CAudioStreamEntityInit(&audio);

    // ctor varsayılanları
    CHECK(audio.subOne == 1);
    CHECK(audio.loopCount == 0);
    CHECK(audio.float0 == 0.0f);

    // ---- MAINMENU obj#2 CAudioStreamEntity ----
    {
        RwStudioFile *pFile = RwStudioFileOpen(argv[1]);
        CHECK(pFile != 0);
        if (pFile) {
            RwStudioAsset asset;
            bool applied = false;
            int objectIndex = 0;
            while (RwStudioFileNextAsset(pFile, &asset)) {
                if (asset.chunkType != rwstudioID_OBJECTRECORD) {
                    continue;
                }
                const RwStudioObject *pObj = RwStudioFileGetObject(pFile);
                CHECK(pObj != 0);
                if (!pObj || pObj->behaviourCount == 0) {
                    ++objectIndex;
                    continue;
                }
                if (std::strcmp(pObj->behaviours[0].className,
                                "CAudioStreamEntity") != 0) {
                    ++objectIndex;
                    continue;
                }
                // obj#2 = "Audio_MenuMusic" (ad segmenti), 5 prop (id 1..5)
                CHECK(objectIndex == 2);
                const RwStudioBehaviour *pBeh = &pObj->behaviours[0];
                CHECK(pBeh->propCount == 5);
                for (RwUInt32 i = 0; i < pBeh->propCount; ++i) {
                    CHECK(pBeh->props[i].id == i + 1); // id 1..5 sıralı
                }

                for (RwUInt32 i = 0; i < pBeh->propCount; ++i) {
                    const RwStudioProperty &prop = pBeh->props[i];
                    CHECK(prop.size == 12);
                    CHECK(CAudioStreamEntityApplyAttribute(
                              &audio, prop.id, prop.value) == TRUE);
                }

                // ---- dosya değerleriyle doğrulama ----
                // props[3] = id4 = 5.0f (bilinen dump)
                CHECK(AsFloat(pBeh->props[3].value) == 5.0f);

                // id1/id2: 0xbfbfbf00 = boş inline string + 0xBF pad
                // (ilk bayt 0x00 → registered flag 0)
                CHECK(audio.slot28[1] == 0);
                CHECK(audio.slot30[1] == 0);
                // id3: 0xFFFFFFFF (-1)
                CHECK(audio.loopCount == 0xFFFFFFFF);
                CHECK(audio.loopCount == AsU32(pBeh->props[2].value));
                // id4: 5.0f
                CHECK(audio.float0 == 5.0f);
                CHECK(audio.float0 == AsFloat(pBeh->props[3].value));
                // id5: 0.0f
                CHECK(audio.float1 == 0.0f);
                CHECK(audio.float1 == AsFloat(pBeh->props[4].value));
                applied = true;
                break;
            }
            CHECK(applied);
            RwStudioFileClose(pFile);
        }
    }

    // ---- id0: stop + SAudioPlayStreamData* register ----
    audio.streamHandle = 0xDEADBEEF;
    audio.state = 1;
    const RwUInt8 emptyName[4] = {0x00, 0xBF, 0xBF, 0xBF};
    CHECK(CAudioStreamEntityApplyAttribute(&audio, 0, emptyName) == TRUE);
    CHECK(audio.streamHandle == 0); // stop
    CHECK(audio.state == 0);
    CHECK(audio.slot20[0] == 0 && audio.slot20[1] == 0); // boş → register yok

    const RwUInt8 streamName[] = {'M', 'y', 'S', 't', 'r', 'e', 'a', 'm', 0};
    audio.streamHandle = 0xDEADBEEF;
    CHECK(CAudioStreamEntityApplyAttribute(&audio, 0, streamName) == TRUE);
    CHECK(audio.streamHandle == 0);
    // kayıtlı: önekin ilk baytı != 0
    CHECK(((audio.slot20[0] & 0xFF) != 0));
    // ilk 8 bayt önek
    CHECK(audio.slot20[0] == AsU32(streamName));
    CHECK(std::memcmp(&audio.slot20[0], streamName, 8) == 0);

    // ---- bilinmeyen id → FALSE (binary default sessiz) ----
    CHECK(CAudioStreamEntityApplyAttribute(&audio, 6, emptyName) == FALSE);
    CHECK(CAudioStreamEntityApplyAttribute(&audio, 0xFF, emptyName) == FALSE);

    // ---- id1 dolu string ----
    CHECK(CAudioStreamEntityApplyAttribute(&audio, 1, streamName) == TRUE);
    CHECK((audio.slot28[0] & 0xFF) != 0); // kayıtlı
    CHECK(std::memcmp(&audio.slot28[0], streamName, 8) == 0);

    RwEngineStop();
    RwEngineClose();
    RwEngineTerm();

    std::printf("audiostream_test: %d checks, %d failures\n", s_checks,
                s_failures);
    return s_failures == 0 ? 0 : 1;
}
