// ============================================================================
// CannonCruise - GameManager bootstrap (gamemanager.cpp)
// Original path: D:\Projects\CannonCruisePC\code\framework\startup\gamemanager.cpp
// Tersine mühendislik ile CC_ida.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_0044aaf0 — GameManagerInit   (startup init zinciri, "Adım 15")
//   FUN_0044aba0 — GameManagerShutdown (startup shutdown zinciri, +24)
//   sub_449d30   — GameManager giriş tablosu (serbest liste) kurulumu
//
// Not: Dosya bağlamı yalnızca startup.cpp'deki çağrıdan biliniyor;
//      binary'de bu fonksiyonlara ait assert path string'i yoktur.
// ============================================================================

#include "rwcore.h"
#include <cstdlib>

namespace
{
    // GameManager giriş kaydı — 12 (0x0C) bayt (binary: sub_4053F0(0Ch, 2))
    //   [+0x00] serbest liste başlangıç imleci (sub_449D30 tarafından yazılır)
    //   [+0x04] giriş sayısı (binary: 0x2710 = 10000)
    //   [+0x08] giriş tablosu (binary: 0x1D4C0 = 10000 * 12 bayt)
    struct GameManagerEntry
    {
        void*    pCursor;   // +0x00 — serbest liste imleci (tabloya işaret)
        RwUInt32 entryCount;// +0x04
        void*    pEntries;  // +0x08
    };

    // dword_636878 — GameManager tekil örnek
    GameManagerEntry* s_pGameManager = 0;
}

// ---------------------------------------------------------------------------
// GameManager giriş tablosunun serbest listesini kurar (binary: sub_449D30)
// thiscall: ECX = this. Tablo elemanları 12 baytlık zincir oluşturur:
//   entry[i].+0x04 = &entry[i+1]  (sonraki)
//   entry[i].+0x08 = &entry[i-1]  (önceki)
//   entry[count-1].+0x08 = &entry[count-2]; this.+0x00 = tablo başlangıcı
// ---------------------------------------------------------------------------
static void InitGameManagerEntries(GameManagerEntry* pThis)
{
    RwUInt32 count = pThis->entryCount;
    char*    pTable = static_cast<char*>(pThis->pEntries);

    if (count <= 1)
    {
        // loc_449DAD: tek giriş — zincir başı 0, imleç = tablo
        if (pTable)
        {
            *reinterpret_cast<void**>(pTable + 4) = 0;
        }
        pThis->pCursor = pTable;
        return;
    }

    // İlk girişin arka bağlantısı yok (+0x08 = 0)
    *reinterpret_cast<void**>(pTable + 8) = 0;

    // entry[i].+0x04 = &entry[i+1]  (i = 0 .. count-2)
    // entry[i].+0x08 = &entry[i-1]  (i = 1 .. count-1)
    for (RwUInt32 i = 0; i + 1 < count; ++i)
    {
        char* pEntry = pTable + i * 12;
        *reinterpret_cast<void**>(pEntry + 4) = pEntry + 12;
        if (i > 0)
        {
            *reinterpret_cast<void**>(pEntry + 8) = pEntry - 12;
        }
    }

    // Son girişin arkası = son öncesi (binary tail: [edx+eax*4-4])
    char* pLast = pTable + (count - 1) * 12;
    *reinterpret_cast<void**>(pLast + 8) = pTable + (count - 2) * 12;

    // this.+0x00 = tablo başlangıcı (binary: mov [ecx], edx)
    pThis->pCursor = pTable;
}

// ============================================================================
// GameManagerInit (FUN_0044AAF0)
// Binary Adresi: 0x0044aaf0
// XREF: sub_41A680 (startup init): 0041a6..+4E5 — startup.cpp "Adım 15"
//
// dword_636878 null ise yeni GameManager oluşturur:
//   1. 12 baytlık yönetici nesnesi (sub_4053F0(0Ch, 2))
//   2. entryCount = 0x2710 (10000); pEntries = new(0x1D4C0 = 10000 * 12)
//   3. sub_449D30 ile serbest liste zinciri kurulur
// Binary'de nesne zaten varsa (dword_636878 != 0) hiç dokunmadan döner.
// (SEH kapsam koruyucu: sub_404E70(unk_657BA8) / sub_404E70(0) — yorumda.)
// ============================================================================
void GameManagerInit(void)
{
    if (s_pGameManager)
        return;

    GameManagerEntry* p = static_cast<GameManagerEntry*>(calloc(1, sizeof(GameManagerEntry)));
    if (!p)
        return;

    p->entryCount = 0x2710;                         // mov [esi+4], 2710h
    p->pEntries = calloc(p->entryCount, 12);        // sub_4053F0(1D4C0h, 2)
    if (!p->pEntries)
    {
        free(p);
        return;
    }

    InitGameManagerEntries(p);                      // sub_449D30

    s_pGameManager = p;                             // mov dword_636878, esi
}

// ============================================================================
// GameManagerShutdown (FUN_0044ABA0)
// Binary Adresi: 0x0044aba0
// XREF: sub_41AB90 (startup shutdown): +24 — startup.cpp shutdown zinciri
//
// dword_636878 null ise atlar. Değilse:
//   1. pEntries (giriş tablosu) serbest (sub_4053B0 = operator delete)
//   2. yönetici nesnesi serbest
//   3. dword_636878 = 0
// ============================================================================
void GameManagerShutdown(void)
{
    GameManagerEntry* p = s_pGameManager;
    if (!p)
        return;

    free(p->pEntries);      // sub_4053B0(p->pEntries)
    free(p);                // sub_4053B0(p)
    s_pGameManager = 0;     // mov dword_636878, 0
}
