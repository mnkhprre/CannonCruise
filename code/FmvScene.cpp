// ============================================================================
// CannonCruise - FMV Scene Descriptor (FmvScene.cpp)
// ============================================================================

#include "FmvScene.h"
#include "RwsAssets.h"

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <windows.h>
#include <strsafe.h>

// ---------------------------------------------------------------------------
// FMV descriptors ~1.2 KB; levels 1.8-19.7 MB -> 64 KB safe limit.
// ---------------------------------------------------------------------------
const long kDescriptorMaxSize = 64L * 1024L;

// Copy string into buffer (including terminator).
void CopyToken(char *pDst, size_t dstSize, const char *pSrc)
{
    if (pDst == nullptr || dstSize == 0) {
        return;
    }
    std::snprintf(pDst, dstSize, "%s", (pSrc != nullptr) ? pSrc : "");
}

// File exists? (existence probe only — no read)
int FileProbe(const char *pPath)
{
    if (pPath == nullptr || pPath[0] == 0) {
        return 0;
    }
    FILE *f = std::fopen(pPath, "rb");
    if (f == nullptr) {
        return 0;
    }
    std::fclose(f);
    return 1;
}

// ---------------------------------------------------------------------------
// FmvScene_Load -- resolve + read + parse per CFMVBehavior.
// ---------------------------------------------------------------------------
int FmvScene_Load(const char *pRelativePath, FmvSceneDescriptor *pOut)
{
    if (pRelativePath == nullptr || pOut == nullptr) {
        return 0;
    }
    std::memset(pOut, 0, sizeof(*pOut));

    char path[512] = {0};
    if (RwsAssets_Resolve(pRelativePath, path, sizeof(path)) != 1) {
        return 0; // file not at this layout
    }

    FILE *f = std::fopen(path, "rb");
    if (f == nullptr) {
        return 0;
    }
    std::fseek(f, 0, SEEK_END);
    const long size = std::ftell(f);
    std::fseek(f, 0, SEEK_SET);
    if (size <= 0 || size > kDescriptorMaxSize) {
        std::fclose(f);
        return 0; // level/world -- not an FMV descriptor (chain end)
    }
    std::vector<unsigned char> buf((size_t)size);
    const size_t got = std::fread(buf.data(), 1, (size_t)size, f);
    std::fclose(f);
    if (got != (size_t)size) {
        return 0;
    }

    // GUID: scan whole buffer for '{' ... '}' — closing '}' sometimes outside
    // chunk boundary (clen) (happened in OUTRO1) -> in-chunk scan missed it.
    // Validity: printable only + at least one '-' (real {GUID} format) —
    // binary leftover '{' candidates eliminated.
    for (size_t j = 0; pOut->guid[0] == 0 && j + 8 < buf.size(); ++j) {
        if (buf[j] != '{') {
            continue;
        }
        char g[sizeof(pOut->guid)] = {0};
        size_t k = 0;
        size_t m = j;
        while (m < buf.size() && buf[m] != '}' && k < sizeof(g) - 1) {
            const unsigned char c = buf[m];
            if (c < 32 || c >= 127) {
                break; // binary leftover — not this candidate
            }
            g[k++] = (char)c;
            ++m;
        }
        if (m >= buf.size() || buf[m] != '}') {
            continue; // no closing
        }
        if (std::strchr(g, '-') == nullptr) {
            continue; // not {GUID}
        }
        g[k] = '}';
        std::snprintf(pOut->guid, sizeof(pOut->guid), "%s", g);
    }

    // chunk scan: behavior name (FMV) + target scene extracted
    for (size_t off = 0; off + 12 <= buf.size();) {
        unsigned int ctype = 0;
        unsigned int clen = 0;
        std::memcpy(&ctype, buf.data() + off, 4);
        std::memcpy(&clen, buf.data() + off + 4, 4);
        if (clen < 4 || off + 12 + clen > buf.size()) {
            break;
        }
        const unsigned char *pl = buf.data() + off + 12;
        // printable long run (behavior + target)
        if (ctype == 0x0704 || ctype == 0x071c) {
            char run[256];
            unsigned int rn = 0;
            for (unsigned int j = 0; j < clen && rn < sizeof(run) - 1; ++j) {
                const unsigned char c = pl[j];
                run[rn++] = (c >= 32 && c < 127) ? (char)c : '\n';
            }
            run[rn] = 0;
            // split by '\n' + trim; skip Behavior-containing ones;
            // first remaining short token = behavior instance name ("Jingle"/"Intro")
            // next = target scene ("MainMenu"/"Nordic01").
            for (char *tok = std::strtok(run, "\n"); tok != nullptr;
                 tok = std::strtok(nullptr, "\n")) {
                while (*tok == ' ') {
                    ++tok;
                }
                const size_t l = std::strlen(tok);
                // min 5: short random ASCII runs added by binary leftovers
                // (CREDITS has "chn", OUTRO1 has "lXM"). Real FMV and target
                // names all >= 5 (Intro/Outro=5, MainMenu=8, Nordic01=8).
                if (l < 5 || l >= 32) {
                    continue;
                }
                // identifier-like only: [A-Za-z0-9_]
                int ident = 1;
                for (size_t ci = 0; ci < l; ++ci) {
                    const char c = tok[ci];
                    if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
                          (c >= '0' && c <= '9') || c == '_')) {
                        ident = 0;
                        break;
                    }
                }
                if (ident == 0) {
                    continue; // binary leftover
                }
                if (std::strstr(tok, "Behavior") != nullptr ||
                    std::strstr(tok, "Attribute") != nullptr) {
                    continue; // class/object names — skip
                }
                if (pOut->behaviour[0] == 0) {
                    CopyToken(pOut->behaviour, sizeof(pOut->behaviour), tok);
                }
                else if (pOut->target[0] == 0) {
                    CopyToken(pOut->target, sizeof(pOut->target), tok);
                }
            }
        }
        off += 12 + clen;
    }

    // FMV descriptor signature: CFMVBehavior behavior name + target present.
    if (pOut->behaviour[0] == 0 || pOut->target[0] == 0) {
        return 0;
    }
    return 1;
}

// ---------------------------------------------------------------------------
// FmvScene_VideoPath -- sub_45F950 case 0: <base>"Video\Win32\<name>.bik"
// ---------------------------------------------------------------------------
int FmvScene_VideoPath(const char *pFmvName, char *pOut, size_t outSize)
{
    if (pFmvName == nullptr || pFmvName[0] == 0 || pOut == nullptr ||
        outSize == 0) {
        return 0;
    }
    // base = "\." (loc_45FA0F — when dword_635AB4 is zero)
    std::snprintf(pOut, outSize, ".\\Video\\Win32\\%s.bik", pFmvName);
    return 1;
}

// ---------------------------------------------------------------------------
// FmvScene_VideoResolve -- resolved full path of .bik file.
// Candidate order: (1) CWD ".\Video\Win32\<name>.bik" (sub_45F950 case0 /
// loc_45FA0F — base = "\."), (2) asset root "<root>Video/Win32/<name>.bik"
// (root found via Streams/ sentinel; CD install has Video\ resource tree
// at root), (3) CANNONCRUISE_VIDEO_ROOT env var specified root
// ("<root>/Video/Win32/<name>.bik"), (4) walk up 4 steps from CWD looking
// for Video\Win32\<name>.bik (for test environment and different build layouts).
// ---------------------------------------------------------------------------
int FmvScene_VideoResolve(const char *pFmvName, char *pOut, size_t outSize)
{
    if (pFmvName == nullptr || pFmvName[0] == 0 || pOut == nullptr ||
        outSize == 0) {
        return 0;
    }
    pOut[0] = 0;

    char cand[512] = {0};
    if (FmvScene_VideoPath(pFmvName, cand, sizeof(cand)) != 1) {
        return 0;
    }
    if (FileProbe(cand) == 1) {
        std::snprintf(pOut, outSize, "%s", cand);
        return 1;
    }
    const char *pRoot = RwsAssets_Dir();
    if (pRoot != nullptr && pRoot[0] != 0) {
        std::snprintf(cand, sizeof(cand), "%sVideo/Win32/%s.bik", pRoot,
                      pFmvName);
        if (FileProbe(cand) == 1) {
            std::snprintf(pOut, outSize, "%s", cand);
            return 1;
        }
    }
    // 3. fallback: CANNONCRUISE_VIDEO_ROOT env variable
    const char *pEnv = std::getenv("CANNONCRUISE_VIDEO_ROOT");
    if (pEnv != nullptr && pEnv[0] != 0) {
        std::snprintf(cand, sizeof(cand), "%sVideo/Win32/%s.bik", pEnv,
                      pFmvName);
        if (FileProbe(cand) == 1) {
            std::snprintf(pOut, outSize, "%s", cand);
            return 1;
        }
    }
    // 4. fallback: walk up 4 steps from CWD looking for Video\Win32
    // (test environment and different build layouts).
    // Test CWD: code\build_win32_v180\tests\Release
    // Repo root is 4 steps up: code → build → tests → Release → CannonCruise
    {
        char cwd[512];
        if (GetCurrentDirectoryA(512, cwd) > 0) {
            // Remove last backslash at each step (using strrchr)
            char parent[512];
            lstrcpynA(parent, cwd, 512);
            for (int i = 0; i < 4; ++i) {  // 4 steps = repo root
                char *lastBS = strrchr(parent, '\\');
                if (lastBS != nullptr) {
                    *lastBS = 0;
                } else {
                    break;
                }
            }
            // parent is now 4 steps up from CWD (repo root: CannonCruise)
            // remove trailing backslash and add Video\Win32
            size_t plen = lstrlenA(parent);
            if (plen > 1 && parent[plen - 1] == '\\') {
                parent[plen - 1] = 0;
            }
            std::snprintf(cand, sizeof(cand), "%sVideo/Win32/%s.bik", parent,
                          pFmvName);
            if (FileProbe(cand) == 1) {
                std::snprintf(pOut, outSize, "%s", cand);
                return 1;
            }
        }
    }
    return 0;
}

// ---------------------------------------------------------------------------
// FmvScene_VideoExists — does .bik exist? (wrapper reduced to resolver)
// ---------------------------------------------------------------------------
int FmvScene_VideoExists(const char *pFmvName)
{
    char full[512] = {0};
    return (FmvScene_VideoResolve(pFmvName, full, sizeof(full)) == 1) ? 1 : 0;
}

// ---------------------------------------------------------------------------
// FmvScene_NextScenePath -- target scene -> next descriptor RELATIVE path.
// Direct match first; UPPERCASE fallback (all .RWS files uppercase).
// ---------------------------------------------------------------------------
int FmvScene_NextScenePath(const char *pTarget, char *pOut, size_t outSize)
{
    if (pTarget == nullptr || pTarget[0] == 0 || pOut == nullptr ||
        outSize == 0) {
        return 0;
    }
    pOut[0] = 0;

    char rel[256];
    char full[512];

    // 1) exact: Streams/<target>.RWS
    std::snprintf(rel, sizeof(rel), "Streams/%s.RWS", pTarget);
    if (RwsAssets_Resolve(rel, full, sizeof(full)) == 1) {
        std::snprintf(pOut, outSize, "%s", rel);
        return 1;
    }

    // 2) uppercase: NORDIC01.RWS / MAINMENU.RWS / IDUNA02.RWS
    char upper[128] = {0};
    size_t ui = 0;
    for (; ui + 1 < sizeof(upper) && pTarget[ui] != 0; ++ui) {
        upper[ui] =
            (char)std::toupper((unsigned char)pTarget[ui]);
    }
    upper[ui] = 0;
    std::snprintf(rel, sizeof(rel), "Streams/%s.RWS", upper);
    if (RwsAssets_Resolve(rel, full, sizeof(full)) == 1) {
        std::snprintf(pOut, outSize, "%s", rel);
        return 1;
    }
    return 0;
}