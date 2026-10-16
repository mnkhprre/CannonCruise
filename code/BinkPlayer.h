// ============================================================================
// CannonCruise - Bink Video Player (BinkPlayer.h)
//
// Binary sources (CC_ida.asm):
//   sub_45F350 — FMV player: BinkSetSoundSystem(BinkOpenDirectSound(hwnd)),
//                BinkSetSoundTrack(1, &track), BinkOpen(path, 0x4000),
//                BinkSetVolume, raster = sub_518C90(640,448,32,0x604)
//                -> BinkWait/BinkDoFrame/BinkNextFrame loop -> BinkClose.
//   sub_45F270 — single frame: BinkDoFrame → BinkCopyToBuffer(..., 0x80000003)
//                -> BinkNextFrame. End: [bink+0xC] == [bink+8].
//
// binkw32.dll v1.6c — exports are __stdcall decorated (_BinkOpen@8, ...).
// BinkCopyToBuffer in this version is 7-ARGUMENT (@28), not modern SDK's
// 10-argument signature:
//   u32 BinkCopyToBuffer(HBINK, void* dest, u32 destpitch,
//                        u32 a4, u32 a5, u32 a6, u32 mode);
// mode = 0x80000003 → 32 bpp X8R8G8B8 (memory order B,G,R,X; X=0).
//
// VERIFIED (Video/Win32/*.bik — 8 files, all 640x448 @ 30 fps):
//   - BINK struct: [0]=width, [4]=height, [8]=total frames, [0xC]=frame no
//   - "while (BinkWait()) busy-spin" → BinkDoFrame → copy → BinkNextFrame
//     = 30 different frames in 30 frames, full 33 ms/frame (BinkWait limits to 30 fps)
//   - Sleep() during wait BREAKS DECODE (frames freeze, same output repeats)
//     -> busy-spin required; only message pumping allowed during spin.
//   - Channel order: byte0=B, byte1=G, byte2=R, byte3=0 (no alpha)
//     — Intro.bik frame extracted as PNG verified visually under both
//       interpretations (B,G,R → green pine trees + red butterflies;
//       swapped interpretation = blue).
// ============================================================================
#pragma once

#include <cstddef>

/// BinkOpen flags — binary sub_45F350: BinkOpen(path, 0x4000).
#define BINKPLAYER_OPEN_FLAGS 0x4000u

/// Load binkw32.dll + resolve exports. 1 = ready, 0 = not found.
/// Multiple candidates tried: CANNONCRUISE_BINKW32 env, exe/CWD search path,
/// <asset-root>binkw32.dll, <asset-root>DLLs/binkw32.dll, .\DLLs\...
int BinkPlayer_Initialize(void);

/// Is loaded? (0 if Initialize not called)
int BinkPlayer_IsAvailable(void);

/// Unload library (at process exit; optional).
void BinkPlayer_Shutdown(void);

/// Open .bik. Returns nullptr on failure. pPath must be full (relative or absolute) path.
void *BinkPlayer_Open(const char *pPath);

/// BINK struct fields (offsets from binary + runtime verified).
int BinkPlayer_Width(const void *pBink);
int BinkPlayer_Height(const void *pBink);
int BinkPlayer_TotalFrames(const void *pBink);
int BinkPlayer_CurrentFrame(const void *pBink);

/// End condition — sub_45F270: [bink+0xC] == [bink+8].
int BinkPlayer_IsFinished(const void *pBink);

/// Wait until frame ready (BinkWait busy-spin — game's loop).
/// pIdle called each iteration (pump messages); return !=0 ends wait early.
/// pIdle may be nullptr.
void BinkPlayer_Wait(void *pBink, int (*pIdle)(void));

/// BinkDoFrame + BinkCopyToBuffer(mode 0x80000003) + BGRX→RGBA conversion.
/// pOut must be at least pitch*Height bytes. 1 = frame ready, 0 = copy failed.
int BinkPlayer_DecodeRGBA(void *pBink, unsigned char *pOut, int pitch);

/// BinkNextFrame — advance to next frame.
void BinkPlayer_Next(void *pBink);

/// BinkClose.
void BinkPlayer_Close(void *pBink);

/// Last error string (from BinkGetError) or empty string. For mirroring/diagnostics.
const char *BinkPlayer_LastError(void);
