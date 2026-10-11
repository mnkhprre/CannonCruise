// ============================================================================
// CannonCruise - Sistem servisleri bootstrap (systemservices.cpp)
// Original path: D:\Projects\CannonCruisePC\code\framework\startup\systemservices.cpp
// Tersine mühendislik ile CC_ida.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_005ca6c0 — FileSystemInit      (startup init zinciri, "Adım 3a")
//   FUN_00428de0 — TextureCacheInit    (startup init zinciri, "Adım 3b")
//   FUN_004cac40 — SubSystemShutdown   (startup shutdown zinciri, +1A)
//   FUN_0040fd60 — ResourceShutdown    (startup shutdown zinciri, +2E)
//   FUN_00405670 — GeneralShutdown     (startup shutdown zinciri, +33)
//
// Not: Dosya bağlamı yalnızca startup.cpp'deki çağrı XREF'lerinden biliniyor;
//      ResourceShutdown haricinde (assert path cattributehandler.cpp) hiçbirinde
//      dosya ispatı string'i yok.
// ============================================================================

#include "rwcore.h"
#include "attributehandler/cattributehandler.h"
#include "eventhandler/ceventhandler.h"
#include <cassert>
#include <cstdlib>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>       // timeGetDevCaps / timeBeginPeriod / timeGetTime
#ifdef _MSC_VER
#pragma comment(lib, "winmm.lib")
#endif
#endif

namespace
{
    // dword_658944 — RW alt sistem cihaz listesi başı (binary shutdown ilk
    // iş olarak bunu test eder; null ise hemen çıkar). Cihaz listesi
    // engine nesnesi (dword_65D7A4) vtable (+0x138h) üzerinden serbest
    // bırakılır; RW motor altyapısı restor edilene kadar null kalır.
    void* s_pSubSystemList = 0;
}

// ============================================================================
// FileSystemInit (FUN_005CA6C0)
// Binary Adresi: 0x005ca6c0
// XREF: sub_41A680 (startup init): 0041a7f8 — startup.cpp "Adım 3a"
//       (shutdown'da da tekrar çağrılır: 0041abb9)
//
// Çözüm: IDA'da bu adres 1 baytlık `nullsub_11` (`retn`) olarak etiketli
// (sub_5CA6A0 ile sub_5CA6D0 arasındaki align boşluğu). Ghidra'daki
// FUN_005ca6c0 CALL hedefi de aynı tek bayt. Yani binary'de dosya sistemi
// başlatma ÇAĞRISI TAM BİR NO-OP'tur — gerçek kurulum statik başlatıcılar
// ya da başka bir aşamada yapılır. Gerçeğe sadık olarak boş gövde yazılır.
// ============================================================================
void FileSystemInit(void)
{
    // Binary: `retn` (nullsub_11) — bilinçli no-op.
}

// ============================================================================
// TextureCacheInit (FUN_00428DE0)
// Binary Adresi: 0x00428de0
// XREF: sub_41A680 (startup init): 0041a7fd — startup.cpp "Adım 3b"
//
// Binary gövdesi (WINMM):
//   1. timeGetDevCaps(&tc, sizeof(TIMECAPS))
//   2. timeBeginPeriod(tc.wPeriodMin)     — zaman çözünürlüğünü yükselt
//   3. ESI = timeGetTime()                — başlangıç damgası (dönüş değeri)
//   4. timeEndPeriod(tc.wPeriodMin)
// Çağıran (startup) dönüş değerini yok sayar; yine de binary gibi zaman
// damgası üretilir ve sıfır değilse TRUE döner.
// Windows dışı ortamda WINMM yok → saat okuması yapılmadan TRUE.
// Not (Faz 4.4b): Windows derlemesinde winmm.lib bağlantısı gerekir
// (bu dosyadaki pragma MSVC'de halleder).
// ============================================================================
RwBool TextureCacheInit(void)
{
#ifdef _WIN32
    TIMECAPS tc;
    if (timeGetDevCaps(&tc, sizeof(tc)) != TIMERR_NOERROR)
        return TRUE;                    // binary de hatada devam ederdi
    timeBeginPeriod(tc.wPeriodMin);
    DWORD start = timeGetTime();
    timeEndPeriod(tc.wPeriodMin);
    return TRUE;                        // dönüş değeri çağıranca yok sayılır
#else
    return TRUE;
#endif
}

// ============================================================================
// SubSystemShutdown (FUN_004CAC40)
// Binary Adresi: 0x004cac40
// XREF: sub_41AB90 (startup shutdown): 0041ab..+1A — startup.cpp shutdown zinciri
//
// Binary gövdesi:
//   1. dword_658944 (cihaz listesi başı) null → hemen dön
//   2. dword_65894C varsa: sub_50D4A0 sonucunu nesnenin +0x10h alanına
//      0x0B dword kopyala, dword_65894C = 0; yoksa nesne [+0x3C] geri
//      çağırısını (arg=3) çalıştır
//   3. Bağlı liste boyunca: her cihazın [+0x18h] geri çağırısı, engine
//      vtable (+0x138h) üzerinden alan bırakma, sub_4CBCE0(cihaz, 2)
//      ile serbest bırakma
//   4. Son olarak engine vtable (+0x138h) ile liste başı nesnesini bırak
// Cihaz listesi, motor alt sistem eklentileri (Faz 4.4b) tarafından
// doldurulur; o zamana kadar binary de null başlıkla sessiz döner.
// ============================================================================
void SubSystemShutdown(void)
{
    if (!s_pSubSystemList)
        return;

    // TODO(Faz 4.4b) — cihaz listesi zinciri + engine vtable[+0x138h]
    // bırakma işlemi: sub_50D4A0 kopyası, cihaz [+0x18h] geri çağrısı,
    // sub_4CBCE0(cihaz, 2). Liste restor edildiğinde burada yürünür.
    s_pSubSystemList = 0;
}

// ============================================================================
// ResourceShutdown (FUN_0040FD60)
// Binary Adresi: 0x0040fd60
// XREF: sub_41AB90 (startup shutdown): 0041ab..+2E — startup.cpp shutdown zinciri
//
// Assert path string'i dosya bağlamını kanıtlar:
//   "D:\Projects\CannonCruisePC\code\framework\core\attributehandler\c..."
//   "g_ptheMap->size() == 0 Failed" (cattributehandler.cpp, satır 0x53)
//
// Binary gövdesi:
//   1. dword_635650 (g_ptheMap = CAttributeHandler instance) null → dön
//   2. map size != 0 → assert "g_ptheMap->size() == 0 Failed"
//      (assert yoksa: "Assert failed:" + dword_63564C testi → _exit)
//   3. sub_411E20(instance) — dtor; sub_4053B0(instance) — delete
//   4. dword_635650 = 0
// Bu sözleşmenin mevcut karşılığı CAttributeHandler::Shutdown()tur
// (cattributehandler.cpp: FUN_0040fd60 referansıyla restore edilmiştir).
// ============================================================================
void ResourceShutdown(void)
{
    RWS::CAttributeHandler::Shutdown();
}

// ============================================================================
// GeneralShutdown (FUN_00405670)
// Binary Adresi: 0x00405670
// XREF: sub_41AB90 (startup shutdown): 0041ab..+33 — startup shutdown zinciri
//       (startup shutdown'un SON adımı)
//
// Binary gövdesi:
//   1. dword_635574 (genel abonelik/kayıt listesi — sub_4055F0 lazy-init eder)
//      null değilse: her kayıt için sub_4057A0 ile abonelik bağlantılarını
//      kaldır (listeden çıkar + sub_4053B0 ile serbest bırak, sayaç-1)
//   2. sub_48B3E0(kayıt) — kalan tüm bağlantıları toplu kaldır
//   3. sub_4053B0(kayıt) — nesneyi sil
//   4. dword_635574 = 0
//
// En yakın mevcut karşılık: olay sisteminin abonelik kayıtları
// (s_RegisteredEvents + s_EventSubscribers) — CEventHandler::Shutdown()
// ikisini de temizler. dword_635574'nin tam sınıf kimliği Faz 4.4b'de
// doğrulanacak.
// ============================================================================
void GeneralShutdown(void)
{
    RWS::CEventHandler::Shutdown();
}
