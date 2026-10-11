// ============================================================================
// CannonCruise - Render Platformu Smoke Testi (renderplatform_test.cpp)
// Faz 4.6n: engine boot → kurulum (idempotent) → 3 kare (clear/Begin/
// Im2D quad/End) → terminate → yeniden kurulum → teardown. NULL platform
// headless (device no-op — kare sequence API sözleşmesini doğrular).
// ============================================================================

#include "RenderPlatform.h"
#include "rwcore.h"

#include <cstdio>

static int s_checks = 0;
static int s_failures = 0;

#define CHECK(cond)                                                        \
    do {                                                                   \
        ++s_checks;                                                        \
        if (!(cond)) {                                                     \
            ++s_failures;                                                  \
            std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);    \
        }                                                                  \
    } while (0)

int main() {
    if (RwEngineInit(0, 0, 0) != TRUE || RwEngineOpen() != TRUE ||
        RwEngineStart() != TRUE) {
        std::fprintf(stderr, "engine init failed\n");
        return 2;
    }

    // Kurulum
    CHECK(RenderPlatform_Initialize(640, 480) == TRUE);
    CHECK(RenderPlatform_IsInitialized() == TRUE);
    CHECK(RenderPlatform_GetWorld() != nullptr);
    CHECK(RenderPlatform_GetCamera() != nullptr);

    // Idempotent
    CHECK(RenderPlatform_Initialize(640, 480) == TRUE);

    // 3 kare: Begin → TestQuad → End
    for (int frame = 0; frame < 3; ++frame) {
        CHECK(RenderPlatform_BeginFrame(nullptr) == TRUE);
        CHECK(RenderPlatform_RenderTestQuad(0.0f, 0.0f, 0.25f, 0.25f,
                                            0xFF40A0FF) == TRUE);
        RenderPlatform_EndFrame();
    }

    // Clear rengi ile BeginFrame
    static RwRGBA kClear = {16, 32, 48, 255};
    CHECK(RenderPlatform_BeginFrame(&kClear) == TRUE);
    RenderPlatform_EndFrame();

    // Terminate → temiz durum
    RenderPlatform_Terminate();
    CHECK(RenderPlatform_IsInitialized() == FALSE);
    CHECK(RenderPlatform_GetWorld() == nullptr);
    CHECK(RenderPlatform_GetCamera() == nullptr);

    // Kurulu değilken kare/quad → FALSE
    CHECK(RenderPlatform_BeginFrame(nullptr) == FALSE);
    CHECK(RenderPlatform_RenderTestQuad(0.0f, 0.0f, 0.1f, 0.1f,
                                        0xFFFFFFFF) == FALSE);

    // Tekrar terminate (idempotent) + yeniden kurulum
    RenderPlatform_Terminate();
    CHECK(RenderPlatform_Initialize(800, 600) == TRUE);
    CHECK(RenderPlatform_BeginFrame(nullptr) == TRUE);
    CHECK(RenderPlatform_RenderTestQuad(0.1f, -0.1f, 0.2f, 0.2f,
                                        0xFFFF8020) == TRUE);
    RenderPlatform_EndFrame();
    RenderPlatform_Terminate();
    CHECK(RenderPlatform_IsInitialized() == FALSE);

    RwEngineStop();
    RwEngineClose();
    RwEngineTerm();

    std::printf("renderplatform_test: %d checks, %d failures\n", s_checks,
                s_failures);
    return (s_failures == 0) ? 0 : 1;
}
