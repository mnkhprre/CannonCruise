// ============================================================================
// CannonCruise — Ocean smoke test (Faz 4.6q wire-in)
// COcean::Initialize ızgara kurulumu + COceanTile dalgaları + Im3D render
// yolunun çökmeden çalıştığını doğrular (NULL platform: device no-op).
// ============================================================================

#include "Ocean.h"

#include <cstdio>

static int s_checks = 0;
static int s_failures = 0;

#define CHECK(cond)                                                            \
    do {                                                                       \
        ++s_checks;                                                            \
        if (!(cond)) {                                                         \
            ++s_failures;                                                      \
            std::printf("CHECK FAILED %s:%d: %s\n", __FILE__, __LINE__,        \
                        #cond);                                                \
        }                                                                      \
    } while (0)

int main() {
    if (RwEngineInit(0, 0, 0) != TRUE || RwEngineOpen() != TRUE ||
        RwEngineStart() != TRUE) {
        std::printf("ocean_test: engine boot failed\n");
        return 2;
    }

    COcean *pOcean = new COcean;
    CHECK(pOcean != nullptr);
    CHECK(COcean::GetInstance() == pOcean);

    // Grid: 2×2 tile, 100 birim, köşe (-100,-100) → [-100..100]²
    pOcean->Initialize(2, 2, 100.0f, -100.0f, -100.0f);
    pOcean->Update(0.5f);
    pOcean->Update(1.0f);
    pOcean->Render(); // Begin/End dışı — null: no-op; GL3: device yutarsa no-op

    // Bozuk girdi clamp'lenmeli (min 1×1, tileSize>0) — çökmemek yeterli
    pOcean->Initialize(0, 0, -1.0f, 0.0f, 0.0f);
    pOcean->Update(0.0f);
    pOcean->Render();

    // Yeniden-init eski ızgarayı temizler (leak/çakışma yok)
    pOcean->Initialize(4, 4, 50.0f, -100.0f, -100.0f);
    pOcean->Render();

    delete pOcean;
    CHECK(COcean::GetInstance() == nullptr);

    // NULL güvenli çağrılar
    pOcean = nullptr;

    RwEngineStop();
    RwEngineClose();
    RwEngineTerm();

    std::printf("ocean_test: %d checks, %d failures\n", s_checks, s_failures);
    return (s_failures == 0) ? 0 : 1;
}
