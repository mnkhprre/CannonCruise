// ============================================================================
// CannonCruise - Bink Video Player (BinkPlayer.cpp)
// See BinkPlayer.h — binkw32.dll v1.6c, __stdcall exports, 7-argument
// BinkCopyToBuffer. Playback loop mirrors binary sub_45F350/sub_45F270.
// ============================================================================

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include "BinkPlayer.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "RwsAssets.h" // <varlik-koku> adaylari — binkw32.dll konumu icin

namespace {

// --------------------------------------------------------------------------
// binkw32 signatures (from binkw32.dll v1.6c export table)
// --------------------------------------------------------------------------
typedef void *HBINK;

typedef HBINK(__stdcall *PFN_BinkOpen)(const char *name, unsigned int flags);
typedef void(__stdcall *PFN_BinkClose)(HBINK bink);
typedef void(__stdcall *PFN_BinkDoFrame)(HBINK bink);
typedef void(__stdcall *PFN_BinkNextFrame)(HBINK bink);
typedef unsigned int(__stdcall *PFN_BinkWait)(HBINK bink);
typedef unsigned int(__stdcall *PFN_BinkCopyToBuffer)(
    HBINK bink, void *dest, int destpitch, unsigned int a4, unsigned int a5,
    unsigned int a6, unsigned int mode);
typedef const char *(__stdcall *PFN_BinkGetError)(void);

struct BinkApi {
    HMODULE hModule;
    PFN_BinkOpen Open;
    PFN_BinkClose Close;
    PFN_BinkDoFrame DoFrame;
    PFN_BinkNextFrame NextFrame;
    PFN_BinkWait Wait;
    PFN_BinkCopyToBuffer CopyToBuffer;
    PFN_BinkGetError GetError;
};

BinkApi g_api = {nullptr, nullptr, nullptr, nullptr,
                 nullptr, nullptr, nullptr, nullptr};

char g_lastError[256] = {0};

// Export names: try binary's decorated __stdcall first, then plain.
struct BinkExportNames {
    const char *decorated;
    const char *plain;
};

const BinkExportNames kExports[] = {
    {"_BinkOpen@8", "BinkOpen"},
    {"_BinkClose@4", "BinkClose"},
    {"_BinkDoFrame@4", "BinkDoFrame"},
    {"_BinkNextFrame@4", "BinkNextFrame"},
    {"_BinkWait@4", "BinkWait"},
    {"_BinkCopyToBuffer@28", "BinkCopyToBuffer"},
    {"_BinkGetError@0", "BinkGetError"},
};
const int kExportCount = (int)(sizeof(kExports) / sizeof(kExports[0]));

FARPROC ResolveExport(HMODULE hModule, int index)
{
    FARPROC p = ::GetProcAddress(hModule, kExports[index].decorated);
    if (p == nullptr) {
        p = ::GetProcAddress(hModule, kExports[index].plain);
    }
    return p;
}

/// Try candidate path; if loaded, write (hModule, resolved path).
HMODULE TryLoad(const char *pPath)
{
    if (pPath == nullptr || pPath[0] == 0) {
        return nullptr;
    }
    HMODULE h = ::LoadLibraryA(pPath);
    if (h != nullptr) {
        std::snprintf(g_lastError, sizeof(g_lastError), "binkw32: %s", pPath);
    }
    return h;
}

// --------------------------------------------------------------------------
// BINK struct fields — offsets runtime verified (probe):
//   [0]=width  [4]=height  [8]=total frames  [0xC]=frame no
// --------------------------------------------------------------------------
inline unsigned int BinkField(const void *pBink, int offsetBytes)
{
    return *(const unsigned int *)((const unsigned char *)pBink + offsetBytes);
}

} // namespace

// --------------------------------------------------------------------------
// BinkPlayer_Initialize
// --------------------------------------------------------------------------
int BinkPlayer_Initialize(void)
{
    if (g_api.Open != nullptr) {
        return 1;
    }
    std::memset(&g_api, 0, sizeof(g_api));
    g_lastError[0] = 0;

    char probe[512] = {0};
    HMODULE h = nullptr;

    // 1) explicit location (diagnostics / custom setup)
    const char *pEnv = std::getenv("CANNONCRUISE_BINKW32");
    if (pEnv != nullptr && pEnv[0] != 0) {
        h = TryLoad(pEnv);
    }
    // 2) standard Windows search (exe dir, CWD, PATH)
    if (h == nullptr) {
        h = ::LoadLibraryA("binkw32.dll");
        if (h != nullptr) {
            std::snprintf(g_lastError, sizeof(g_lastError),
                          "binkw32: Windows arama yolu");
        }
    }
    // 3) asset root — binkw32.dll and DLLs\ subdir (CD install layout)
    const char *pRoot = RwsAssets_Dir();
    if (h == nullptr && pRoot != nullptr && pRoot[0] != 0) {
        std::snprintf(probe, sizeof(probe), "%sbinkw32.dll", pRoot);
        h = TryLoad(probe);
    }
    if (h == nullptr && pRoot != nullptr && pRoot[0] != 0) {
        std::snprintf(probe, sizeof(probe), "%sDLLs/binkw32.dll", pRoot);
        h = TryLoad(probe);
    }
    // 4) relative candidates (CWD-dependent build directories)
    if (h == nullptr) {
        static const char *const kRel[] = {"DLLs/binkw32.dll", "binkw32.dll",
                                           "DLLs\\binkw32.dll"};
        for (int i = 0; i < 3 && h == nullptr; ++i) {
            h = TryLoad(kRel[i]);
        }
    }
    if (h == nullptr) {
        std::snprintf(g_lastError, sizeof(g_lastError),
                      "binkw32: failed to load (err=%lu)", ::GetLastError());
        return 0;
    }

    FARPROC *slots[(int)(sizeof(BinkApi) / sizeof(FARPROC))];
    slots[0] = (FARPROC *)&g_api.Open;
    slots[1] = (FARPROC *)&g_api.Close;
    slots[2] = (FARPROC *)&g_api.DoFrame;
    slots[3] = (FARPROC *)&g_api.NextFrame;
    slots[4] = (FARPROC *)&g_api.Wait;
    slots[5] = (FARPROC *)&g_api.CopyToBuffer;
    slots[6] = (FARPROC *)&g_api.GetError;

    for (int i = 0; i < kExportCount; ++i) {
        *slots[i] = ResolveExport(h, i);
        if (*slots[i] == nullptr) {
            std::snprintf(g_lastError, sizeof(g_lastError),
                          "binkw32: missing export %s / %s", kExports[i].decorated,
                          kExports[i].plain);
            ::FreeLibrary(h);
            std::memset(&g_api, 0, sizeof(g_api));
            return 0;
        }
    }

    g_api.hModule = h;
    return 1;
}

int BinkPlayer_IsAvailable(void)
{
    return (g_api.Open != nullptr) ? 1 : 0;
}

void BinkPlayer_Shutdown(void)
{
    if (g_api.hModule != nullptr) {
        ::FreeLibrary(g_api.hModule);
    }
    std::memset(&g_api, 0, sizeof(g_api));
    g_lastError[0] = 0;
}

// --------------------------------------------------------------------------
// BinkPlayer_Open — sub_45F350: BinkOpen(path, 0x4000)
// --------------------------------------------------------------------------
void *BinkPlayer_Open(const char *pPath)
{
    if (pPath == nullptr || pPath[0] == 0) {
        return nullptr;
    }
    if (BinkPlayer_Initialize() != 1) {
        return nullptr;
    }
    HBINK bink = g_api.Open(pPath, BINKPLAYER_OPEN_FLAGS);
    if (bink == nullptr) {
        const char *pErr = (g_api.GetError != nullptr) ? g_api.GetError()
                                                       : nullptr;
        std::snprintf(g_lastError, sizeof(g_lastError), "BinkOpen: %s",
                      (pErr != nullptr && pErr[0] != 0) ? pErr : "unknown");
        return nullptr;
    }
    std::snprintf(g_lastError, sizeof(g_lastError), "opened: %s", pPath);
    return (void *)bink;
}

int BinkPlayer_Width(const void *pBink)
{
    return (pBink != nullptr) ? (int)BinkField(pBink, 0) : 0;
}

int BinkPlayer_Height(const void *pBink)
{
    return (pBink != nullptr) ? (int)BinkField(pBink, 4) : 0;
}

int BinkPlayer_TotalFrames(const void *pBink)
{
    return (pBink != nullptr) ? (int)BinkField(pBink, 8) : 0;
}

int BinkPlayer_CurrentFrame(const void *pBink)
{
    return (pBink != nullptr) ? (int)BinkField(pBink, 0x0C) : 0;
}

int BinkPlayer_IsFinished(const void *pBink)
{
    // sub_45F270: cmp [eax+0Ch], [eax+8] / jnz devam / xor al,al (bitis)
    if (pBink == nullptr) {
        return 1;
    }
    return (BinkField(pBink, 0x0C) == BinkField(pBink, 8)) ? 1 : 0;
}

// --------------------------------------------------------------------------
// BinkPlayer_Wait — game loop: while (BinkWait(bink) != 0) { ... }
// No Sleep: calling BinkWait back-to-back is required for decode (back-to-back).
// Return !=0 breaks infinite loop and decode proceeds.
// --------------------------------------------------------------------------
void BinkPlayer_Wait(void *pBink, int (*pIdle)(void))
{
    if (pBink == nullptr || g_api.Wait == nullptr) {
        return;
    }
    // Infinite loop protection — Bink frame accelerator returns 0 once per frame
    // so this limit is never reached; added for safety anyway.
    // After 100 iterations decode proceeds.
    for (int i = 0; i < 100; ++i)
    {
        if (g_api.Wait((HBINK)pBink) == 0) {
            return;
        }
        if (pIdle != nullptr && pIdle() != 0) {
            return;
        }
    }
    // finite wait end: decode proceeds without blocking
}

// --------------------------------------------------------------------------
// BinkPlayer_DecodeRGBA — sub_45F270: DoFrame → CopyToBuffer → conversion
// Output X8R8G8B8 (B,G,R,X) → RGBA (R,G,B,255).
// --------------------------------------------------------------------------
int BinkPlayer_DecodeRGBA(void *pBink, unsigned char *pOut, int pitch)
{
    if (pBink == nullptr || pOut == nullptr || g_api.DoFrame == nullptr ||
        g_api.CopyToBuffer == nullptr) {
        return 0;
    }
    const int w = BinkPlayer_Width(pBink);
    const int h = BinkPlayer_Height(pBink);
    if (w <= 0 || h <= 0 || pitch < w * 4) {
        return 0;
    }

    g_api.DoFrame((HBINK)pBink);
    // mode = 0x80000003 → 32 bpp X8R8G8B8 (same as binary sub_45F270+6E)
    const unsigned int r = g_api.CopyToBuffer(
        (HBINK)pBink, pOut, pitch, 0u, 0u, 0u, 0x80000003u);
    if (r != 0u) {
        // binary: test eax,eax / setz al — only 0 means success
        return 0;
    }

    // B,G,R,X → R,G,B,255 (UploadTexture expects RGBA)
    for (int y = 0; y < h; ++y) {
        unsigned char *row = pOut + (size_t)y * (size_t)pitch;
        for (int x = 0; x < w; ++x) {
            unsigned char *px = row + (size_t)x * 4u;
            const unsigned char b = px[0];
            px[0] = px[2]; // R
            px[2] = b;     // B
            px[3] = 255;   // alfa yok → opak
        }
    }
    return 1;
}

void BinkPlayer_Next(void *pBink)
{
    if (pBink != nullptr && g_api.NextFrame != nullptr) {
        g_api.NextFrame((HBINK)pBink);
    }
}

void BinkPlayer_Close(void *pBink)
{
    if (pBink != nullptr && g_api.Close != nullptr) {
        g_api.Close((HBINK)pBink);
    }
}

const char *BinkPlayer_LastError(void)
{
    return g_lastError;
}