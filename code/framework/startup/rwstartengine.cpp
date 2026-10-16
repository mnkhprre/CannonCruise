// ============================================================================
// CannonCruise - RwStartEngine (rwstartengine.cpp)
// Original path: D:\Projects\CannonCruisePC\code\framework\startup\rwstartengine.cpp
// Tersine mühendislik ile CC_ida.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonu:
//   FUN_00418ac0 — RwStartEngine (startup init zinciri, "Adım 7")
//
// Not: Dosya bağlamı yalnızca startup.cpp'deki çağrı XREF'inden biliniyor;
//      fonksiyon içi assert path string'i yok.
// ============================================================================

#include "rwcore.h"
#include <cstdlib>
#include <cstring>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>        // GetCurrentDirectoryA / GetDriveTypeA
#include <direct.h>         // _getdrive / _getcwd / _chdir
#else
#include <unistd.h>         // getcwd / chdir
#include <sys/stat.h>       // stat — arama yolu doğrulama
#endif

namespace
{
    // sub_4CB8D0 — arama yolu girişi (binary: arg_0 = 0x14 = 20 bayt yapı;
    // arg_1 = kök dizin ("X:" ya da cwd), arg_2 = ad tek harf/"."/
    // off_612FE0 varsayılanı). Yapı binary'de FS sistemi tarafından
    // sahiplenilir; Faz 4.4b'de RtFS/CustomFS restorasyonuna bağlanır.
    // NOT: "SearchPath" adı Windows API (processenv.h) ile çakışıyor —
    // "RwSearchPath" olarak yeniden adlandırıldı.
    struct RwSearchPath
    {
        char        path[0x104];
        char        name[8];
        bool        active;
        RwSearchPath* pNext;
    };

    RwSearchPath* s_pSearchPaths = 0;    // kayıtlı arama yolları (bağlı liste)
    RwSearchPath* s_pActivePath = 0;     // sub_4CAFC0 ile aktifleşen son giriş

    // sub_4CB8D0(0x14, path, name) — yeni arama yolu girişi oluştur
    RwSearchPath* CreateSearchPath(const char* pPath, const char* pName)
    {
        RwSearchPath* p = static_cast<RwSearchPath*>(calloc(1, sizeof(RwSearchPath)));
        if (!p)
            return 0;
        strncpy(p->path, pPath, sizeof(p->path) - 1);
        strncpy(p->name, pName, sizeof(p->name) - 1);
        p->pNext = s_pSearchPaths;
        s_pSearchPaths = p;
        return p;
    }

    // sub_4CAD20(pPath) — girişin geçerli (mevcut dizin) olduğunu doğrula
    RwBool ValidateSearchPath(const RwSearchPath* pPath)
    {
        if (!pPath || pPath->path[0] == 0)
            return FALSE;
#ifdef _WIN32
        DWORD attrs = GetFileAttributesA(pPath->path);
        return (attrs != INVALID_FILE_ATTRIBUTES &&
                (attrs & FILE_ATTRIBUTE_DIRECTORY)) ? TRUE : FALSE;
#else
        struct stat st;
        return (stat(pPath->path, &st) == 0 && S_ISDIR(st.st_mode)) ? TRUE : FALSE;
#endif
    }

    // sub_4CAFC0(pPath) — girişi aktif arama yolu yap
    void ActivateSearchPath(RwSearchPath* pPath)
    {
        if (pPath)
        {
            pPath->active = true;
            s_pActivePath = pPath;
        }
    }
}

// ============================================================================
// RwStartEngine (FUN_00418AC0)
// Binary Adresi: 0x00418ac0
// XREF: sub_41A680 (startup init): 0041aa66 — startup.cpp "Adım 7"
// Dönüş: TRUE = arama yolu hazır / FALSE = arama yolu kurulamadı
//
// Binary gövdesi (MSVC CRT + KERNEL32):
//   1. drive = __getdrive(); cwd = __getcwd(Path, 0x104)
//   2. drive < 1 (geçerli sürücü yok):
//        n = GetCurrentDirectoryA(Buffer, 0x100)
//        n == 0 veya n > 0x100 → FALSE
//        engine dword_65D7A4 vtable[+0x100h](Buffer, unk_612E84)
//          — çalışma dizinini motora bildir
//        esi = sub_4CB8D0(0x14, Buffer, off_612FE0) — arama yolu oluştur
//        esi null veya sub_4CAD20(esi) başarısız → FALSE
//        sub_4CAFC0(esi) — aktif arama yolu yap → TRUE
//   3. drive >= 1: B: .. Z: tara (ebx = 2 .. 0x1A)
//        GetDriveTypeA("X:\"); X: yap; tip 0/1 (UNKNOWN/NO_ROOT_DIR) → atla
//        sub_4CB8D0(0x14, "X:", "X") — ad = sürücü harfi
//        oluşturma/doğrulama başarısız → FALSE (döngü kırılır)
//        sürücü == geçerli sürücü ise sub_4CAFC0 ile aktifleştir
//        (döngü aktifleştirmeden sonra da devam eder)
//   4. __chdir(Path) — eski çalışma dizinine dön → TRUE
//
// Faz 4.4b TODO: engine vtable[+0x100h] çağrısı (dword_65D7A4 motor nesnesi
// RwEngineOpen sonrası mevcut olacak).
// ============================================================================
RwBool RwStartEngine(void)
{
#ifdef _WIN32
    int  drive = _getdrive();
    char savedCwd[0x104];
    savedCwd[0] = 0;
    _getcwd(savedCwd, 0x104);

    // --- Şube 2: geçerli sürücü yok ---
    if (drive < 1)
    {
        char cwd[0x100];
        DWORD n = GetCurrentDirectoryA(sizeof(cwd), cwd);
        if (n == 0 || n > sizeof(cwd))
            return FALSE;

        // TODO(Faz 4.4b): engine dword_65D7A4 vtable[+0x100h](cwd, unk_612E84)
        RwSearchPath* pPath = CreateSearchPath(cwd, ".");   // off_612FE0 (varsayılan ad)
        if (!pPath)
            return FALSE;
        if (!ValidateSearchPath(pPath))
            return FALSE;
        ActivateSearchPath(pPath);
        return TRUE;
    }

    // --- Şube 3: B: .. Z: sürücü taraması ---
    for (int d = 2; d <= 26; ++d)
    {
        char root[4];
        root[0] = static_cast<char>('A' + d - 1);
        root[1] = ':';
        root[2] = '\\';
        root[3] = 0;
        UINT driveType = GetDriveTypeA(root);

        root[2] = 0;                                // "X:" (binary: var_20A = 0)
        if (driveType == DRIVE_UNKNOWN || driveType == DRIVE_NO_ROOT_DIR)
            continue;

        char name[2];
        name[0] = root[0];                          // ad = sürücü harfi
        name[1] = 0;

        RwSearchPath* pPath = CreateSearchPath(root, name);
        if (!pPath)
            return FALSE;                           // binary: loc_418B5E → FALSE
        if (!ValidateSearchPath(pPath))
            return FALSE;

        if (d == drive)                             // binary: cmp ebp, ebx
            ActivateSearchPath(pPath);              // yalnızca geçerli sürücüde aktifleştir
    }

    if (savedCwd[0])
        _chdir(savedCwd);                           // __chdir(Path)
    return TRUE;

#else
    // POSIX karşılığı: tek arama yolu = çalışma dizini (şube 2 mantığı)
    char cwd[0x104];
    if (!getcwd(cwd, sizeof(cwd)))
        return FALSE;

    RwSearchPath* pPath = CreateSearchPath(cwd, ".");
    if (!pPath)
        return FALSE;
    if (!ValidateSearchPath(pPath))
        return FALSE;
    ActivateSearchPath(pPath);
    return TRUE;
#endif
}
