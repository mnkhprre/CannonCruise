// ============================================================================
// CannonCruise - .RWS Asset Yol Çözücü Test (rwsassets_test.cpp)
//
// Doğrulanan davranış (code/RwsAssets.cpp):
//   - CANNONCRUISE_STREAMS ortam değişkeni önceliklidir ve normalize edilir
//     (sondaki / veya \ eklenir/korunur)
//   - YOKSA sentinel (Streams/MAINMENU.RWS) ile aday önek taraması —
//     derin VS build yerleşimleri (code/build_<x>/tests → kök 3 seviye
//     yukarı) dahil edilir
//   - RwsAssets_Resolve yalnızca dosya MEVCUTSA yol döndürür
//   - bulunamayan dosya → 0 ve boş dize
//
// Önceki hata: main.cpp/SceneManager.cpp içinde 5 ayrı yerde elle yazılmış
// 3-önekli liste ("" / "../" / "../../") vardı; build çıktısı daha derin
// olduğu için asset hiç bulunamıyor (clump=0, assets=0).
// ============================================================================

#include "RwsAssets.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

static int g_checks = 0;
static int g_failures = 0;

static void Check(bool cond, const char *pExpr, int line) {
    ++g_checks;
    if (!cond) {
        ++g_failures;
        std::printf("FAIL line %d: %s\n", line, pExpr);
    }
}

#define CHECK(expr) Check((expr), #expr, __LINE__)

// Uygulama-bağımlı ortam değişkeni seti (Windows/POSIX)
static void SetEnv(const char *pName, const char *pValue) {
#if defined(_WIN32)
    _putenv_s(pName, pValue != nullptr ? pValue : "");
#else
    if (pValue != nullptr) {
        setenv(pName, pValue, 1);
    } else {
        unsetenv(pName);
    }
#endif
}

// ---------------------------------------------------------------------------
// 1) Ortam değişkeni ezme yolu — en yüksek öncelik, normalize edilmiş dize
// ---------------------------------------------------------------------------
static void TestEnvOverride() {
    RwsAssets_Reset();
    SetEnv("CANNONCRUISE_STREAMS", "Z:/bogus_streams"); // sondaki ayraç YOK
    const char *pDir = RwsAssets_Dir();
    CHECK(pDir != nullptr);
    // normalize: sondaki '/' eklenmeli
    CHECK(std::string(pDir) == "Z:/bogus_streams/");
    SetEnv("CANNONCRUISE_STREAMS", "Z:/bogus_streams/"); // sondaki '/' VAR
    RwsAssets_Reset();
    CHECK(std::string(RwsAssets_Dir()) == "Z:/bogus_streams/"); // çift ekleme yok

    // env geçersizken Resolve bulamaz → 0 + boş dize
    char out[512];
    CHECK(RwsAssets_Resolve("Streams/MAINMENU.RWS", out, sizeof(out)) == 0);
    CHECK(out[0] == '\0');

    SetEnv("CANNONCRUISE_STREAMS", nullptr);
    RwsAssets_Reset();
}

// ---------------------------------------------------------------------------
// 2) Sentinel taraması — Streams/ kökünü önek derinliğinden bulur
// ---------------------------------------------------------------------------
static void TestSentinelScan() {
    RwsAssets_Reset();
    const char *pDir = RwsAssets_Dir();
    CHECK(pDir != nullptr);
    // build/tests CWD'den kök ne kadar yukarı olursa olsun boş olmamalı
    // (5 aday önek: "" → "../../../../../")
    CHECK(pDir[0] != '\0');

    // kök bulunduysa sentinel Resolve edilebilmeli
    char out[512];
    const int found =
        RwsAssets_Resolve("Streams/MAINMENU.RWS", out, sizeof(out));
    if (found) {
        // yol kök dizin + dosya adıyla başlamalı
        CHECK(std::strncmp(out, pDir, std::strlen(pDir)) == 0);
        CHECK(std::strstr(out, "MAINMENU.RWS") != nullptr);
        // ve gerçekten açılabilir olmalı
        FILE *f = std::fopen(out, "rb");
        CHECK(f != nullptr);
        if (f != nullptr) {
            std::fclose(f);
        }
    } else {
        // CWD ağacın dışında olabilir — bu bir başarısızlık değil, ama
        // raporlayalım ki tarama derinliği yetersiz kalmasın.
        std::printf("NOTE: Streams/MAINMENU.RWS sentinel bulunamadı "
                    "(CWD kökün dışında olabilir), pDir='%s'\n", pDir);
    }
}

// ---------------------------------------------------------------------------
// 3) Önbellek — ikinci çağrı aynı sonucu verir, dosya yoksa 0 döner
// ---------------------------------------------------------------------------
static void TestCacheAndMissing() {
    RwsAssets_Reset();
    char a[512], b[512];
    const int ra = RwsAssets_Resolve("Streams/MAINMENU.RWS", a, sizeof(a));
    const int rb = RwsAssets_Resolve("Streams/MAINMENU.RWS", b, sizeof(b));
    CHECK(ra == rb);
    CHECK(std::string(a) == std::string(b));

    // var olmayan dosya → 0, tampon temizlenir
    char m[512] = {0x7f};
    CHECK(RwsAssets_Resolve("Streams/__NO_SUCH_FILE__.RWS", m, sizeof(m)) ==
          0);
    CHECK(m[0] == '\0');

    // null/boş girdi → güvenli
    CHECK(RwsAssets_Resolve(nullptr, m, sizeof(m)) == 0);
    CHECK(RwsAssets_Resolve("", m, sizeof(m)) == 0);
    CHECK(RwsAssets_Resolve("Streams/MAINMENU.RWS", nullptr, 512) == 0);

    // mevcut ama farklı aday (GLOBAL/VIKING01) tutarlı çözülmeli
    char g[512];
    const int rg = RwsAssets_Resolve("Streams/GLOBAL.RWS", g, sizeof(g));
    if (rg) {
        FILE *f = std::fopen(g, "rb");
        CHECK(f != nullptr);
        if (f != nullptr) {
            std::fclose(f);
        }
    }
}

// ---------------------------------------------------------------------------
// 4) Reset — env değişkeni tekrar değerlendirilir
// ---------------------------------------------------------------------------
static void TestResetReevaluatesEnv() {
    RwsAssets_Reset();
    SetEnv("CANNONCRUISE_STREAMS", "Y:/first/");
    CHECK(std::string(RwsAssets_Dir()) == "Y:/first/");

    SetEnv("CANNONCRUISE_STREAMS", "Y:/second/");
    // önbellekliyken ESKİ değer korunmalı
    CHECK(std::string(RwsAssets_Dir()) == "Y:/first/");
    // Reset sonrası yeniden değerlendirilir
    RwsAssets_Reset();
    CHECK(std::string(RwsAssets_Dir()) == "Y:/second/");

    SetEnv("CANNONCRUISE_STREAMS", nullptr);
    RwsAssets_Reset();
}

int main() {
    TestEnvOverride();
    TestSentinelScan();
    TestCacheAndMissing();
    TestResetReevaluatesEnv();

    std::printf("rwsassets_test: %d checks, %d failures\n", g_checks,
                g_failures);
    return (g_failures == 0) ? 0 : 1;
}
