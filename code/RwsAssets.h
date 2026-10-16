// ============================================================================
// CannonCruise - .RWS Asset Path Resolver (RwsAssets.h)
//
// Problem: Streams/ directory is at asset tree root, but working directory
// (CWD) varies by build output (e.g. code/build_win32_v180/Release →
// root 3 levels up). main.cpp had 5 separate places with hand-written relative
// prefix lists ("" / "../" / "../../"); these missed deep build layouts so
// assets failed to load (clump=0, assets=0).
//
// Solution: single central resolver.
//   1) CANNONCRUISE_STREAMS env var → root dir (overrides default)
//   2) Otherwise candidate prefix scan — validated by sentinel file (MAINMENU.RWS)
//   3) Found prefix is cached → all subsequent calls are fast
// ============================================================================
#pragma once

#include <cstddef>

/// Resolve and cache .RWS asset root directory (directory ending with '/').
/// Returns empty string if not found.
const char* RwsAssets_Dir(void);

/// Directory + filename → full path. If file EXISTS, writes to pOut and returns 1;
/// if not found, writes pOut[0] = 0 and returns 0.
/// pFileName is given in "Streams/X.RWS" format (merged with root directory).
int RwsAssets_Resolve(const char* pFileName, char* pOut, size_t outSize);

/// Reset cache — TEST ONLY. Next call re-evaluates env var and candidate
/// prefix scan.
void RwsAssets_Reset(void);
