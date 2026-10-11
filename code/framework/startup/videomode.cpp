// ============================================================================
// CannonCruise - Video Mode & Camera Setup (videomode.cpp)
// Original path: D:\Projects\CannonCruisePC\code\framework\startup\videomode.cpp
// Tersine mühendislik ile CC_ida.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_0041af10 — CameraInit  (assert "No best sub-system", videomode.cpp:228)
//   FUN_0041ac00 — CameraShutdown (startup shutdown zincirinin ilk adımı)
//
// Not: FUN_0041ac00'nin dosya bağlamı yalnızca startup.cpp'deki çağrıdan
//      biliniyor; asıl "videomode.cpp" assert'i (satır 0xE4 = 228)
//      FUN_0041af10 içindedir ve bu dosyanın CameraInit'i içerdiğini kanıtlar.
// ============================================================================

#include "rwcore.h"
#include "PhysicsWorld.h"
#include <cassert>

// ---------------------------------------------------------------------------
// Puanlama yardımcıları (FUN_0041af10 skor karşılaştırması — binary ağırlıkları)
// ---------------------------------------------------------------------------
namespace
{
    // Aday video modunu istenen width/height/depth değerlerine göre puanlar.
    // Ağırlıklar binary'deki gibi: derinlik eşleşmesi 16, boyut eşleşmesi 4,
    // istenen değeri aşan boyut 2 (büyük olasılıkla eşleşme bonusu).
    // Not: ASM'de düşük ağırlıklı (+1) ek bir eşleşme adımı daha vardır;
    // stack offset okuması belirsiz olduğundan genişlik eşleşmesine dahil edilmiştir.
    int ScoreVideoMode(const RwVideoMode& vm, int width, int height, int depth)
    {
        int score = 0;

        if (vm.width == width)
            score += 4;
        else if (vm.width > width)
            score += 2;

        if (vm.height == height)
            score += 4;
        else if (vm.height > height)
            score += 2;

        if (vm.depth == depth)
            score += 16;

        return score;
    }

    // İki aday eşit puanda: binary'de alan (genişlik*yükseklik) karşılaştırması
    // yapılır, daha küçük alan tercih edilir.
    RwBool IsBetterVideoMode(const RwVideoMode& candidate, int candidateScore,
                             const RwVideoMode& currentBest, int currentScore)
    {
        if (candidateScore != currentScore)
            return candidateScore > currentScore;

        const long long candidateArea =
            (long long)candidate.width * (long long)candidate.height;
        const long long bestArea =
            (long long)currentBest.width * (long long)currentBest.height;
        return candidateArea < bestArea;
    }
} // namespace

// ============================================================================
// CameraInit (FUN_0041af10)
// Binary Adresi: 0x0041af10
// XREF: sub_41A680 (startup init): 0041aae5 — startup.cpp "Adım 10"
//
// Tüm alt sistemleri ve video modlarını tarar; verilen width/height/depth
// değerlerine en iyi uyan modu puanlayarak seçer. Seçilen mod
// *videoMode üzerinden yazılır ve TRUE dönülür.
//
// En iyi mod hiç bulunamazsa binary'de:
//   "No best sub-system" assert'i (videomode.cpp, satır 0xE4 = 228)
//   → INT3 → (assert bayrağı yoksa) exit(0)
// ============================================================================
RwBool CameraInit(RwVideoMode* videoMode, int width, int height, int depth)
{
    int bestSubsystem = -1;
    int bestVideoMode = -1;
    RwVideoMode bestModeInfo;
    bestModeInfo.width = 0;
    bestModeInfo.height = 0;
    bestModeInfo.depth = 0;
    bestModeInfo.flags = 0;
    bestModeInfo.unknown2 = 0;
    bestModeInfo.unknown3 = 0;

    const int numSubSystems = RwEngineGetNumSubSystems();

    for (int subsystem = 0; subsystem < numSubSystems; ++subsystem)
    {
        if (!RwEngineSetSubSystem(subsystem))
            continue;

        const int numVideoModes = RwEngineGetNumVideoModes();

        for (int modeIndex = 0; modeIndex < numVideoModes; ++modeIndex)
        {
            RwVideoMode candidate;
            if (!RwEngineGetVideoModeInfo(&candidate, modeIndex))
                continue;

            if (bestVideoMode < 0)
            {
                // İlk aday doğrudan en iyi mod olarak alınır (binary:
                // henüz en iyi mod yokken kopyalama yolu, loc_41B05F).
                bestSubsystem = subsystem;
                bestVideoMode = modeIndex;
                bestModeInfo = candidate;
                continue;
            }

            const int candidateScore =
                ScoreVideoMode(candidate, width, height, depth);
            const int currentScore =
                ScoreVideoMode(bestModeInfo, width, height, depth);

            if (IsBetterVideoMode(candidate, candidateScore,
                                  bestModeInfo, currentScore))
            {
                bestSubsystem = subsystem;
                bestVideoMode = modeIndex;
                bestModeInfo = candidate;
            }
        }
    }

    // -----------------------------------------------------------------
    // En iyi mod bulunamadı mı?
    // ASM: "No best sub-system" → In file videomode.cpp, line 0E4h (228)
    //      → "Assert failed:" → INT3 → exit(0)
    // -----------------------------------------------------------------
    if (bestSubsystem < 0 || bestVideoMode < 0)
    {
        assert(!"No best sub-system");
        return FALSE;
    }

    // Seçilen alt sistem ve video modunu uygula
    if (!RwEngineSetSubSystem(bestSubsystem))
        return FALSE;

    if (!RwEngineSetVideoMode(bestVideoMode))
        return FALSE;

    // Seçilen mod bilgisini çağıran tarafın videoMode yapısına yaz
    // (ASM: aday struct'ın 6 dwords'ü kopyalanır — rep movsd, 24 byte).
    if (videoMode)
        *videoMode = bestModeInfo;

    return TRUE;
}

// ============================================================================
// CameraShutdown (FUN_0041ac00)
// Binary Adresi: 0x0041ac00
// XREF: sub_41AB90 (startup shutdown): 0041ab91 — startup.cpp "Adım 1"
//
// Binary davranışı:
//   1. DAT_00635af4 (ana oyun bağlamı singleton'ı — startup'ta "Havok
//      singleton" olarak adlandırılır) okunur.
//   2. NULL ise: "ms_pSingleton != NULL Failed" assert'i (Singleton.h:58)
//      tetiklenir; assert bayrağı yoksa exit(0).
//   3. Değilse: singleton'ın ilk sanal metodu 1 argümanla çağrılır
//      (vtable slot 0 — shutdown benzeri bağlam kapanışı).
//   4. DAT_00635af4 = NULL yazılır (obje bellekte serbest edilmez).
//
// Kod tabanı notu: singleton'ın somut türü (FUN_0049c870 ctor'ı) henüz
// eşlenmedi; Havok/fizik entegrasyonu (Faz 4.4b) tamamlandığında çağrısı
// bağlanacaktır. Şu an yalnızca assert sözleşmesi uygulanır.
// ============================================================================
void CameraShutdown(void)
{
    CPhysicsWorld* pContext = CPhysicsWorld::GetSingletonPtr();

    assert(pContext != 0 && "ms_pSingleton != NULL Failed");

    if (pContext)
    {
        // TODO(Faz 4.4b): binary'deki virtual slot0(pContext, 1) çağrısı —
        // FUN_0049c870 ctor'ının ilk sanal metodu (shutdown benzeri).
        // CPhysicsWorld'de karşılığı modellenene kadar çağrılmaz.
    }

    // Binary: DAT_00635af4 = NULL — singleton işaretçisi sıfırlanır,
    // obje belleği serbest edilmez (Singleton.h dtor'u bunu yapar).
}
