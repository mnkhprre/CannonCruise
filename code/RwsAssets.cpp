// ============================================================================
// CannonCruise - .RWS Asset Path Resolver (RwsAssets.cpp)
// See RwsAssets.h — sentinel scan + CANNONCRUISE_STREAMS env override.
// ============================================================================

#include "RwsAssets.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace
{
    // Sentinel — file found under Streams/ in every build. Candidate prefixes
    // are validated against this to find root directory.
    const char* kSentinel = "Streams/MAINMENU.RWS";

    // Candidate relative prefixes — including deep VS build layouts
    // (code/build_<x>/Release root is 3 levels above).
    const char* const kPrefixes[] = {
        "", "../", "../../", "../../../", "../../../../", "../../../../../",
    };
    const int kPrefixCount = (int)(sizeof(kPrefixes) / sizeof(kPrefixes[0]));

    char g_dir[64] = {0};   // cache: working prefix ('/' terminated) or env
    int  g_resolved = 0;    // 0 = not yet tried

    int FileExists(const char* pPath)
    {
        FILE* f = std::fopen(pPath, "rb");
        if (f == nullptr) {
            return 0;
        }
        std::fclose(f);
        return 1;
    }
}

const char* RwsAssets_Dir(void)
{
    if (g_resolved) {
        return g_dir;
    }
    g_resolved = 1;

    // 1) Environment variable override path (test/distribution flexibility)
    const char* pEnv = std::getenv("CANNONCRUISE_STREAMS");
    if (pEnv != nullptr && pEnv[0] != '\0') {
        std::size_t n = std::strlen(pEnv);
        if (n >= sizeof(g_dir)) {
            n = sizeof(g_dir) - 1;
        }
        std::memcpy(g_dir, pEnv, n);
        g_dir[n] = '\0';
        // normalize trailing separator
        if (g_dir[n - 1] != '/' && g_dir[n - 1] != '\\') {
            if (n + 1 < sizeof(g_dir)) {
                g_dir[n] = '/';
                g_dir[n + 1] = '\0';
            }
        }
        return g_dir;
    }

    // 2) candidate prefix scan — sentinel validation
    char probe[256];
    for (int i = 0; i < kPrefixCount; ++i) {
        std::snprintf(probe, sizeof(probe), "%s%s", kPrefixes[i], kSentinel);
        if (FileExists(probe)) {
            std::size_t n = std::strlen(kPrefixes[i]);
            std::memcpy(g_dir, kPrefixes[i], n + 1); // NUL dahil
            return g_dir;
        }
    }

    // not found — empty directory (caller follows file-not-found path)
    g_dir[0] = '\0';
    return g_dir;
}

void RwsAssets_Reset(void)
{
    g_dir[0] = '\0';
    g_resolved = 0;
}

int RwsAssets_Resolve(const char* pFileName, char* pOut, size_t outSize)
{
    if (pOut == nullptr || outSize == 0) {
        return 0;
    }
    pOut[0] = '\0';
    if (pFileName == nullptr || pFileName[0] == '\0') {
        return 0;
    }

    const char* pDir = RwsAssets_Dir();
    char full[512];
    std::snprintf(full, sizeof(full), "%s%s", pDir, pFileName);

    if (!FileExists(full)) {
        return 0;
    }
    std::snprintf(pOut, outSize, "%s", full);
    return 1;
}
