// ============================================================================
// CannonCruise - Faz 4.6p: dünya spawn zinciri testi
// VIKING01 0x0704 kayıtları → CClassFactory create + apply dispatch;
// bilinen sınıflar üretilir, bilinmeyenler binary'de olduğu gibi atlanır.
// Çalıştırma: worldspawn_test <VIKING01.RWS> <MAINMENU.RWS>
// ============================================================================

#include "WorldSpawn.h"

#include "BehaviourLoader.h"
#include "RwStudioStream.h"
#include "framework/core/attributehandler/cclassfactory.h"

#include <cstdio>
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

int main(int argc, char **argv) {
    if (argc < 3) {
        std::fprintf(stderr, "usage: %s <VIKING01.RWS> <MAINMENU.RWS>\n",
                     argv[0]);
        return 2;
    }

    if (RwEngineInit(0, 0, 0) != TRUE || RwEngineOpen() != TRUE ||
        RwEngineStart() != TRUE) {
        std::fprintf(stderr, "engine init failed\n");
        return 2;
    }

    CHECK(GameBehaviours_Register() == TRUE);

    // RWS_REGISTER_CLASS statikleri: oyun sınıfları factory'de hazır
    CHECK(RWS::CClassFactory::ClassIsRegistered("CTowerCatapult") == TRUE);
    CHECK(RWS::CClassFactory::ClassIsRegistered("CCollisionTrigger") == TRUE);

    // VIKING01: 497 kaydın tamamı walk edilir; bilinen sınıflar üretilir
    {
        WorldSpawnStats stats;
        const RwInt32 applied = WorldSpawn_FromFile(argv[1], &stats);
        std::printf("VIKING01 spawn: records=%u created=%u skipped=%u "
                    "applied=%d\n",
                    stats.objectRecords, stats.instancesCreated,
                    stats.instancesSkipped, (int)applied);
        CHECK(stats.objectRecords == 497); // rwstudio_test envanteri
        // bilinen: render(1) + camera(obj264: 2×) + audio(1) + physics(0)
        CHECK(stats.instancesCreated >= 4);
        CHECK(stats.instancesSkipped > 0); // CTowerCatapult vb. atlandı
        CHECK(applied >= 4);
    }

    // MAINMENU: render/camera/audio apply üçlüsü (behaviourloader_test
    // fixture'larıyla aynı nesneler)
    {
        WorldSpawnStats stats;
        const RwInt32 applied = WorldSpawn_FromFile(argv[2], &stats);
        std::printf("MAINMENU spawn: records=%u created=%u skipped=%u "
                    "applied=%d\n",
                    stats.objectRecords, stats.instancesCreated,
                    stats.instancesSkipped, (int)applied);
        CHECK(stats.objectRecords == 4);
        CHECK(stats.instancesCreated >= 3); // render + camera + audio
        CHECK(applied >= 3);
    }

    CHECK(RwEngineStop() == TRUE);
    CHECK(RwEngineClose() == TRUE);
    CHECK(RwEngineTerm() == TRUE);

    std::printf("worldspawn_test: %d checks, %d failures\n", s_checks,
                s_failures);
    return s_failures == 0 ? 0 : 1;
}
