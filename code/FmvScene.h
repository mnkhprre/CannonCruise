// ============================================================================
// CannonCruise - FMV Scene Descriptor (FmvScene.h)
//
// Binary sources (CC_ida.asm):
//   sub_45F6D0 — CClassFactory::Register("CFMVBehavior", sub_45F580,
//                0x12C, 1, 1)  [FUN_00413700 = Register]
//   sub_45F950 — CFMVBehavior::apply (4-case property switch):
//                  case 0 → [this+0x20] = <base> "Video\Win32\" <FMV name> ".bik"
//                           <base> = dword_635AB4 ? dword_635AB0 : ".\"
//                           (loc_45FA0F)  and [this+0x124] = 1
//                  case 1 → [this+0x128] = value (target ID)
//                  case 2 → [this+0x120] = value (playback flag:
//                           sub_45FA80 `test [this+0x120]; setnz`)
//                  case 3 → copy string → [this+0xA0] (target scene name)
//   sub_45FA80 — sub_45F350(path@+0x20, arg, flag, sub_45FAE0) = FMV PLAYBACK
//                (callback sub_45FAE0 = end -> scene transition)
//   sub_4A2010 — startup: sub_4A31B0("GLOBAL") -> sub_4A2950("JINGLE")
//                (called from WinMain vicinity 004196EA)
//
// .RWS FMV descriptor (~1.2 KB) — 8 total: JINGLE, INTRO1/2/3, CREDITS,
// OUTRO1/2/3. Content: Texture Dictionary / WorldGeometry /
// StaticCollisionData / PathFindingData chunks (ALL excluding
// {GUID}.txd/.dff/.col/.pfm — X:\RwStudio\WaveBreaker\Build Output\...) +
// CFMVBehavior instance (FMV name + target scene) + CAttributeHandler.
//
// NOTE: external {GUID} assets and .bik videos NOT available (binkw32.dll present)
// -> FMV skipped and transition to target scene. Repository 1.2 KB descriptors
// separated from levels (>1.8 MB).
//
// Extracted transition graph (from file data):
//   JINGLE  → Jingle  → MainMenu      (binary STARTUP here)
//   INTRO1  → Intro   → Nordic01      (pre-level cut)
//   INTRO2  → Intro2  → Nordic02      (pre-level cut)
//   INTRO3  → Intro3  → MAPMENU
//   CREDITS → Credits → MainMenu
//   OUTRO1  → Outro   → Iduna02
//   OUTRO2  → Outro2  → Iduna03
//   OUTRO3  → Outro3  → Credits
// ============================================================================
#pragma once

#include <cstddef>

/// FMV scene descriptor — CFMVBehavior instance (sub_45F950 apply output).
struct FmvSceneDescriptor {
    char guid[64];      ///< {GUID} — external asset ID (not available)
    char behaviour[64]; ///< instance name == FMV file name ("Jingle"/"Intro"...)
    char target[64];    ///< target scene ("MainMenu"/"Nordic01"/"MAPMENU")
};

/// Resolve .RWS, read, parse per CFMVBehavior.
/// Returns 1 if FMV descriptor (small file + behaviour + target);
/// returns 0 if level/world -> chain end.
/// pRelativePath is in "Streams/XXX.RWS" format (resolved via RwsAssets_Resolve).
int FmvScene_Load(const char *pRelativePath, FmvSceneDescriptor *pOut);

/// Binary-exact video path (sub_45F950 case 0):
///   ".\Video\Win32\<name>.bik"   (base = ".\" — loc_45FA0F)
int FmvScene_VideoPath(const char *pFmvName, char *pOut, size_t outSize);

/// Does FMV file exist? Candidates: (1) CWD ".\Video\Win32\<name>.bik",
/// (2) asset root "<root>/Video/Win32/<name>.bik". No .bik in repo -> 0.
int FmvScene_VideoExists(const char *pFmvName);

/// Write RESOLVED full path of .bik file (same candidate order as
/// FmvScene_VideoExists). Returns 1 if found and pOut gets real path —
/// player (BinkPlayer_Open) uses this; if not found, pOut[0] = 0 and 0 returned.
int FmvScene_VideoResolve(const char *pFmvName, char *pOut, size_t outSize);

/// Target scene -> next descriptor RELATIVE path ("Streams/XXX.RWS").
/// Since all files are uppercase (NORDIC01.RWS, MAINMENU.RWS), UPPERCASE
/// second attempt is made if exact match fails. Returns 1 if file found.
int FmvScene_NextScenePath(const char *pTarget, char *pOut, size_t outSize);
