// ============================================================================
// CannonCruise - Platform Smoke + RWS Pipeline Entry (main.cpp)
// Phase 4.5: Link verification (-force_load resolves all symbols).
// Phase 4.6n: Render platform frame loop smoke.
// Phase 4.6o+: Optional RWS argument -- end-to-end world pipeline:
//   RWS inventory -> WorldGeometry clump instantiate -> RenderPlatform add
//   -> StaticCollisionData (.col) trimesh shape -> frame loop
//   (clear/Begin -> world render -> Im2D frame -> End). NULL platform
//   headless (device no-op); GL3 build has real window/pixel.
//
// Usage: ./CannonCruise [world.RWS]
//
// Game's real runtime entry on Windows is startup.cpp / WinMain.
// ============================================================================

#include "CameraState.h"
#include "Ocean.h"
#include "PhysicsBackend.h"
#include "RenderPlatform.h"
#include "Rt2dMaestro.h"
#include "Rt2dMaestroStream.h"
#include "MenuBehavior.h"
#include "AfterGameMenuBehavior.h"
#include "RwsAssets.h"   // .RWS asset yol Ã§Ã¶zÃ¼cÃ¼ (merkezi)
#include "FmvScene.h"    // FMV sahne tanÄ±mlayÄ±cÄ±sÄ± (CFMVBehavior) + hedef geÃ§iÅŸi

// FMV story (SCR.mov/SCR1.mov) game state machine -- Phase 4.6s target.
// BOOT -> LOGO_ITE -> LOADING -> MAINMENU -> OPTIONS -> SOUND/CREDITS -> MAPMENU
// -> LEVELINTRO -> GAMEPLAY(NORMAL/CANNON/BINOCULARS) -> PAUSE -> LEVELCOMPLETE.
// Currently BOOT->MAINMENU->GAMEPLAY skeleton works; intermediate states are phased in later.
enum EGameState {
    GAMESTATE_BOOT, GAMESTATE_LOGO_ITE, GAMESTATE_LOADING,
    GAMESTATE_MAINMENU, GAMESTATE_OPTIONS, GAMESTATE_SOUND,
    GAMESTATE_CREDITS, GAMESTATE_MAPMENU, GAMESTATE_LEVELINTRO,
    GAMESTATE_GAMEPLAY, GAMESTATE_PAUSE, GAMESTATE_LEVELCOMPLETE,
    GAMESTATE_OUTRO, GAMESTATE_QUIT
};
static EGameState s_gameState = GAMESTATE_BOOT;
#include "RwStudioStream.h"
#include "ShowText.h"
#include "StaticCollisionData.h"
#include "WorldGeometry.h"
#include "WorldSpawn.h"
#include "LocalizationManager.h"
#include "BinkPlayer.h"  // binkw32 oynatici - .bik oynatimi (sub_45F350/45F270)
#include "rwcore.h"
#include "AudioManager.h"
#include "SpeakManager.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

namespace {

// Is FMV chain terminal target a PHASE? MainMenu/MAPMENU are already handled
// by menu/map phases; others (Nordic01/Nordic02/Iduna02/Iduna03...)
// are LEVELS -> load world to that target at chain end (binary iMsgChangeScene
// counterpart). Case-insensitive (target "MainMenu", file MAINMENU).
int IsFmvPhaseTarget(const char *pTarget) {
    if (pTarget == nullptr) {
        return 0;
    }
    char up[64] = {0};
    for (size_t i = 0; i + 1 < sizeof(up) && pTarget[i] != 0; ++i) {
        const char c = pTarget[i];
        up[i] = (c >= 'a' && c <= 'z') ? (char)(c - 'a' + 'A') : c;
    }
    return (std::strcmp(up, "MAINMENU") == 0 ||
            std::strcmp(up, "MAPMENU") == 0)
               ? 1
               : 0;
}

/// RWS inventory + WorldGeometry clump + .col shape -- single world pipeline.
struct RwsPipeline {
    void *pClump;             ///< rw::Clump* (RenderPlatform'a eklendi)
    void *pCollisionShape;    ///< PhysicsShape* (btBvhTriangleMeshShape)
    RwUInt32 assetCount;
    RwUInt32 worldGeomBytes;
    RwUInt32 collisionBytes;
    StaticCollisionMesh collisionMesh;
};

RwBool RwsPipeline_Load(const char *pPath, RwsPipeline *pOut) {
    std::memset(pOut, 0, sizeof(*pOut));

    RwStudioFile *pFile = RwStudioFileOpen(pPath);
    if (!pFile) {
        std::fprintf(stderr, "RWS open failed: %s\n", pPath);
        return FALSE;
    }

    RwStudioAsset asset;
    while (RwStudioFileNextAsset(pFile, &asset)) {
        ++pOut->assetCount;

        if (std::strcmp(asset.typeName, "WorldGeometry") == 0 &&
            asset.resourceLength > 0) {
            unsigned char *pBuf = static_cast<unsigned char *>(
                std::malloc(asset.resourceLength));
            if (!pBuf) {
                continue;
            }
            if (RwStudioAssetReadResource(pFile, &asset, pBuf,
                                          asset.resourceLength) ==
                asset.resourceLength) {
                WorldGeometryInfo info;
                if (WorldGeometry_Inspect(pBuf, asset.resourceLength,
                                          &info) == TRUE) {
                    pOut->pClump =
                        WorldGeometry_CreateClump(pBuf, asset.resourceLength);
                    pOut->worldGeomBytes = asset.resourceLength;
                    std::printf(
                        "WorldGeometry: %u+%u atomics, %u verts, %u tris "
                        "(%u bytes) -> clump %s\n",
                        info.atomicCount, info.atomicGroup2Count,
                        info.totalVertices + info.group2Vertices,
                        info.totalTriangles + info.group2Triangles,
                        asset.resourceLength, pOut->pClump ? "OK" : "FAIL");
                }
            }
            std::free(pBuf);
        } else if (std::strcmp(asset.typeName, "StaticCollisionData") == 0 &&
                   asset.resourceLength > 0) {
            unsigned char *pBuf = static_cast<unsigned char *>(
                std::malloc(asset.resourceLength));
            if (!pBuf) {
                continue;
            }
            if (RwStudioAssetReadResource(pFile, &asset, pBuf,
                                          asset.resourceLength) ==
                asset.resourceLength) {
                if (StaticCollisionData_Decode(pBuf, asset.resourceLength,
                                               &pOut->collisionMesh) ==
                    TRUE) {
                    pOut->pCollisionShape = StaticCollisionData_CreateShape(
                        &pOut->collisionMesh);
                    pOut->collisionBytes = asset.resourceLength;
                    std::printf(
                        "StaticCollisionData: %u verts, %u tris "
                        "(%u bytes) -> shape %s\n",
                        pOut->collisionMesh.vertexCount,
                        pOut->collisionMesh.triangleCount,
                        asset.resourceLength,
                        pOut->pCollisionShape ? "OK" : "FAIL");
                }
            }
            std::free(pBuf); // shape mesh is copied -- payload freed
        }
    }
    RwStudioFileClose(pFile);
    return TRUE;
}

void RwsPipeline_Unload(RwsPipeline *pPipe) {
    if (pPipe->pClump) {
        WorldGeometry_DestroyClump(pPipe->pClump);
        pPipe->pClump = nullptr;
    }
    if (pPipe->pCollisionShape) {
        PhysicsShape_Destroy(pPipe->pCollisionShape);
        pPipe->pCollisionShape = nullptr;
    }
}

// Phase 4.6p -- World camera pose from RWS 0x0704 CWbCameraBehavior registrations
// (VIKING01 obj[264]: id0 pos, id6 dir, id4 FOV). First props CWbCameraBehavior is applied
// (binary: CCameraState singleton).
RwBool RwsCamera_LoadPose(const RwChar *pPath, RwReal posOut[3],
                          RwReal lookOut[3]) {
    RwStudioFile *pFile = RwStudioFileOpen(pPath);
    if (!pFile) {
        return FALSE;
    }
    CCameraState state;
    std::memset(&state, 0, sizeof(state));
    RwBool bPos = FALSE;
    RwBool bDir = FALSE;
    RwStudioAsset asset;
    while (RwStudioFileNextAsset(pFile, &asset) == TRUE) {
        const RwStudioObject *pObj = RwStudioFileGetObject(pFile);
        if (!pObj) {
            continue;
        }
        for (RwUInt32 i = 0; i < pObj->behaviourCount; ++i) {
            const RwStudioBehaviour *pBeh = &pObj->behaviours[i];
            if (std::strcmp(pBeh->className, "CWbCameraBehavior") != 0 ||
                pBeh->propCount == 0) {
                continue;
            }
            for (RwUInt32 j = 0; j < pBeh->propCount; ++j) {
                const RwStudioProperty &prop = pBeh->props[j];
                if (prop.size == 20) {
                    CCameraStateApplyVec3(&state, prop.id, prop.value);
                    if (prop.id == 0x0) {
                        bPos = TRUE;
                    } else if (prop.id == 0x6) {
                        bDir = TRUE;
                    }
                } else if (prop.size == 12) {
                    RwUInt32 raw = 0;
                    std::memcpy(&raw, prop.value, 4);
                    state.ApplyAttribute(prop.id, raw);
                }
            }
            if (bPos == TRUE && bDir == TRUE) {
                break;
            }
        }
        if (bPos == TRUE && bDir == TRUE) {
            break;
        }
    }
    RwStudioFileClose(pFile);
    if (bPos != TRUE || bDir != TRUE) {
        return FALSE;
    }
    posOut[0] = state.m_pos[0];
    posOut[1] = state.m_pos[1];
    posOut[2] = state.m_pos[2];
    // id6 (+0x24) ABSOLUTE look target (lookAt point) -- NOT a direction vector:
    // VIKING01 pos=(0.35,7,0), target=(0.85,0.85,0.85) ~= looking down at world center.
    // pos+dir would look up-to-sky instead.
    lookOut[0] = state.m_dir[0];
    lookOut[1] = state.m_dir[1];
    lookOut[2] = state.m_dir[2];
    return TRUE;
}

// --------------------------------------------------------------------------
// FMV playback -- "no-op" callback running inside BinkPlayer_Wait busy-spin.
// As verified in BinkPlayer.h, Sleep BREAKS DECODE (frames freeze) ->
// only message pumping during wait.
// Return !=0 -> stop playback (Esc/close).
//
// s_fmvArmed: disables skip on first frame to swallow keypresses leaking
// from previous phase (same logic as s_prevEsc on title card).
// --------------------------------------------------------------------------
RwBool s_fmvArmed = FALSE;
RwBool s_fmvSkip = FALSE;

int FmvSpinIdle(void) {
    RwPlatformPollEvents();
    if (RwPlatformShouldClose() == TRUE) {
        s_fmvSkip = TRUE;
        return 1;
    }
    if (s_fmvArmed == TRUE &&
        (RwPlatformKeyDown(RW_KEY_ESCAPE) || RwPlatformKeyDown('Q') ||
         RwPlatformKeyDown(' '))) {
        s_fmvSkip = TRUE;
        return 1;
    }
    return 0;
}

} // namespace

int main(int argc, char **argv) {
    // Diagnostics: stdout UNBUFFERED -- stdio buffer flushed only on exit;
    // otherwise last output was lost on crash (e.g. 0xC0000005); needed to
    // see which step we were at.
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    std::printf("CannonCruise: link verification OK\n");

    // Engine boot -- test pattern (Init/Open/Start; teardown Stop/Close/Term).
    if (RwEngineInit(0, 0, 0) != TRUE || RwEngineOpen() != TRUE ||
        RwEngineStart() != TRUE) {
        std::fprintf(stderr, "CannonCruise: engine init failed\n");
        return 2;
    }

    // Localization Manager (sub_4918C0 / sub_4923A0: lang_<lang>.txt + win_<lang>.txd)
    // Screen resolution - FMV quad also uses this (.bik 640x448 ->
    // fits full screen; original game draws 640x448 raster to screen
    // with same layout).
    const RwInt32 kScreenW = 1280;
    const RwInt32 kScreenH = 720;
    if (RenderPlatform_Initialize(kScreenW, kScreenH) != TRUE) {
        std::fprintf(stderr, "CannonCruise: render platform init failed\n");
        RwEngineStop();
        RwEngineClose();
        RwEngineTerm();
        return 2;
    }

    // Localization Manager (sub_4918C0 / sub_4923A0: lang_<dil>.txt + win_<dil>.txd)
    CLocalizationManager locMgr;
    locMgr.LoadDefinitions();
    locMgr.SetLanguage(locMgr.GetCurrentLanguage());

    // Audio & Speech subsystems (Phase 4.6z + Turkish dubbing)
    CAudioManager audioMgr;
    CSpeakManager speakMgr;
    speakMgr.SetLanguage("Tyr");

    // World: if no argument, like game flow MAINMENU.RWS (binary:
    // intro -> menu). Note: menu RWS resources are NOT embedded (resLen=0,
    // resource path X:\RwStudio\...\{GUID}.dff -- build-output reference,
    // we don't have it) -> empty clump falls to first working world (VIKING01).
    const char *pWorldPath = (argc > 1) ? argv[1] : nullptr;
    // 4.6r -- boot flow: argumentless run uses entry chain (INTRO->menu)
    // overlaying the world; if world arg given, goes directly to
    // interactive world (entry chain skipped).
    const RwBool bBootFlow = (argc <= 1) ? TRUE : FALSE;
    // CANNONCRUISE_SCREENSHOT=<path.png> -- save first overlay frame as PNG
    // (intro frame; CANNONCRUISE_INTRO_FRAMES=0 for menu frame). One-time only.
    const char *pShotPath = std::getenv("CANNONCRUISE_SCREENSHOT");
    // CANNONCRUISE_SCREENSHOT_MAP=<path.png> -- MAPMENU frame.
    const char *pMapShotPath = std::getenv("CANNONCRUISE_SCREENSHOT_MAP");
    // World path updated by MAPMENU selection (setup block uses it).
    // 512: RwsAssets_Resolve returns 512-length paths -- no truncation.
    char mapLevelPath[512] = {0};

    RwsPipeline pipeline = {};
    RwBool bWorldLoaded = FALSE;
    // Resolved path lives in permanent buffer -- pWorldPath pointer must stay
    // valid throughout loop.
    static char resolvedWorldPath[512] = {0};
    if (pWorldPath != nullptr) {
        // Load world textures first (materials resolved at stream time).
        RenderPlatform_LoadWorldDictionary(pWorldPath);
        bWorldLoaded = RwsPipeline_Load(pWorldPath, &pipeline);
    } else {
        // RwsAssets_Resolve resolves relative depth (build layout) + env override
        // centrally -- no per-call prefix list needed anymore.
        static const char *kBootWorlds[] = {
            "Streams/MAINMENU.RWS", "Streams/VIKING01.RWS",
        };
        for (RwUInt32 i = 0; i < 2 && bWorldLoaded == FALSE; ++i) {
            if (!RwsAssets_Resolve(kBootWorlds[i], resolvedWorldPath,
                                   sizeof(resolvedWorldPath))) {
                continue; // dosya bu yerleÅŸimde yok
            }
            RenderPlatform_LoadWorldDictionary(resolvedWorldPath);
            if (RwsPipeline_Load(resolvedWorldPath, &pipeline) == TRUE &&
                pipeline.pClump != nullptr) {
                pWorldPath = resolvedWorldPath;
                bWorldLoaded = TRUE;
                break;
            }
            RwsPipeline_Unload(&pipeline); // no clump (menu: external dff)
            RenderPlatform_ClearWorldDictionary();
        }
    }
    // Note: world setup (AddClump/spawn/camera/ocean) moved AFTER boot-flow
    // -- MAPMENU selection set up with single load (reload + RemoveClump +
    // double-spawn not needed). (4.6r-4 MAPMENU)
    COcean *pOcean = nullptr; // quit-cleanup notification (setup below)

    // 4.6r -- MAINMENU menu phase (entry chain: GLOBAL->INTRO skip->MAINMENU->
    // world). INTRO1-3/FMV {GUID} files not available -> skip; MAINMENU
    // external {GUID}.dff missing (empty clump) but embedded Menu.anm stream
    // rendered via Rt2dMaestro + command sequences walked.
    // CANNONCRUISE_MENU_FRAMES: frame cap (0=skip menu, -1=unlimited).
    RwBool bQuitApp = FALSE;
    if (bBootFlow == TRUE) {
        s_gameState = GAMESTATE_LOGO_ITE;
        // 4.6r-6 -- FMV scene phase (binary sub_4A2010: GLOBAL -> JINGLE; JINGLE
        // target MainMenu -> menu phase). Old assumption "INTRO1->2->3"
        // WRONG: INTRO1/2 not referenced by name in binary (only INTRO3
        // exists); INTRO1->Nordic01, INTRO2->Nordic02 = pre-level cuts,
        // INTRO3->MAPMENU. Full transition graph extracted in FmvScene.h.
        //
        // Every .RWS FMV descriptor is CFMVBehavior (sub_45F6D0 Register,
        // sub_45F950 apply) -> FMV name + TARGET SCENE (FmvScene module).
        // Video path ".\Video\Win32\<name>.bik" (case0 / loc_45FA0F); .bik and
        // external {GUID} txd/dff/col/pfm not available -> FMV skip and GO TO
        // TARGET SCENE -- Nordic connection is chain target following.
        int introCap = 180; // ~3 sn @60fps; CANNONCRUISE_INTRO_FRAMES
        const char *introEnv = std::getenv("CANNONCRUISE_INTRO_FRAMES");
        if (introEnv != nullptr) {
            introCap = std::atoi(introEnv);
        }
        // CANNONCRUISE_FMV: starting descriptor (default JINGLE =
        // binary startup). E.g. Streams/INTRO1.RWS -> target Nordic01.
        char fmvRel[512] = {0};
        const char *fmvEnv = std::getenv("CANNONCRUISE_FMV");
        std::snprintf(fmvRel, sizeof(fmvRel), "%s",
                      (fmvEnv != nullptr) ? fmvEnv : "Streams/JINGLE.RWS");
        // Chain terminal target + resolved next path (level start).
        char fmvTerminalTarget[64] = {0};
        char fmvNextRel[512] = {0};
        for (RwUInt32 ii = 0; ii < 8 && bQuitApp == FALSE; ++ii) {
            // FMV descriptor: CFMVBehavior -> FMV name + target scene
            FmvSceneDescriptor fmv;
            if (FmvScene_Load(fmvRel, &fmv) != 1) {
                std::printf("FMV: %s â†’ tanÄ±mlayÄ±cÄ± deÄŸil (seviye/dÃ¼nya) â†’ "
                            "zincir sonu\n",
                            fmvRel);
                break;
            }
            char video[512] = {0};
            const int hasVideo =
                FmvScene_VideoPath(fmv.behaviour, video, sizeof(video));
            // Resolve .bik real path (CWD + asset root candidates) - used at
            // playback time; FmvScene_VideoExists scans the same order.
            char videoFull[512] = {0};
            const int videoOk =
                FmvScene_VideoResolve(fmv.behaviour, videoFull,
                                      sizeof(videoFull));
            char nextRel[512] = {0};
            const int hasNext =
                FmvScene_NextScenePath(fmv.target, nextRel, sizeof(nextRel));
            // target connection visible here: target -> Streams/<TARGET>.RWS
            std::printf("FMV[%u]: %s fmv='%s' target='%s'%s%s\n",
                        (unsigned)ii, fmv.guid, fmv.behaviour, fmv.target,
                        (hasNext == 1) ? " â†’ " : "",
                        (hasNext == 1) ? nextRel : "(sahne yok)");
            if (hasVideo == 1) {
                std::printf("  video: %s (%s)\n", video,
                            (videoOk == 1) ? "oynat" : "yok â†’ skip");
            }
            // title card loop
            // ------------------------------------------------------------------
            // FMV VIDEO PLAYBACK (binkw32) - binary sub_45F350 / sub_45F270.
            // If .bik exists, REAL video played instead of title card; after
            // frame ends (or skipped via Esc) chain continues to TARGET SCENE
            // (JINGLE->MainMenu, INTRO1->Nordic01, OUTRO3->Credits).
            //
            // Loop mirrors binary: while (BinkWait()) busy-spin ->
            // BinkDoFrame -> BinkCopyToBuffer(mode 0x80000003) -> BinkNextFrame.
            // BinkWait limits frame rate to 30 fps; Sleep NOT added during
            // wait (breaks decode) - only messages pumped.
            // ------------------------------------------------------------------
            RwBool bFmvPlayed = FALSE;
            if (videoOk == 1) {
                void *pBink = BinkPlayer_Open(videoFull);
                if (pBink != nullptr) {
                    const int vw = BinkPlayer_Width(pBink);
                    const int vh = BinkPlayer_Height(pBink);
                    std::printf("  oynat: %s (%dx%d, %d kare) %s\n", videoFull,
                                vw, vh, BinkPlayer_TotalFrames(pBink),
                                BinkPlayer_LastError());
                    std::vector<unsigned char> fmvPixels((size_t)vw *
                                                         (size_t)vh * 4u);
                    const int fmvPitch = vw * 4;
                    int fmvShown = 0;
                    s_fmvArmed = FALSE; // skip disabled until first frame
                    s_fmvSkip = FALSE;
                    for (;;) {
                        // wait until frame ready (busy-spin + message pumping;
                        // FmvSpinIdle breaks on close/Esc)
                        BinkPlayer_Wait(pBink, FmvSpinIdle);
                        if (s_fmvSkip == TRUE) {
                            break;
                        }
                        if (BinkPlayer_IsFinished(pBink) == 1) {
                            break;
                        }
                        if (BinkPlayer_DecodeRGBA(pBink, fmvPixels.data(),
                                                  fmvPitch) != 1) {
                            std::printf("  video kare hatasi (%s)\n",
                                        BinkPlayer_LastError());
                            break;
                        }
                        s_fmvArmed = TRUE;
                        if (RenderPlatform_UploadTexture(
                                "FMV", vw, vh, fmvPixels.data(),
                                fmvPitch) != TRUE) {
                            break;
                        }
                        if (RenderPlatform_BeginFrame(nullptr) != TRUE) {
                            break;
                        }
                        // video overlay - ZTEST off, fullscreen quad
                        // (640x448 .bik matches screen resolution; original
                        // game draws 640x448 raster to screen)
                        RenderPlatform_BeginOverlay2D();
                        RenderPlatform_DrawTexturedQuad(
                            0.0f, 0.0f, (RwReal)kScreenW, (RwReal)kScreenH,
                            "FMV");
                        RenderPlatform_EndOverlay2D();
                        if (pShotPath != nullptr && fmvShown == 0) {
                            RenderPlatform_RequestScreenshot(pShotPath);
                        }
                        RenderPlatform_EndFrame();
                        bFmvPlayed = TRUE;
                        ++fmvShown;
                        BinkPlayer_Next(pBink);
                    }
                    BinkPlayer_Close(pBink);
                    std::printf("  video bitti: %d kare%s oynatildi\n", fmvShown,
                                (s_fmvSkip == TRUE) ? " (atlandi)" : "");
                    RenderPlatform_ClearTextures(); // release FMV texture
                } else {
                    std::printf("  video acilamadi: %s (%s)\n", videoFull,
                                BinkPlayer_LastError());
                }
            }
            if (bFmvPlayed == FALSE) {

            static RwBool s_prevEsc = FALSE;
            // Phase entry: consume held Esc (skip leakage from previous phase)
            RwPlatformPollEvents();
            s_prevEsc = (RwBool)(RwPlatformKeyDown(RW_KEY_ESCAPE) ||
                                 RwPlatformKeyDown('Q') ||
                                 RwPlatformKeyDown(' '));
            for (int fr = 0;; ++fr) {
                if (introCap >= 0 && fr >= introCap) {
                    break;
                }
                RwPlatformPollEvents();
                if (RwPlatformShouldClose() == TRUE) {
                    bQuitApp = TRUE;
                    break;
                }
                const RwBool esc = RwPlatformKeyDown(RW_KEY_ESCAPE) ||
                                   RwPlatformKeyDown('Q') ||
                                   RwPlatformKeyDown(' ');
                if (esc == TRUE && s_prevEsc == FALSE) {
                    s_prevEsc = esc;
                    break;
                }
                s_prevEsc = esc;
                if (RenderPlatform_BeginFrame(nullptr) != TRUE) {
                    break;
                }
                if (bWorldLoaded == TRUE && pipeline.pClump != nullptr) {
                    RenderPlatform_RenderScene();
                }
                // intro card -- Im2D overlay (ZTEST off; 3D scene depth written)
                RenderPlatform_BeginOverlay2D();
                char l1[64], l2[64], l3[96];
                std::snprintf(l1, sizeof(l1), "CANNONCRUISE");
                std::snprintf(l2, sizeof(l2), "FMV SCENE %u", (unsigned)ii + 1);
                std::snprintf(l3, sizeof(l3),
                              "FMV SKIPPED (external {GUID} assets missing)");
                ShowText_DrawBitmapText(l1, 40.0f, 200.0f, 5, 220, 230, 255,
                                        255);
                ShowText_DrawBitmapText(l2, 40.0f, 280.0f, 4, 200, 210, 230,
                                        255);
                ShowText_DrawBitmapText(fmv.behaviour, 40.0f, 350.0f, 3, 180,
                                        190, 200, 255);
                ShowText_DrawBitmapText(fmv.target, 40.0f, 390.0f, 3, 180, 190,
                                        200, 255);
                ShowText_DrawBitmapText(l3, 40.0f, 660.0f, 2, 140, 150, 160,
                                        255);
                RenderPlatform_EndOverlay2D();
                if (pShotPath != nullptr && fr == 1) {
                    RenderPlatform_RequestScreenshot(pShotPath);
                }
                RenderPlatform_EndFrame();
            }
            // advance chain: is target scene a NEXT FMV descriptor?
            // (JINGLE->MainMenu, INTRO1->Nordic01, OUTRO3->Credits chain)
            } // if (bFmvPlayed == FALSE) - title card

            std::snprintf(fmvTerminalTarget, sizeof(fmvTerminalTarget), "%s",
                          fmv.target);
            fmvNextRel[0] = 0;
            if (hasNext != 1) {
                break; // target not resolved -> chain end
            }
            std::snprintf(fmvNextRel, sizeof(fmvNextRel), "%s", nextRel);
            std::snprintf(fmvRel, sizeof(fmvRel), "%s", nextRel);
        }
        if (bQuitApp == TRUE) {
            delete pOcean;
            pOcean = nullptr;
            RwsPipeline_Unload(&pipeline);
            RenderPlatform_Terminate();
            RwEngineStop();
            RwEngineClose();
            RwEngineTerm();
            return 0;
        }

        // Chain terminal -> TARGET LEVEL (Nordic01/Nordic02/Iduna02...):
        // bind world to that level -- intro's Nordic connection is REALIZED.
        // Phase targets (MainMenu/MAPMENU) already handled by menu/map phases
        // below -> untouched. Loading pattern same as MAPMENU
        // (preserve predecessor for fallback).
        if (bQuitApp == FALSE && fmvNextRel[0] != 0 &&
            IsFmvPhaseTarget(fmvTerminalTarget) == 0) {
            char prevLevel[512] = {0};
            if (pWorldPath != nullptr) {
                std::snprintf(prevLevel, sizeof(prevLevel), "%s", pWorldPath);
            }
            RwsPipeline_Unload(&pipeline);
            bWorldLoaded = FALSE;
            RenderPlatform_LoadWorldDictionary(fmvNextRel);
            if (RwsPipeline_Load(fmvNextRel, &pipeline) == TRUE &&
                pipeline.pClump != nullptr) {
                std::snprintf(mapLevelPath, sizeof(mapLevelPath), "%s",
                              fmvNextRel);
                pWorldPath = mapLevelPath;
                bWorldLoaded = TRUE;
                std::printf("FMV: hedef seviye '%s' yÃ¼klendi (%s)\n",
                            fmvTerminalTarget, fmvNextRel);
            } else {
                RwsPipeline_Unload(&pipeline);
                RenderPlatform_ClearWorldDictionary();
                if (prevLevel[0] != 0) {
                    RenderPlatform_LoadWorldDictionary(prevLevel);
                    if (RwsPipeline_Load(prevLevel, &pipeline) == TRUE &&
                        pipeline.pClump != nullptr) {
                        bWorldLoaded = TRUE;
                    }
                }
                std::printf("FMV: hedef seviye yÃ¼klenemedi ('%s')\n",
                            fmvTerminalTarget);
            }
        }

        int menuCap = -1; // -1 = unlimited (wait Enter/Esc); test env override
        const char *menuEnv = std::getenv("CANNONCRUISE_MENU_FRAMES");
        if (menuEnv != nullptr) {
            menuCap = std::atoi(menuEnv);
        }
        RwBool bMenuBackedOut = FALSE; // MAIN-Esc exit (map not opened)
        if (menuCap != 0) {
            // Relative depth centrally resolved (RwsAssets_Resolve) -> single candidate;
            // previous 3-prefix loop removes redundant repetition. Loop
            // structure kept (continue agreement if missing).
            char menuPath[512] = {0};
            RwsAssets_Resolve("Streams/MAINMENU.RWS", menuPath,
                              sizeof(menuPath));
            const char *kMenuFiles[] = {menuPath};
            for (RwUInt32 mi = 0; mi < 1; ++mi) {
                // Platform: single-path maestro load (0x0716-walk deduplicated).
                Rt2dMaestro *pMaestro =
                    Rt2dMaestro_LoadFromRwsFile(kMenuFiles[mi]);
                if (pMaestro == nullptr) {
                    continue;
                }
                // Storyboard: menu state lives in CMenuBehavior (MAINMENU/
                // OPTIONS/SOUND/PAUSE share same Menu.anm).
                CMenuBehavior menu;
                menu.AttachMaestro(pMaestro);
                pMaestro = menu.GetMaestro();
                // embedded menu textures (TexDictionary -> GL; Menu_074 ground,
                // Menu_076 logo) -- inside RWS, not external file.
                menu.LoadTextures(kMenuFiles[mi]);
                s_gameState = GAMESTATE_MAINMENU;
                audioMgr.PlayStream("MenuMus", TRUE);
                std::printf("Menu: nodes=%u commands=%d canvas=%.0fx%.0f\n",
                            pMaestro->info.nodeCount, pMaestro->commandCount,
                            (double)pMaestro->info.canvasWidth,
                            (double)pMaestro->info.canvasHeight);
                for (RwInt32 ci = 0; ci < pMaestro->commandCount; ++ci) {
                    std::printf("  [%d] %s\n", ci,
                                pMaestro->info.nodes[pMaestro->commands[ci]]
                                    .name);
                }
                // menu loop: select (W/S/up/down), confirm (Enter/Space), back
                // (Esc/Q) -- auto-confirm at frame cap (NewSave flow).
                int menuFrame = 0;
                static RwBool s_prevDown = FALSE, s_prevUp = FALSE,
                              s_prevEnter = FALSE, s_prevEsc = FALSE,
                              s_prevLeft = FALSE, s_prevRight = FALSE;
                // Phase entry: consume held keys ONCE (Enter/Space leaking
                // from intro shouldn't confirm menu unseen).
                RwPlatformPollEvents();
                s_prevDown = RwPlatformKeyDown('S') ||
                               RwPlatformKeyDown(RW_KEY_DOWN);
                s_prevUp = RwPlatformKeyDown('W') ||
                           RwPlatformKeyDown(RW_KEY_UP);
                s_prevLeft = RwPlatformKeyDown('A') ||
                             RwPlatformKeyDown(RW_KEY_LEFT);
                s_prevRight = RwPlatformKeyDown('D') ||
                              RwPlatformKeyDown(RW_KEY_RIGHT);
                s_prevEnter = RwPlatformKeyDown(' ') ||
                              RwPlatformKeyDown(RW_KEY_ENTER);
                s_prevEsc = RwPlatformKeyDown(RW_KEY_ESCAPE) ||
                            RwPlatformKeyDown('Q');
                for (;;) {
                    ++menuFrame;
                    if (menuCap > 0 && menuFrame > menuCap) {
                        break; // auto-confirm -> go to world
                    }
                    RwPlatformPollEvents();
                    if (RwPlatformShouldClose() == TRUE) {
                        bQuitApp = TRUE;
                        break;
                    }
                    const RwBool down =
                        RwPlatformKeyDown('S') ||
                        RwPlatformKeyDown(RW_KEY_DOWN);
                    const RwBool up =
                        RwPlatformKeyDown('W') ||
                        RwPlatformKeyDown(RW_KEY_UP);
                    const RwBool left =
                        RwPlatformKeyDown('A') ||
                        RwPlatformKeyDown(RW_KEY_LEFT);
                    const RwBool right =
                        RwPlatformKeyDown('D') ||
                        RwPlatformKeyDown(RW_KEY_RIGHT);
                    const RwBool enter =
                        RwPlatformKeyDown(' ') ||
                        RwPlatformKeyDown(RW_KEY_ENTER);
                    const RwBool esc = RwPlatformKeyDown(RW_KEY_ESCAPE) ||
                                       RwPlatformKeyDown('Q');
                    if (down == TRUE && s_prevDown == FALSE) {
                        menu.MoveNext();
                        audioMgr.PlaySound("Audio_MenuSelect");
                    }
                    if (up == TRUE && s_prevUp == FALSE) {
                        menu.MovePrev();
                        audioMgr.PlaySound("Audio_MenuSelect");
                    }
                    if (left == TRUE && s_prevLeft == FALSE) {
                        menu.AdjustLeft();
                    }
                    if (right == TRUE && s_prevRight == FALSE) {
                        menu.AdjustRight();
                    }
                    if (esc == TRUE && s_prevEsc == FALSE) {
                        s_prevDown = down; s_prevUp = up; s_prevEnter = enter;
                        s_prevEsc = esc;
                        s_prevLeft = left; s_prevRight = right;
                        if (menu.Back() == TRUE) {
                            bMenuBackedOut = TRUE;
                            break; // MAIN -> exit loop
                        }
                    }
                    if (enter == TRUE && s_prevEnter == FALSE) {
                        s_prevDown = down; s_prevUp = up; s_prevEnter = enter;
                        s_prevEsc = esc;
                        s_prevLeft = left; s_prevRight = right;
                        if (menu.Confirm() == TRUE) {
                            break; // terminal action (NEW/LOAD/QUIT/CREDITS)
                        }
                    }
                    s_prevDown = down; s_prevUp = up; s_prevEnter = enter;
                    s_prevEsc = esc;
                    s_prevLeft = left; s_prevRight = right;
                    menu.Update(1.0f / 60.0f);
                    if (RenderPlatform_BeginFrame(nullptr) != TRUE) {
                        break;
                    }
                    if (bWorldLoaded == TRUE && pipeline.pClump != nullptr) {
                        RenderPlatform_RenderScene();
                    }
                    menu.Render();
                    if (pShotPath != nullptr && menuFrame == 2) {
                        RenderPlatform_RequestScreenshot(pShotPath);
                    }
                    RenderPlatform_EndFrame();
                }
                // maestro owned by menu (dtor destroys it)
                pMaestro = nullptr;
                // storyboard terminal action: NEW/LOAD -> continue to world,
                // QUIT -> exit, CREDITS -> log (CREDITS.RWS phase pending)
                const CMenuBehavior::EMenuAction act =
                    menu.TakePendingAction();
                if (act == CMenuBehavior::MENUACTION_QUITGAME) {
                    std::printf("Menu: action QUITGAME\n");
                    bQuitApp = TRUE;
                } else if (act == CMenuBehavior::MENUACTION_CREDITS) {
                    // Load Credits.RWS; afterwards return to MainMenu.
                    char credPath[512] = {0};
                    if (FmvScene_NextScenePath("Credits", credPath, sizeof(credPath)) == 1) {
                        std::printf("Menu: action CREDITS → loading %s\n", credPath);
                        // Load RWS file; textures/visuals loaded via
                        // MenuTextures_LoadFromRws then return to MainMenu.
                        RwsAssets_Resolve("Streams/CREDITS.RWS", credPath,
                                          sizeof(credPath));
                        // Set game state to return to MainMenu.
                        s_gameState = GAMESTATE_CREDITS;
                    } else {
                        std::printf("Menu: action CREDITS (no path found)\n");
                    }
                } else if (act == CMenuBehavior::MENUACTION_NEWGAME) {
                    std::printf("Menu: action NEWGAME\n");
                } else if (act == CMenuBehavior::MENUACTION_LOADGAME) {
                    std::printf("Menu: action LOADGAME\n");
                }
                RenderPlatform_ClearTextures(); // free menu textures
                // 4.6r-4 -- MAPMENU phase (CAG_Menu.anm + level selection).
                // After NEW/LOAD (also at auto-cap); skipped on Esc-backout.
                const RwBool wantMap =
                    (RwBool)(act == CMenuBehavior::MENUACTION_NEWGAME ||
                             act == CMenuBehavior::MENUACTION_LOADGAME ||
                             (act == CMenuBehavior::MENUACTION_NONE &&
                              bMenuBackedOut == FALSE && menuCap > 0));
                if (wantMap == TRUE && bQuitApp == FALSE) {
                    s_gameState = GAMESTATE_MAPMENU;
                    int mapCap = -1;
                    const char *mapEnv =
                        std::getenv("CANNONCRUISE_MAPMENU_FRAMES");
                    if (mapEnv != nullptr) {
                        mapCap = std::atoi(mapEnv);
                    }
                    // Test: automatic level selection (non-interactive reload path).
                    int mapAutoSel = -1;
                    const char *mapSelEnv =
                        std::getenv("CANNONCRUISE_MAPMENU_SELECT");
                    if (mapSelEnv != nullptr) {
                        mapAutoSel = std::atoi(mapSelEnv);
                    }
                    static char mapPath[512] = {0};
                    RwsAssets_Resolve("Streams/MAPMENU.RWS", mapPath,
                                      sizeof(mapPath));
                    const char *kMapFiles[] = {mapPath};
                    for (RwUInt32 gi = 0; gi < 1; ++gi) {
                        // Platform: single-path maestro load.
                        Rt2dMaestro *pMapMaestro =
                            Rt2dMaestro_LoadFromRwsFile(kMapFiles[gi]);
                        if (pMapMaestro == nullptr) {
                            continue;
                        }
                        CAfterGameMenuBehavior mapmenu;
                        mapmenu.AttachMaestro(pMapMaestro);
                        pMapMaestro = nullptr;
                        MenuTextures_LoadFromRws(kMapFiles[gi]);
                        std::printf("MapMenu: levels=%d\n",
                                    (int)mapmenu.GetLevelCount());
                        for (RwInt32 li = 0;
                             li < mapmenu.GetLevelCount(); ++li) {
                            std::printf("  [%d] %s (%s)\n", (int)li,
                                        mapmenu.GetLevelDisplay(li),
                                        mapmenu.GetLevelFile(li));
                        }
                        int mapFrame = 0;
                        static RwBool s_prevMapDown = FALSE,
                                      s_prevMapUp = FALSE,
                                      s_prevMapEnter = FALSE,
                                      s_prevMapEsc = FALSE;
                        // Phase entry: consume held keys ONCE (Enter from menu
                        // confirm shouldn't skip past map unseen -- "direct viking
                        // 1" bug).
                        RwPlatformPollEvents();
                        s_prevMapDown = RwPlatformKeyDown('S') ||
                                        RwPlatformKeyDown(RW_KEY_DOWN);
                        s_prevMapUp = RwPlatformKeyDown('W') ||
                                      RwPlatformKeyDown(RW_KEY_UP);
                        s_prevMapEnter = RwPlatformKeyDown(' ') ||
                                         RwPlatformKeyDown(RW_KEY_ENTER);
                        s_prevMapEsc = RwPlatformKeyDown(RW_KEY_ESCAPE) ||
                                       RwPlatformKeyDown('Q');
                        for (;;) {
                            ++mapFrame;
                            if (mapAutoSel >= 0 &&
                                mapAutoSel < mapmenu.GetLevelCount()) {
                                // otomatik seÃ§im (seÃ§im 0'dan baÅŸlar)
                                for (int k = 0; k < mapAutoSel; ++k) {
                                    mapmenu.MoveNext();
                                }
                                mapmenu.Confirm();
                                break;
                            }
                            if (mapCap > 0 && mapFrame > mapCap) {
                                mapmenu.Confirm(); // auto-confirm (first level)
                                break;
                            }
                            RwPlatformPollEvents();
                            if (RwPlatformShouldClose() == TRUE) {
                                bQuitApp = TRUE;
                                break;
                            }
                            const RwBool down =
                                RwPlatformKeyDown('S') ||
                                RwPlatformKeyDown(RW_KEY_DOWN);
                            const RwBool up =
                                RwPlatformKeyDown('W') ||
                                RwPlatformKeyDown(RW_KEY_UP);
                            const RwBool enter =
                                RwPlatformKeyDown(' ') ||
                                RwPlatformKeyDown(RW_KEY_ENTER);
                            const RwBool esc =
                                RwPlatformKeyDown(RW_KEY_ESCAPE) ||
                                RwPlatformKeyDown('Q');
                            if (down == TRUE && s_prevMapDown == FALSE) {
                                mapmenu.MoveNext();
                            }
                            if (up == TRUE && s_prevMapUp == FALSE) {
                                mapmenu.MovePrev();
                            }
                            if (esc == TRUE && s_prevMapEsc == FALSE) {
                                s_prevMapDown = down; s_prevMapUp = up;
                                s_prevMapEnter = enter; s_prevMapEsc = esc;
                                if (mapmenu.Back() == TRUE) {
                                    break; // no selection -> fallback world
                                }
                            }
                            if (enter == TRUE && s_prevMapEnter == FALSE) {
                                s_prevMapDown = down; s_prevMapUp = up;
                                s_prevMapEnter = enter; s_prevMapEsc = esc;
                                if (mapmenu.Confirm() == TRUE) {
                                    break; // MapMenuEnter
                                }
                            }
                            s_prevMapDown = down; s_prevMapUp = up;
                            s_prevMapEnter = enter; s_prevMapEsc = esc;
                            mapmenu.Update(1.0f / 60.0f);
                            if (RenderPlatform_BeginFrame(nullptr) != TRUE) {
                                break;
                            }
                            if (bWorldLoaded == TRUE &&
                                pipeline.pClump != nullptr) {
                                RenderPlatform_RenderScene();
                            }
                            mapmenu.Render();
                            if (pMapShotPath != nullptr && mapFrame == 2) {
                                RenderPlatform_RequestScreenshot(
                                    pMapShotPath);
                            }
                            RenderPlatform_EndFrame();
                        }
                        // Selected level -> switch world pipeline
                        // (setup block runs AFTER -- single load).
                        const RwInt32 sel = mapmenu.TakeSelectedLevel();
                        if (sel >= 0 && bQuitApp == FALSE) {
                            const char *core = mapmenu.GetLevelFile(sel);
                            if (core != nullptr) {
                                char levelName[128];
                                char levelPath[512] = {0};
                                std::snprintf(levelName, sizeof(levelName),
                                              "Streams/%s.RWS", core);
                                // Relative depth centrally resolved
                                // (no manual prefix scan)
                                RwsAssets_Resolve(levelName, levelPath,
                                                  sizeof(levelPath));
                                if (levelPath[0] != 0) {
                                    // same file -> no reload
                                    const char *curBase = pWorldPath;
                                    if (curBase != nullptr) {
                                        const char *slash =
                                            std::strrchr(curBase, '/');
                                        curBase = (slash != nullptr)
                                                      ? slash + 1
                                                      : curBase;
                                    }
                                    const char *newSlash =
                                        std::strrchr(levelPath, '/');
                                    const char *newBase =
                                        (newSlash != nullptr) ? newSlash + 1
                                                              : levelPath;
                                    if (curBase == nullptr ||
                                        std::strcmp(curBase, newBase) != 0) {
                                        // 512: same size as resolved path buffer
                                        // -> no truncation.
                                        char prevPath[512] = {0};
                                        if (pWorldPath != nullptr) {
                                            std::snprintf(
                                                prevPath, sizeof(prevPath),
                                                "%s", pWorldPath);
                                        }
                                        RwsPipeline_Unload(&pipeline);
                                        bWorldLoaded = FALSE;
                                        RenderPlatform_LoadWorldDictionary(
                                            levelPath);
                                        if (RwsPipeline_Load(
                                                levelPath,
                                                &pipeline) == TRUE &&
                                            pipeline.pClump != nullptr) {
                                            std::snprintf(
                                                mapLevelPath,
                                                sizeof(mapLevelPath), "%s",
                                                levelPath);
                                            pWorldPath = mapLevelPath;
                                            bWorldLoaded = TRUE;
                                            std::printf(
                                                "MapMenu: level '%s' "
                                                "loaded\n",
                                                mapmenu.GetLevelDisplay(
                                                    sel));
                                        } else {
                                            RwsPipeline_Unload(&pipeline);
                                            RenderPlatform_ClearWorldDictionary();
                                            if (prevPath[0] != 0) {
                                                RenderPlatform_LoadWorldDictionary(
                                                    prevPath);
                                            }
                                            if (prevPath[0] != 0 &&
                                                RwsPipeline_Load(
                                                    prevPath,
                                                    &pipeline) == TRUE &&
                                                pipeline.pClump != nullptr) {
                                                bWorldLoaded = TRUE;
                                            }
                                            std::printf(
                                                "MapMenu: level load "
                                                "failed, keep '%s'\n",
                                                (pWorldPath != nullptr)
                                                    ? pWorldPath
                                                    : "(none)");
                                        }
                                    } else {
                                        std::printf(
                                            "MapMenu: level '%s' already "
                                            "loaded\n",
                                            mapmenu.GetLevelDisplay(sel));
                                    }
                                }
                            }
                        }
                        RenderPlatform_ClearTextures(); // map textures
                        break; // first valid map file
                    }
                }
                break; // first valid menu file
            }
        }
        if (bQuitApp == TRUE) {
            delete pOcean;
            pOcean = nullptr;
            RwsPipeline_Unload(&pipeline);
            RenderPlatform_Terminate();
            RwEngineStop();
            RwEngineClose();
            RwEngineTerm();
            return 0;
        }
    }

    // World setup (after boot-flow -- MAPMENU selection set up with single
    // load). 4.6q camera + 4.6p spawn/ocean carried over.
    // 4.6q: camera state lives throughout loop (freecam WASD/QE + mouse look).
    RwReal camPos[3] = {0.0f, 180.0f, 420.0f};
    RwReal camLook[3] = {0.0f, 0.0f, 0.0f};
    RwReal camYaw = 0.0f;   // rad -- mouse/arrow key pitch/yaw
    RwReal camPitch = 0.0f;
    RwReal camSpan = 400.0f; // world scale -- freecam speed base
    if (bWorldLoaded == TRUE && pipeline.pClump != nullptr) {
        const char *bgmTrack = "BeaverM";
        if (pWorldPath != nullptr) {
            if (std::strstr(pWorldPath, "VIKING") || std::strstr(pWorldPath, "Viking")) bgmTrack = "VikingM";
            else if (std::strstr(pWorldPath, "NORDIC") || std::strstr(pWorldPath, "Nordic")) bgmTrack = "NordicM";
            else if (std::strstr(pWorldPath, "IDUNA") || std::strstr(pWorldPath, "Iduna")) bgmTrack = "IdunaM";
            else if (std::strstr(pWorldPath, "BEAVER") || std::strstr(pWorldPath, "Beaver")) bgmTrack = "BeaverM";
        }
        audioMgr.PlayStream(bgmTrack, TRUE);
        speakMgr.PlaySpeak(1, 0); // Introductory speech line

        if (RenderPlatform_AddClump(pipeline.pClump) != TRUE) {
            std::fprintf(stderr, "CannonCruise: AddClump failed\n");
        }
        // Phase 4.6p -- spawn chain: 0x0704 registrations -> CClassFactory
        // create + apply (known classes; unknown silently skipped).
        if (GameBehaviours_Register() == TRUE) {
            WorldSpawnStats spawnStats;
            const RwInt32 spawned =
                WorldSpawn_FromFile(pWorldPath, &spawnStats);
            std::printf("WorldSpawn: records=%u created=%u skipped=%u "
                        "applied=%d\n",
                        spawnStats.objectRecords, spawnStats.instancesCreated,
                        spawnStats.instancesSkipped, (int)spawned);
        }
        // Phase 4.6q -- camera auto-framing: overview from world bbox
        // (0x0704 camera data is a chase-rig -- without follow target,
        // not world coordinates; CANNONCRUISE_CAM=DATA for data camera).
        const char *camMode = std::getenv("CANNONCRUISE_CAM");
        if (camMode != nullptr && std::strcmp(camMode, "DATA") == 0) {
            if (RwsCamera_LoadPose((pWorldPath != nullptr) ? pWorldPath : "",
                                   camPos, camLook) == TRUE) {
                std::printf("CannonCruise: camera DATA pos=(%.2f, %.2f, "
                            "%.2f) look=(%.2f, %.2f, %.2f)\n",
                            (double)camPos[0], (double)camPos[1],
                            (double)camPos[2], (double)camLook[0],
                            (double)camLook[1], (double)camLook[2]);
            }
        } else {
            WorldGeometryBounds bb;
            if (WorldGeometry_GetLastBounds(&bb) == TRUE) {
                const RwReal cx = (bb.min[0] + bb.max[0]) * 0.5f;
                const RwReal cy = (bb.min[1] + bb.max[1]) * 0.5f;
                const RwReal cz = (bb.min[2] + bb.max[2]) * 0.5f;
                const RwReal dx = bb.max[0] - bb.min[0];
                const RwReal dz = bb.max[2] - bb.min[2];
                const RwReal span = (dx > dz) ? dx : dz;
                // look at center (known-good direction), distance span*0.62:
                // island dominates view -- ground lookAt was empty frame
                // 4.6q rev: lower+close (island was thin sliver at horizon)
                camPos[0] = cx;
                camPos[1] = cy + span * 0.14f;
                camPos[2] = cz + span * 0.38f;
                camLook[0] = cx;
                camLook[1] = cy;
                camLook[2] = cz;
                camSpan = span; // freecam speed base
                // Diagnostics: CANNONCRUISE_LOOKAT=x,y,z[,dist] -> override
                {
                    const char *lookEnv =
                        std::getenv("CANNONCRUISE_LOOKAT");
                    if (lookEnv != nullptr) {
                        float lx = cx, ly = cy, lz = cz, dd = -1.0f;
                        if (std::sscanf(lookEnv, "%f,%f,%f,%f", &lx, &ly,
                                        &lz, &dd) >= 3) {
                            camLook[0] = lx;
                            camLook[1] = ly;
                            camLook[2] = lz;
                            if (dd > 0.0f) {
                                RwReal f[3] = {lx - camPos[0], ly - camPos[1],
                                               lz - camPos[2]};
                                RwReal fl = std::sqrt(f[0] * f[0] +
                                                      f[1] * f[1] +
                                                      f[2] * f[2]);
                                if (fl > 1e-4f) {
                                    camPos[0] = lx - f[0] / fl * dd;
                                    camPos[1] = ly - f[1] / fl * dd;
                                    camPos[2] = lz - f[2] / fl * dd;
                                }
                            }
                        }
                    }
                }
                std::printf("CannonCruise: camera AUTO bbox "
                            "(%.0f..%.0f, %.0f..%.0f, %.0f..%.0f) "
                            "pos=(%.1f, %.1f, %.1f)\n",
                            (double)bb.min[0], (double)bb.max[0],
                            (double)bb.min[1], (double)bb.max[1],
                            (double)bb.min[2], (double)bb.max[2],
                            (double)camPos[0], (double)camPos[1],
                            (double)camPos[2]);
            }
        }
        RenderPlatform_SetCameraPose(camPos, camLook);
        // mouse look init: yaw/pitch derived from (look-pos) direction
        {
            RwReal f[3] = {camLook[0] - camPos[0], camLook[1] - camPos[1],
                           camLook[2] - camPos[2]};
            const RwReal len = std::sqrt(f[0] * f[0] + f[1] * f[1] +
                                         f[2] * f[2]);
            if (len > 1e-4f) {
                camYaw = std::atan2(f[0] / len, f[2] / len);
                camPitch = std::asin(f[1] / len);
            }
        }
        RwPlatformSetMouseCapture(TRUE); // cursor hidden -- mouse look
        // Phase 4.6p (wire-in) -- ocean: 5x5 tile grid covering bbox XZ
        // (tile = span/2 -> total span*2.5; y=0 water level, VIKING min.y=-9).
        {
            WorldGeometryBounds bb;
            if (WorldGeometry_GetLastBounds(&bb) == TRUE) {
                const RwReal cx = (bb.min[0] + bb.max[0]) * 0.5f;
                const RwReal cz = (bb.min[2] + bb.max[2]) * 0.5f;
                const RwReal dx = bb.max[0] - bb.min[0];
                const RwReal dz = bb.max[2] - bb.min[2];
                const RwReal span = (dx > dz) ? dx : dz;
                const RwReal tileSize = span * 0.5f;
                pOcean = new COcean;
                pOcean->Initialize(5, 5, tileSize,
                                   cx - 2.5f * tileSize,
                                   cz - 2.5f * tileSize);
                std::printf("CannonCruise: ocean 5x5 tiles tileSize=%.0f "
                            "origin=(%.0f, %.0f)\n",
                            (double)tileSize,
                            (double)(cx - 2.5f * tileSize),
                            (double)(cz - 2.5f * tileSize));
            }
        }
    }
    // Geometry/material diagnostics (root cause of colorlessness).
    if (std::getenv("CANNONCRUISE_GEODIAG") != nullptr) {
        WorldGeometry_Diagnose(pipeline.pClump, 400);
    }

    // Frame loop -- if world loaded, run until window close (unlimited;
    // CANNONCRUISE_FRAMES env for test cap, e.g. 300), else 3 frames
    // platform smoke.
    int frameCap = -1; // -1 = unlimited
    const char *capEnv = std::getenv("CANNONCRUISE_FRAMES");
    if (capEnv != nullptr) {
        frameCap = std::atoi(capEnv);
    }
    const int kFrames = (bWorldLoaded == TRUE) ? frameCap : 3;
    int framesRendered = 0;
    s_gameState = GAMESTATE_GAMEPLAY; // video-kurgu: NORMAL kamera kipi
    for (int frame = 0; kFrames < 0 || frame < kFrames; ++frame) {
        RwPlatformPollEvents(); // GLFW event pump (keys + mouse delta)
        if (RwPlatformShouldClose() == TRUE) {
            break;
        }
        if (RenderPlatform_BeginFrame(nullptr) != TRUE) {
            std::fprintf(stderr, "CannonCruise: BeginFrame %d failed\n",
                         frame);
            break;
        }
        if (bWorldLoaded == TRUE && pipeline.pClump != nullptr) {
            RenderPlatform_RenderScene();
            if (RwPlatformKeyDown(' ')) {
                audioMgr.PlaySound("Audio_CannonFire");
            }
            if (RwPlatformKeyDown('T')) {
                speakMgr.PlaySpeak(2, 0);
            } // world->render() -- between Begin/End
            // Phase 4.6q -- freecam: mouse look + WASD (in view plane) +
            // Q/E (vertical) + arrow keys (yaw/pitch). Null platform no key/
            // mouse -> no-op. View derived from yaw/pitch.
            {
                const RwReal kDt = 1.0f / 60.0f;
                const RwReal speed = camSpan * 0.10f * kDt;
                const RwReal turn = 1.8f * kDt; // arrow key rad/s
                const RwReal sens = 0.0035f;    // mouse radians/pixel
                RwBool lookChanged = FALSE;
                if (RwPlatformKeyDown(RW_KEY_LEFT)) {
                    camYaw += turn;
                    lookChanged = TRUE;
                }
                if (RwPlatformKeyDown(RW_KEY_RIGHT)) {
                    camYaw -= turn;
                    lookChanged = TRUE;
                }
                if (RwPlatformKeyDown(RW_KEY_UP)) {
                    camPitch += turn;
                    lookChanged = TRUE;
                }
                if (RwPlatformKeyDown(RW_KEY_DOWN)) {
                    camPitch -= turn;
                    lookChanged = TRUE;
                }
                int mdx = 0;
                int mdy = 0;
                if (RwPlatformMouseDelta(&mdx, &mdy) == TRUE) {
                    camYaw += (RwReal)mdx * sens;
                    camPitch -= (RwReal)mdy * sens; // drag up = look up
                    lookChanged = TRUE;
                }
                const RwReal kMaxPitch = 1.55f; // ~89Â°
                if (camPitch > kMaxPitch) {
                    camPitch = kMaxPitch;
                }
                if (camPitch < -kMaxPitch) {
                    camPitch = -kMaxPitch;
                }
                // view: fwd = (sin(yaw)cos(p), sin(p), cos(yaw)cos(p))
                const RwReal cp = std::cos(camPitch);
                const RwReal fwd[3] = {std::sin(camYaw) * cp,
                                       std::sin(camPitch),
                                       std::cos(camYaw) * cp};
                const RwReal right[3] = {std::cos(camYaw), 0.0f,
                                         -std::sin(camYaw)}; // yaw+90Â°
                RwReal delta[3] = {0.0f, 0.0f, 0.0f};
                if (RwPlatformKeyDown('W')) {
                    delta[0] += fwd[0] * speed;
                    delta[1] += fwd[1] * speed;
                    delta[2] += fwd[2] * speed;
                }
                if (RwPlatformKeyDown('S')) {
                    delta[0] -= fwd[0] * speed;
                    delta[1] -= fwd[1] * speed;
                    delta[2] -= fwd[2] * speed;
                }
                if (RwPlatformKeyDown('A')) {
                    delta[0] -= right[0] * speed;
                    delta[2] -= right[2] * speed;
                }
                if (RwPlatformKeyDown('D')) {
                    delta[0] += right[0] * speed;
                    delta[2] += right[2] * speed;
                }
                if (RwPlatformKeyDown('Q')) {
                    delta[1] -= speed;
                }
                if (RwPlatformKeyDown('E')) {
                    delta[1] += speed;
                }
                const RwBool moved =
                    (delta[0] != 0.0f || delta[1] != 0.0f || delta[2] != 0.0f)
                        ? TRUE
                        : FALSE;
                if (moved == TRUE) {
                    camPos[0] += delta[0];
                    camPos[1] += delta[1];
                    camPos[2] += delta[2];
                }
                if (lookChanged == TRUE || moved == TRUE) {
                    camLook[0] = camPos[0] + fwd[0];
                    camLook[1] = camPos[1] + fwd[1];
                    camLook[2] = camPos[2] + fwd[2];
                    RenderPlatform_SetCameraPose(camPos, camLook);
                }
            }
            if (pOcean != nullptr) {
                pOcean->Update((RwReal)frame * (1.0f / 60.0f));
                pOcean->Render(); // between Begin/End
            }
        }
        if (RenderPlatform_RenderTestQuad(0.0f, 0.0f, 0.25f, 0.25f,
                                          0xFF40A0FF) != TRUE) {
            std::fprintf(stderr, "CannonCruise: TestQuad %d failed\n",
                         frame);
            break;
        }
        // World frame capture (if phase pulls didn't consume; one-time only).
        if (pShotPath != nullptr && frame == 2) {
            RenderPlatform_RequestScreenshot(pShotPath);
        }
        RenderPlatform_EndFrame();
        ++framesRendered;
    }

    if (kFrames < 0) {
        std::printf(
            "CannonCruise: %d frames OK (unlimited; world=%p camera=%p "
            "clump=%p shape=%p assets=%u)\n",
            framesRendered, RenderPlatform_GetWorld(), RenderPlatform_GetCamera(),
            pipeline.pClump, pipeline.pCollisionShape,
            (bWorldLoaded == TRUE) ? pipeline.assetCount : 0u);
    } else {
        std::printf(
            "CannonCruise: %d/%d frames OK (world=%p camera=%p clump=%p "
            "shape=%p assets=%u)\n",
            framesRendered, kFrames, RenderPlatform_GetWorld(),
            RenderPlatform_GetCamera(), pipeline.pClump,
            pipeline.pCollisionShape,
            (bWorldLoaded == TRUE) ? pipeline.assetCount : 0u);
    }

    audioMgr.StopAllVoices();
    speakMgr.StopSpeak();
    delete pOcean; // COcean::Shutdown + tile cleanup
    RwPlatformSetMouseCapture(FALSE); // cursor back

    RwsPipeline_Unload(&pipeline);
    RenderPlatform_Terminate();
    RwEngineStop();
    RwEngineClose();
    RwEngineTerm();

    // Window close on world path (ShouldClose) counts as success.
    return (framesRendered > 0) ? 0 : 1;
}
