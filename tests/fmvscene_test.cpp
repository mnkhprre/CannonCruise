// ============================================================================
// CannonCruise - FMV Sahne Tanımlayıcısı Test (fmvscene_test.cpp)
//
// Doğrulanan davranış (code/FmvScene.cpp):
//   - FMV tanımlayıcısı (CFMVBehavior) ayrıştırması: GUID + FMV adı + HEDEF
//     SAHNE — 8 dosyalık tam geçiş grafiği dosya verisinden çıkarılır:
//       JINGLE  → Jingle  → MainMenu   (binary sub_4A2010 AÇILIŞI)
//       INTRO1  → Intro   → Nordic01   (intro → nordic BAĞLANTISI)
//       INTRO2  → Intro2  → Nordic02
//       INTRO3  → Intro3  → MAPMENU
//       CREDITS → Credits → MainMenu
//       OUTRO1  → Outro   → Iduna02
//       OUTRO2  → Outro2  → Iduna03
//       OUTRO3  → Outro3  → Credits
//   - seviye/dünya dosyaları (>64 KB) tanımlayıcı SAYILMAZ → 0 (zincir sonu)
//   - video yolu binary-exact: ".\Video\Win32\<ad>.bik" (sub_45F950 case0)
//   - .bik depoda YOK → FmvScene_VideoExists = 0 (FMV skip)
//   - hedef → sıra dosyası; hedef büyük/küçük harf FARKI için UPPERCASE
//     ikinci denemesi ("Nordic01" → Streams/NORDIC01.RWS)
//
// Öncelik: önceki kod hedefi (target) yalnızca yazdırıp geçiyordu — nordic
// bağlantısı yoktu; ayrıca giriş INTRO1→2→3 varsayımı yanlıştı.
// ============================================================================

#include "FmvScene.h"
#include "RwsAssets.h"

#include <cstdio>
#include <cstring>

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

// Büyük/küçük harf duyarsız dize karşılaştırma — Windows'ta dosya sistemi
// büyük/küçük harfe duyarsız olduğundan FmvScene_NextScenePath birebir denemede
// "Streams/Nordic01.RWS" gibi karışık-harf yol döndürür; duyarsız FS'te ise
// UPPERCASE denemesiyle "Streams/NORDIC01.RWS". İkisi de doğru davranış →
// test hangi biçimin döndüğünü değil, AYNI dosyaya çözüldüğünü doğrular.
static bool EqualsNoCase(const char *pA, const char *pB) {
    if (pA == nullptr || pB == nullptr) {
        return false;
    }
    for (; *pA != 0 && *pB != 0; ++pA, ++pB) {
        char ca = *pA;
        char cb = *pB;
        if (ca >= 'a' && ca <= 'z') {
            ca = (char)(ca - 'a' + 'A');
        }
        if (cb >= 'a' && cb <= 'z') {
            cb = (char)(cb - 'a' + 'A');
        }
        if (ca != cb) {
            return false;
        }
    }
    return *pA == 0 && *pB == 0;
}

// ---------------------------------------------------------------------------
// 1) Geçiş grafiği — 8 FMV tanımlayıcısının hepsi (FMV adı + hedef)
// ---------------------------------------------------------------------------
static void TestTransitionGraph() {
    struct Case {
        const char *pFile;
        const char *pFmv;
        const char *pTarget;
    };
    static const Case kCases[] = {
        {"Streams/JINGLE.RWS", "Jingle", "MainMenu"},
        {"Streams/INTRO1.RWS", "Intro", "Nordic01"},
        {"Streams/INTRO2.RWS", "Intro2", "Nordic02"},
        {"Streams/INTRO3.RWS", "Intro3", "MAPMENU"},
        {"Streams/CREDITS.RWS", "Credits", "MainMenu"},
        {"Streams/OUTRO1.RWS", "Outro", "Iduna02"},
        {"Streams/OUTRO2.RWS", "Outro2", "Iduna03"},
        {"Streams/OUTRO3.RWS", "Outro3", "Credits"},
    };
    for (size_t i = 0; i < sizeof(kCases) / sizeof(kCases[0]); ++i) {
        FmvSceneDescriptor d;
        std::memset(&d, 0xAA, sizeof(d));
        const int ok = FmvScene_Load(kCases[i].pFile, &d);
        if (ok != 1) {
            std::printf("FAIL: %s tanımlayıcı değil\n", kCases[i].pFile);
        }
        CHECK(ok == 1);
        CHECK(std::strcmp(d.behaviour, kCases[i].pFmv) == 0);
        CHECK(std::strcmp(d.target, kCases[i].pTarget) == 0);
        // GUID: harici {GUID} kimliği mevcut (içinde '{' ve '}' var)
        CHECK(d.guid[0] == '{');
        CHECK(std::strchr(d.guid, '}') != nullptr);
        if (ok == 1) {
            std::printf("  %s: fmv='%s' target='%s' guid=%s\n",
                        kCases[i].pFile, d.behaviour, d.target, d.guid);
        }
    }
}

// ---------------------------------------------------------------------------
// 2) Seviye/dünya dosyaları tanımlayıcı DEĞİL → 0 (zincir sonu garantisi)
//    eksik dosya → 0
// ---------------------------------------------------------------------------
static void TestNonDescriptorAndMissing() {
    FmvSceneDescriptor d;
    // 15.9 MB seviye — 64 KB sınırının çok üzerinde
    CHECK(FmvScene_Load("Streams/VIKING01.RWS", &d) == 0);
    // 1.8 MB menü — seviye/dünya sayılır (JINGLE hedefi burada biter)
    CHECK(FmvScene_Load("Streams/MAINMENU.RWS", &d) == 0);
    // bulunmayan dosya
    CHECK(FmvScene_Load("Streams/NO_SUCH_SCENE.RWS", &d) == 0);
    // boş/geçersiz girdi
    CHECK(FmvScene_Load(nullptr, &d) == 0);
    CHECK(FmvScene_Load("Streams/JINGLE.RWS", nullptr) == 0);
}

// ---------------------------------------------------------------------------
// 3) Video yolu — sub_45F950 case0: <base>"Video\Win32\"<ad>".bik", base ".\"
// ---------------------------------------------------------------------------
static void TestVideoPath() {
    char buf[512];
    CHECK(FmvScene_VideoPath("Jingle", buf, sizeof(buf)) == 1);
    CHECK(std::strcmp(buf, ".\\Video\\Win32\\Jingle.bik") == 0);
    CHECK(FmvScene_VideoPath("Intro2", buf, sizeof(buf)) == 1);
    CHECK(std::strcmp(buf, ".\\Video\\Win32\\Intro2.bik") == 0);
    // geçersiz girdi
    CHECK(FmvScene_VideoPath(nullptr, buf, sizeof(buf)) == 0);
    CHECK(FmvScene_VideoPath("", buf, sizeof(buf)) == 0);
    CHECK(FmvScene_VideoPath("Jingle", nullptr, 0) == 0);

    // Depoda hiç .bik yok → her zaman 0 (FMV skip); .bik eklenince çalışır.
    CHECK(FmvScene_VideoExists("Jingle") == 0);
    CHECK(FmvScene_VideoExists("Intro") == 0);
    CHECK(FmvScene_VideoExists(nullptr) == 0);
}

// ---------------------------------------------------------------------------
// 4) Hedef → sıradaki tanımlayıcı yolu. Kritik: dosyalar BÜYÜK HARF
//    (NORDIC01.RWS, MAINMENU.RWS) ama hedef "Nordic01"/"MainMenu" →
//    UPPERCASE ikinci denemesi şart (yoksa nordic bağlantısı KAYIP olur).
// ---------------------------------------------------------------------------
static void TestNextScenePath() {
    char rel[512];

    CHECK(FmvScene_NextScenePath("Nordic01", rel, sizeof(rel)) == 1);
    CHECK(EqualsNoCase(rel, "Streams/NORDIC01.RWS"));

    CHECK(FmvScene_NextScenePath("Nordic02", rel, sizeof(rel)) == 1);
    CHECK(EqualsNoCase(rel, "Streams/NORDIC02.RWS"));

    CHECK(FmvScene_NextScenePath("MainMenu", rel, sizeof(rel)) == 1);
    CHECK(EqualsNoCase(rel, "Streams/MAINMENU.RWS"));

    CHECK(FmvScene_NextScenePath("MAPMENU", rel, sizeof(rel)) == 1);
    CHECK(EqualsNoCase(rel, "Streams/MAPMENU.RWS"));

    CHECK(FmvScene_NextScenePath("Iduna02", rel, sizeof(rel)) == 1);
    CHECK(EqualsNoCase(rel, "Streams/IDUNA02.RWS"));

    // birebir eşleşen büyük harf hedef (CREDITS)
    CHECK(FmvScene_NextScenePath("CREDITS", rel, sizeof(rel)) == 1);
    CHECK(EqualsNoCase(rel, "Streams/CREDITS.RWS"));

    // var olmayan hedef → 0
    CHECK(FmvScene_NextScenePath("NoSuchLevel", rel, sizeof(rel)) == 0);
    // geçersiz girdi
    CHECK(FmvScene_NextScenePath(nullptr, rel, sizeof(rel)) == 0);
    CHECK(FmvScene_NextScenePath("", rel, sizeof(rel)) == 0);
}

// ---------------------------------------------------------------------------
// 5) Zincir kurulumu: JINGLE → (MAINMENU) → tanımlayıcı değil → SON.
//    INTRO1 → (NORDIC01) → tanımlayıcı değil → SON + hedef kayıtlı.
// ---------------------------------------------------------------------------
static void TestChainTermination() {
    char rel[512] = {0};
    char terminal[64] = {0};

    // a) binary açılışı: JINGLE
    FmvSceneDescriptor d;
    CHECK(FmvScene_Load("Streams/JINGLE.RWS", &d) == 1);
    std::snprintf(terminal, sizeof(terminal), "%s", d.target);
    CHECK(FmvScene_NextScenePath(d.target, rel, sizeof(rel)) == 1);
    CHECK(EqualsNoCase(rel, "Streams/MAINMENU.RWS"));
    // zincir sonraki adımda tanımlayıcı değil (1.8 MB) → SON
    CHECK(FmvScene_Load(rel, &d) == 0);
    CHECK(std::strcmp(terminal, "MainMenu") == 0);

    // b) INTRO1 → nordic bağlantısı
    CHECK(FmvScene_Load("Streams/INTRO1.RWS", &d) == 1);
    std::snprintf(terminal, sizeof(terminal), "%s", d.target);
    CHECK(FmvScene_NextScenePath(d.target, rel, sizeof(rel)) == 1);
    CHECK(EqualsNoCase(rel, "Streams/NORDIC01.RWS"));
    // NORDIC01 tanımlayıcı değil (14 MB seviye) → zincir sonu, hedef korunur
    CHECK(FmvScene_Load(rel, &d) == 0);
    CHECK(std::strcmp(terminal, "Nordic01") == 0);

    // c) OUTRO zinciri: OUTRO3 → Credits (TANIMLAYICI → zincir devam eder)
    CHECK(FmvScene_Load("Streams/OUTRO3.RWS", &d) == 1);
    CHECK(FmvScene_NextScenePath(d.target, rel, sizeof(rel)) == 1);
    CHECK(FmvScene_Load(rel, &d) == 1);
    CHECK(std::strcmp(d.behaviour, "Credits") == 0);
    // Credits → MainMenu → tanımlayıcı değil → SON
    CHECK(FmvScene_NextScenePath(d.target, rel, sizeof(rel)) == 1);
    CHECK(FmvScene_Load(rel, &d) == 0);
}

int main() {
    // Sentinel taraması test CWD'sinden kök bulur (rwsassets_test ile aynı).
    RwsAssets_Reset();

    TestTransitionGraph();
    TestNonDescriptorAndMissing();
    TestVideoPath();
    TestNextScenePath();
    TestChainTermination();

    std::printf("fmvscene_test: %d checks, %d failures\n", g_checks,
                g_failures);
    if (g_failures != 0) {
        return 1;
    }
    std::printf("fmvscene_test: OK\n");
    return 0;
}
