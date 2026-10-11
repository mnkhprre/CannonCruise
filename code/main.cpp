// ============================================================================
// CannonCruise - Platform Smoke + RWS Pipeline Girişi (main.cpp)
// Faz 4.5: Link doğrulama (-force_load tüm sembolleri çözer).
// Faz 4.6n: Render platformu kare döngüsü smoke.
// Faz 4.6o+: Opsiyonel RWS argümanı — uçtan uca dünya boru hattı:
//   RWS envanter → WorldGeometry clump instantiate → RenderPlatform'a
//   ekle → StaticCollisionData (.col) trimesh shape → kare döngüsü
//   (clear/Begin → world render → Im2D kare → End). NULL platform
//   headless (device no-op); GL3 build'de gerçek pencere/piksel.
//
// Kullanım: ./CannonCruise [world.RWS]
//
// Oyunun gerçek çalışma zamanı girişi Windows'ta startup.cpp / WinMain.
// ============================================================================

#include "CameraState.h"
#include "Ocean.h"
#include "PhysicsBackend.h"
#include "RenderPlatform.h"
#include "Rt2dMaestro.h"
#include "Rt2dMaestroStream.h"
#include "MenuBehavior.h"
#include "AfterGameMenuBehavior.h"

// Video-kurgu (SCR.mov/SCR1.mov) oyun durum makinesi — Faz 4.6s hedefi.
// BOOT → LOGO_ITE → LOADING ⇄ MAINMENU ⇄ OPTIONS ⇄ SOUND/CREDITS → MAPMENU
// → LEVELINTRO → GAMEPLAY(NORMAL/CANNON/BINOCULARS) ⇄ PAUSE → LEVELCOMPLETE.
// Şu an BOOT→MAINMENU→GAMEPLAY iskeleti çalışır; ara durumlar fazlarca dolar.
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
#include "rwcore.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

namespace {

/// RWS envanteri + WorldGeometry clump + .col shape — tek dünya boru hattı.
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
                        "WorldGeometry: %u atomics, %u verts, %u tris "
                        "(%u bytes) -> clump %s\n",
                        info.atomicCount, info.totalVertices,
                        info.totalTriangles, asset.resourceLength,
                        pOut->pClump ? "OK" : "FAIL");
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
            std::free(pBuf); // shape mesh'i kopyalar — payload serbest
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

// Faz 4.6p — RWS 0x0704 CWbCameraBehavior kayıtlarından dünya kamera
// yerleşimi (VIKING01 obj[264]: id0 pos, id6 dir, id4 FOV). İlk props'u
// olan CWbCameraBehavior apply edilir (binary: CCameraState singleton).
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
    // id6 (+0x24) MUTLAK bakış hedefi (lookAt point) — yön vektörü DEĞİL:
    // VIKING01 pos=(0.35,7,0), hedef=(0.85,0.85,0.85) ≈ dünya merkezine
    // aşağı bakış. pos+dir yapsaydık yukarı-sağa (gökyüzüne) bakardık.
    lookOut[0] = state.m_dir[0];
    lookOut[1] = state.m_dir[1];
    lookOut[2] = state.m_dir[2];
    return TRUE;
}

} // namespace

int main(int argc, char **argv) {
    std::printf("CannonCruise: link verification OK\n");

    // Engine boot — test deseni (Init/Open/Start; teardown Stop/Close/Term).
    if (RwEngineInit(0, 0, 0) != TRUE || RwEngineOpen() != TRUE ||
        RwEngineStart() != TRUE) {
        std::fprintf(stderr, "CannonCruise: engine init failed\n");
        return 2;
    }

    if (RenderPlatform_Initialize(1280, 720) != TRUE) {
        std::fprintf(stderr, "CannonCruise: render platform init failed\n");
        RwEngineStop();
        RwEngineClose();
        RwEngineTerm();
        return 2;
    }

    // Dünya: argüman yoksa oyun akışı gibi MAINMENU.RWS (binary:
    // intro → menü). Not: menu RWS kaynakları GÖMÜLÜ değil (resLen=0,
    // kaynak yolu X:\RwStudio\...\{GUID}.dff — build-output referansı,
    // elimizde yok) → clump boşsa ilk çalışma dünyasına (VIKING01) düş.
    const char *pWorldPath = (argc > 1) ? argv[1] : nullptr;
    // 4.6r — boot akışı: argümansız çalıştırmada giriş zinciri (INTRO→menü)
    // overlay olarak dünya üstünde akar; dünya argüsü verilirse doğrudan
    // interaktif dünyaya girilir (giriş akışı atlanır).
    const RwBool bBootFlow = (argc <= 1) ? TRUE : FALSE;
    // CANNONCRUISE_SCREENSHOT=<yol.png> — ilk overlay karesini PNG kaydet
    // (intro karesi; CANNONCRUISE_INTRO_FRAMES=0 ile menü karesi). Tek seferlik.
    const char *pShotPath = std::getenv("CANNONCRUISE_SCREENSHOT");
    // CANNONCRUISE_SCREENSHOT_MAP=<yol.png> — MAPMENU karesi.
    const char *pMapShotPath = std::getenv("CANNONCRUISE_SCREENSHOT_MAP");
    // MAPMENU seçimiyle güncellenen dünya yolu (kurulum bloğu kullanır).
    char mapLevelPath[256] = {0};

    RwsPipeline pipeline = {};
    RwBool bWorldLoaded = FALSE;
    if (pWorldPath != nullptr) {
        // Dünya dokuları ÖNCE (malzemeler stream anında çözülür).
        RenderPlatform_LoadWorldDictionary(pWorldPath);
        bWorldLoaded = RwsPipeline_Load(pWorldPath, &pipeline);
    } else {
        static const char *kBootCandidates[] = {
            "Streams/MAINMENU.RWS",    "../Streams/MAINMENU.RWS",
            "../../Streams/MAINMENU.RWS",
            "Streams/VIKING01.RWS",    "../Streams/VIKING01.RWS",
            "../../Streams/VIKING01.RWS",
        };
        for (RwUInt32 i = 0; i < 6; ++i) {
            FILE *f = std::fopen(kBootCandidates[i], "rb");
            if (f == nullptr) {
                continue;
            }
            std::fclose(f);
            RenderPlatform_LoadWorldDictionary(kBootCandidates[i]);
            if (RwsPipeline_Load(kBootCandidates[i], &pipeline) == TRUE &&
                pipeline.pClump != nullptr) {
                pWorldPath = kBootCandidates[i];
                bWorldLoaded = TRUE;
                break;
            }
            RwsPipeline_Unload(&pipeline); // clump yok (menü: external dff)
            RenderPlatform_ClearWorldDictionary();
        }
    }
    // Not: dünya kurulumu (AddClump/spawn/kamera/okyanus) boot-flow
    // SONRASINA taşındı — MAPMENU seçimi tek yüklemeyle kurulur
    // (reload + RemoveClump + çifte-spawn gerekmez). (4.6r-4 MAPMENU)
    COcean *pOcean = nullptr; // quit-cleanup bildirimi (kurulum aşağıda)

    // 4.6r — MAINMENU menü fazı (giriş akışı: GLOBAL→INTRO skip→MAINMENU→
    // dünya). INTRO1-3/FMV {GUID} dosyaları elimizde yok → skip; MAINMENU
    // external {GUID}.dff yok (clump boş) ama gömülü Menu.anm stream'i
    // Rt2dMaestro ile render edilir + komut düğümleri gezilir.
    // CANNONCRUISE_MENU_FRAMES: kare kapak (0=skip menü, -1=sınırsız).
    RwBool bQuitApp = FALSE;
    if (bBootFlow == TRUE) {
        s_gameState = GAMESTATE_LOGO_ITE;
        // 4.6r-6 — INTRO fazı (binary: INTRO1→2→3→MAPMENU→MAINMENU).
        // INTRO RWS = FMV sahne tanımlayıcı: CFMVBehavior + dış {GUID}
        // txd/dff/col/pfm (Bink + geometri elimizde yok) → başlık kartı
        // + chunk log + Esc skip (fallback: sonraki intro/menü).
        int introCap = 180; // ~3 sn @60fps; CANNONCRUISE_INTRO_FRAMES
        const char *introEnv = std::getenv("CANNONCRUISE_INTRO_FRAMES");
        if (introEnv != nullptr) {
            introCap = std::atoi(introEnv);
        }
        static const char *kPrefixes[] = {"", "../", "../../"};
        static const char *kIntros[] = {"Streams/INTRO1.RWS",
                                        "Streams/INTRO2.RWS",
                                        "Streams/INTRO3.RWS"};
        for (RwUInt32 ii = 0; ii < 3 && bQuitApp == FALSE; ++ii) {
            FILE *inf = nullptr;
            char ipath[256];
            for (RwUInt32 pi = 0; pi < 3; ++pi) {
                std::snprintf(ipath, sizeof(ipath), "%s%s", kPrefixes[pi],
                              kIntros[ii]);
                inf = std::fopen(ipath, "rb");
                if (inf != nullptr) {
                    break;
                }
            }
            if (inf == nullptr) {
                continue;
            }
            std::fseek(inf, 0, SEEK_END);
            long isize = std::ftell(inf);
            std::fseek(inf, 0, SEEK_SET);
            if (isize <= 0 || isize > 4L * 1024 * 1024) {
                std::fclose(inf);
                continue;
            }
            std::vector<unsigned char> ibuf((size_t)isize);
            if (std::fread(ibuf.data(), 1, (size_t)isize, inf) != (size_t)isize) {
                std::fclose(inf);
                continue;
            }
            std::fclose(inf);
            // chunk tarama: GUID + davranış adı + hedef sahne çıkar
            char guid[64] = {0};
            char behaviour[32] = {0};
            char target[32] = {0};
            for (RwUInt32 off = 0; off + 12 <= (RwUInt32)isize;) {
                RwUInt32 ctype = 0, clen = 0;
                std::memcpy(&ctype, ibuf.data() + off, 4);
                std::memcpy(&clen, ibuf.data() + off + 4, 4);
                if (clen < 4 || off + 12 + clen > (RwUInt32)isize) {
                    break;
                }
                const unsigned char *pl = ibuf.data() + off + 12;
                // GUID: '{' ... '}'
                if (guid[0] == 0) {
                    for (RwUInt32 j = 0; j + 8 < clen; ++j) {
                        if (pl[j] == '{') {
                            RwUInt32 k = 0;
                            while (j + k < clen && pl[j + k] != '}' &&
                                   k < sizeof(guid) - 1) {
                                guid[k] = (char)pl[j + k];
                                ++k;
                            }
                            if (j + k < clen) {
                                guid[k] = '}';
                            }
                            break;
                        }
                    }
                }
                // yazdırılabilir uzun koşu (davranış + hedef)
                if (ctype == 0x0704 || ctype == 0x071c) {
                    char run[256];
                    RwUInt32 rn = 0;
                    for (RwUInt32 j = 0; j < clen && rn < sizeof(run) - 1; ++j) {
                        const unsigned char c = pl[j];
                        run[rn++] = (c >= 32 && c < 127) ? (char)c : '\n';
                    }
                    run[rn] = 0;
                    // '\n' ile böl + trim; Behavior içerenleri atla,
                    // kalan ilk kısa token = davranış instance adı
                    // ("Intro"/"Intro2"/"Intro3"), sonraki = hedef sahne.
                    for (char *tok = std::strtok(run, "\n"); tok != nullptr;
                         tok = std::strtok(nullptr, "\n")) {
                        while (*tok == ' ') {
                            ++tok;
                        }
                        const size_t l = std::strlen(tok);
                        if (l < 3 || l >= 32) {
                            continue;
                        }
                        // yalnız identifier benzeri: [A-Za-z0-9_]
                        RwBool ident = TRUE;
                        for (size_t ci = 0; ci < l; ++ci) {
                            const char c = tok[ci];
                            if (!((c >= 'A' && c <= 'Z') ||
                                  (c >= 'a' && c <= 'z') ||
                                  (c >= '0' && c <= '9') || c == '_')) {
                                ident = FALSE;
                                break;
                            }
                        }
                        if (ident == FALSE) {
                            continue; // binary artığı
                        }
                        if (std::strstr(tok, "Behavior") != nullptr ||
                            std::strstr(tok, "Attribute") != nullptr) {
                            continue; // class/nesne adları — atla
                        }
                        if (behaviour[0] == 0) {
                            std::snprintf(behaviour, sizeof(behaviour), "%s",
                                          tok);
                        } else if (target[0] == 0) {
                            std::snprintf(target, sizeof(target), "%s", tok);
                        }
                    }
                }
                off += 12 + clen;
            }
            std::printf("Intro%d: %s behaviour='%s' target='%s' (FMV skip)\n",
                        (int)ii + 1, guid, behaviour, target);
            // başlık kartı döngüsü
            static RwBool s_prevEsc = FALSE;
            // Faz girişi: basılı Esc'yi tüket (önceki fazdan sızma atlamasın)
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
                // intro kartı — Im2D overlay (ZTEST kapalı;3D sahne derinlik yazdı)
                RenderPlatform_BeginOverlay2D();
                char l1[64], l2[64], l3[96];
                std::snprintf(l1, sizeof(l1), "CANNONCRUISE");
                std::snprintf(l2, sizeof(l2), "INTRO %d", (int)ii + 1);
                std::snprintf(l3, sizeof(l3),
                              "FMV SKIPPED (external {GUID} assets missing)");
                ShowText_DrawBitmapText(l1, 40.0f, 200.0f, 5, 220, 230, 255,
                                        255);
                ShowText_DrawBitmapText(l2, 40.0f, 280.0f, 4, 200, 210, 230,
                                        255);
                ShowText_DrawBitmapText(behaviour, 40.0f, 350.0f, 3, 180, 190,
                                        200, 255);
                ShowText_DrawBitmapText(target, 40.0f, 390.0f, 3, 180, 190,
                                        200, 255);
                ShowText_DrawBitmapText(l3, 40.0f, 660.0f, 2, 140, 150, 160,
                                        255);
                RenderPlatform_EndOverlay2D();
                if (pShotPath != nullptr && fr == 1) {
                    RenderPlatform_RequestScreenshot(pShotPath);
                }
                RenderPlatform_EndFrame();
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

        int menuCap = -1; // -1 = sınırsız (Enter/Esc bekle); test env ile
        const char *menuEnv = std::getenv("CANNONCRUISE_MENU_FRAMES");
        if (menuEnv != nullptr) {
            menuCap = std::atoi(menuEnv);
        }
        RwBool bMenuBackedOut = FALSE; // MAIN-Esc çıkışı (harita açılmaz)
        if (menuCap != 0) {
            static const char *kMenuFiles[] = {
                "Streams/MAINMENU.RWS",    "../Streams/MAINMENU.RWS",
                "../../Streams/MAINMENU.RWS",
            };
            for (RwUInt32 mi = 0; mi < 3; ++mi) {
                // Altyapı: tek-yol maestro yükleme (0x0716-walk tekilleşti).
                Rt2dMaestro *pMaestro =
                    Rt2dMaestro_LoadFromRwsFile(kMenuFiles[mi]);
                if (pMaestro == nullptr) {
                    continue;
                }
                // Video-kurgu: menü durumu CMenuBehavior'da yaşar (MAINMENU/
                // OPTIONS/SOUND/PAUSE aynı Menu.anm'i paylaşır).
                CMenuBehavior menu;
                menu.AttachMaestro(pMaestro);
                pMaestro = menu.GetMaestro();
                // gömülü menü dokuları (TexDictionary → GL; Menu_074 zemin,
                // Menu_076 logo) — RWS içinde, dış dosya değil.
                menu.LoadTextures(kMenuFiles[mi]);
                s_gameState = GAMESTATE_MAINMENU;
                std::printf("Menu: nodes=%u commands=%d canvas=%.0fx%.0f\n",
                            pMaestro->info.nodeCount, pMaestro->commandCount,
                            (double)pMaestro->info.canvasWidth,
                            (double)pMaestro->info.canvasHeight);
                for (RwInt32 ci = 0; ci < pMaestro->commandCount; ++ci) {
                    std::printf("  [%d] %s\n", ci,
                                pMaestro->info.nodes[pMaestro->commands[ci]]
                                    .name);
                }
                // menü döngüsü: seçim (W/S/↑/↓), onay (Enter/Space), kaçış
                // (Esc/Q) — kare kapakta otomatik onay (NewSave akışı).
                int menuFrame = 0;
                static RwBool s_prevDown = FALSE, s_prevUp = FALSE,
                              s_prevEnter = FALSE, s_prevEsc = FALSE,
                              s_prevLeft = FALSE, s_prevRight = FALSE;
                // Faz girişi: basılı tuşları BİR KEZ tüket (intro'dan sızan
                // Enter/Space menüyü görünmez onaylamasın).
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
                        break; // otomatik onay — dünyaya geç
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
                    }
                    if (up == TRUE && s_prevUp == FALSE) {
                        menu.MovePrev();
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
                            break; // MAIN → döngüden çık
                        }
                    }
                    if (enter == TRUE && s_prevEnter == FALSE) {
                        s_prevDown = down; s_prevUp = up; s_prevEnter = enter;
                        s_prevEsc = esc;
                        s_prevLeft = left; s_prevRight = right;
                        if (menu.Confirm() == TRUE) {
                            break; // terminal eylem (NEW/LOAD/QUIT/CREDITS)
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
                // maestro sahipliği menu'de (dtor destroy eder)
                pMaestro = nullptr;
                // video-kurgu terminal eylem: NEW/LOAD → dünyaya devam,
                // QUIT → çıkış, CREDITS → log (CREDITS.RWS fazı bekliyor)
                const CMenuBehavior::EMenuAction act =
                    menu.TakePendingAction();
                if (act == CMenuBehavior::MENUACTION_QUITGAME) {
                    std::printf("Menu: action QUITGAME\n");
                    bQuitApp = TRUE;
                } else if (act == CMenuBehavior::MENUACTION_CREDITS) {
                    std::printf("Menu: action CREDITS (phase pending)\n");
                } else if (act == CMenuBehavior::MENUACTION_NEWGAME) {
                    std::printf("Menu: action NEWGAME\n");
                } else if (act == CMenuBehavior::MENUACTION_LOADGAME) {
                    std::printf("Menu: action LOADGAME\n");
                }
                RenderPlatform_ClearTextures(); // menü dokuları serbest
                // 4.6r-4 — MAPMENU fazı (CAG_Menu.anm + seviye seçimi).
                // NEW/LOAD sonrası (otomatik kapakta da); Esc-çıkışta atlanır.
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
                    // Test: otomatik seviye seçimi (etkileşimsiz reload yolu).
                    int mapAutoSel = -1;
                    const char *mapSelEnv =
                        std::getenv("CANNONCRUISE_MAPMENU_SELECT");
                    if (mapSelEnv != nullptr) {
                        mapAutoSel = std::atoi(mapSelEnv);
                    }
                    static const char *kMapFiles[] = {
                        "Streams/MAPMENU.RWS", "../Streams/MAPMENU.RWS",
                        "../../Streams/MAPMENU.RWS",
                    };
                    for (RwUInt32 gi = 0; gi < 3; ++gi) {
                        // Altyapı: tek-yol maestro yükleme.
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
                        // Faz girişi: basılı tuşları BİR KEZ tüket (menü
                        // onayındaki Enter haritayı görünmez geçmesin —
                        // "direkt viking 1" bug'ı).
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
                                // otomatik seçim (seçim 0'dan başlar)
                                for (int k = 0; k < mapAutoSel; ++k) {
                                    mapmenu.MoveNext();
                                }
                                mapmenu.Confirm();
                                break;
                            }
                            if (mapCap > 0 && mapFrame > mapCap) {
                                mapmenu.Confirm(); // otomatik onay (ilk seviye)
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
                                    break; // seçim yok — fallback dünya
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
                        // Onaylanan seviye → dünya boru hattını değiştir
                        // (kurulum bloğu SONRA çalışır — tek yükleme).
                        const RwInt32 sel = mapmenu.TakeSelectedLevel();
                        if (sel >= 0 && bQuitApp == FALSE) {
                            const char *core = mapmenu.GetLevelFile(sel);
                            if (core != nullptr) {
                                static const char *kPrefixes[] = {
                                    "", "../", "../../"};
                                char levelPath[256] = {0};
                                for (RwUInt32 pi = 0; pi < 3; ++pi) {
                                    std::snprintf(
                                        levelPath, sizeof(levelPath), "%s"
                                        "Streams/%s.RWS", kPrefixes[pi],
                                        core);
                                    FILE *lf = std::fopen(levelPath, "rb");
                                    if (lf != nullptr) {
                                        std::fclose(lf);
                                        break;
                                    }
                                    levelPath[0] = 0;
                                }
                                if (levelPath[0] != 0) {
                                    // aynı dosya → yeniden yükleme yok
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
                                        char prevPath[256] = {0};
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
                        RenderPlatform_ClearTextures(); // harita dokuları
                        break; // ilk geçerli harita dosyası
                    }
                }
                break; // ilk geçerli menü dosyası
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

    // Dünya kurulumu (boot-flow SONRASI — MAPMENU seçimi tek yüklemeyle
    // kurulur). 4.6q kamera + 4.6p spawn/okyanus aynen taşındı.
    // 4.6q: kamera durumu döngü boyunca yaşar (freecam WASD/QE + mouse look).
    RwReal camPos[3] = {0.0f, 180.0f, 420.0f};
    RwReal camLook[3] = {0.0f, 0.0f, 0.0f};
    RwReal camYaw = 0.0f;   // rad — mouse/ok tuşu pitch/yaw için
    RwReal camPitch = 0.0f;
    RwReal camSpan = 400.0f; // dünya ölçeği — freecam hız temeli
    if (bWorldLoaded == TRUE && pipeline.pClump != nullptr) {
        if (RenderPlatform_AddClump(pipeline.pClump) != TRUE) {
            std::fprintf(stderr, "CannonCruise: AddClump failed\n");
        }
        // Faz 4.6p — spawn zinciri: 0x0704 kayıtları → CClassFactory
        // create + apply (bilinen sınıflar; bilinmeyenler sessiz atla).
        if (GameBehaviours_Register() == TRUE) {
            WorldSpawnStats spawnStats;
            const RwInt32 spawned =
                WorldSpawn_FromFile(pWorldPath, &spawnStats);
            std::printf("WorldSpawn: records=%u created=%u skipped=%u "
                        "applied=%d\n",
                        spawnStats.objectRecords, spawnStats.instancesCreated,
                        spawnStats.instancesSkipped, (int)spawned);
        }
        // Faz 4.6q — kamera auto-framing: dünya bbox'ından genel bakış
        // (0x0704 kamera verisi bir chase-rig — takip hedefi olmadan
        // dünya koordinatı değil; CANNONCRUISE_CAM=DATA ile veri kamerası).
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
                // merkeze bakış (bilinen-iyi yön), mesafe span*0.62:
                // adaya çerçevede hakim — zemin lookAt'ı boş kare yapıyordu
                // 4.6q rev: alçalt+yaklaştır (ada ufukta ince şerit kalıyordu)
                camPos[0] = cx;
                camPos[1] = cy + span * 0.14f;
                camPos[2] = cz + span * 0.38f;
                camLook[0] = cx;
                camLook[1] = cy;
                camLook[2] = cz;
                camSpan = span; // freecam hız ölçeği
                // Teşhis: CANNONCRUISE_LOOKAT=x,y,z[,dist] → hedef/mesafe ezer.
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
        // mouse look başlangıcı: yaw/pitch'i (look-pos) yönünden türet
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
        RwPlatformSetMouseCapture(TRUE); // imleç gizli — mouse look
        // Faz 4.6p (wire-in) — okyanus: bbox XZ'ini kapsayan 5×5 tile ızgara
        // (tile = span/2 → toplam span*2.5; y=0 su seviyesi, VIKING min.y=-9).
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
    // Geometri/material teşhisi (renksizlik kök nedeni).
    if (std::getenv("CANNONCRUISE_GEODIAG") != nullptr) {
        WorldGeometry_Diagnose(pipeline.pClump, 400);
    }

    // Kare döngüsü — dünya varsa pencere kapanana dek (sınırsız;
    // CANNONCRUISE_FRAMES env ile test kapağı, ör. 300), yoksa 3 kare
    // platform smoke.
    int frameCap = -1; // -1 = sınırsız
    const char *capEnv = std::getenv("CANNONCRUISE_FRAMES");
    if (capEnv != nullptr) {
        frameCap = std::atoi(capEnv);
    }
    const int kFrames = (bWorldLoaded == TRUE) ? frameCap : 3;
    int framesRendered = 0;
    s_gameState = GAMESTATE_GAMEPLAY; // video-kurgu: NORMAL kamera kipi
    for (int frame = 0; kFrames < 0 || frame < kFrames; ++frame) {
        RwPlatformPollEvents(); // GLFW olay tazeleme (tuş + mouse delta)
        if (RwPlatformShouldClose() == TRUE) {
            break;
        }
        if (RenderPlatform_BeginFrame(nullptr) != TRUE) {
            std::fprintf(stderr, "CannonCruise: BeginFrame %d failed\n",
                         frame);
            break;
        }
        if (bWorldLoaded == TRUE && pipeline.pClump != nullptr) {
            RenderPlatform_RenderScene(); // world->render() — Begin/End arası
            // Faz 4.6q — freecam: mouse look + WASD (bakış düzleminde) +
            // Q/E (dikey) + ok tuşları (yaw/pitch). Null platformda tuş/
            // mouse yok → no-op. Bakış yaw/pitch'ten türetilir.
            {
                const RwReal kDt = 1.0f / 60.0f;
                const RwReal speed = camSpan * 0.10f * kDt;
                const RwReal turn = 1.8f * kDt; // ok tuşu rad/sn
                const RwReal sens = 0.0035f;    // mouse radians/piksel
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
                    camPitch -= (RwReal)mdy * sens; // yukarı sürükle = yukar
                    lookChanged = TRUE;
                }
                const RwReal kMaxPitch = 1.55f; // ~89°
                if (camPitch > kMaxPitch) {
                    camPitch = kMaxPitch;
                }
                if (camPitch < -kMaxPitch) {
                    camPitch = -kMaxPitch;
                }
                // bakış: fwd = (sin(yaw)cos(p), sin(p), cos(yaw)cos(p))
                const RwReal cp = std::cos(camPitch);
                const RwReal fwd[3] = {std::sin(camYaw) * cp,
                                       std::sin(camPitch),
                                       std::cos(camYaw) * cp};
                const RwReal right[3] = {std::cos(camYaw), 0.0f,
                                         -std::sin(camYaw)}; // yaw+90°
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
                pOcean->Render(); // Begin/End arası
            }
        }
        if (RenderPlatform_RenderTestQuad(0.0f, 0.0f, 0.25f, 0.25f,
                                          0xFF40A0FF) != TRUE) {
            std::fprintf(stderr, "CannonCruise: TestQuad %d failed\n",
                         frame);
            break;
        }
        // Dünya karesi yakalama (faz çekimleri tüketmediyse; tek seferlik).
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

    delete pOcean; // COcean::Shutdown + tile temizliği
    RwPlatformSetMouseCapture(FALSE); // imleç geri

    RwsPipeline_Unload(&pipeline);
    RenderPlatform_Terminate();
    RwEngineStop();
    RwEngineClose();
    RwEngineTerm();

    // Dünya yolunda pencere erken kapatma (ShouldClose) başarı sayılır.
    return (framesRendered > 0) ? 0 : 1;
}
