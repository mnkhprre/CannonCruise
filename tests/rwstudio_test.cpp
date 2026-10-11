// ============================================================================
// CannonCruise - rwstudio Konteyner Test (rwstudio_test.cpp)
// Kapı 4: gerçek Streams/*.RWS dosyalarından rwstudio konteyneri parse
// edilir (Faz 4.6e). Beklenenler hexdump forensics ile sabitlendi:
//   - 0x0716 asset kayıtları: meta + gömülü RW chunk (txd: type 0x23)
//   - 0x071C manifest: sınıf adları (CWb* behaviours)
//   - 0x0704 nesne kayıtları
//   - wbID_STATICWORLDCOLLISION: CPhysicsResource::Load hedefi
//
// Kullanım: rwstudio_test <mainmenu.rws> <global.rws> <viking01.rws>
// ============================================================================

#include "RwStudioStream.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

static float AsFloat(const RwUInt8 *pValue) {
    float f = 0.0f;
    std::memcpy(&f, pValue, 4);
    return f;
}

static int g_checks = 0;
static int g_failures = 0;

static void Check(bool cond, const char *pExpr, int line) {
    ++g_checks;
    if (!cond) {
        ++g_failures;
        std::printf("FAIL line %d: %s\n", line, pExpr);
    }
}

#define CHECK(expr) Check((expr), #expr, __LINE__)

struct Counts {
    int asset = 0;
    int object = 0;
    int manifest = 0;
    int txdEmbed = 0;
    int colAsset = 0; // wbID_STATICWORLDCOLLISION
    int dffAsset = 0; // wbID_WORLDGEOMETRY
};

static Counts Scan(const char *pPath, bool verboseTxd) {
    Counts c;
    RwStudioFile *pFile = RwStudioFileOpen(pPath);
    if (!pFile) {
        std::printf("FAIL open %s\n", pPath);
        ++g_failures;
        return c;
    }
    RwStudioAsset asset;
    int idx = 0;
    const bool verbose = std::getenv("RWSTUDIO_VERBOSE") != 0;
    while (RwStudioFileNextAsset(pFile, &asset)) {
        if (verbose) {
            std::printf("  [%d] type=0x%04X len=%u emb=0x%X '%s' '%s'\n",
                        idx, asset.chunkType, asset.chunkLength,
                        asset.embeddedType, asset.typeName, asset.rwIdName);
        }
        ++idx;
        if (asset.chunkType == rwstudioID_ASSETRECORD) {
            ++c.asset;
            if (asset.embeddedType == rwstudioEMBED_TEXDICT) {
                ++c.txdEmbed;
                if (verboseTxd) {
                    std::printf("txd: %s | %s | embLen=%u libid=0x%08X\n",
                                asset.typeName, asset.rwIdName,
                                asset.embeddedLength, asset.embeddedLibId);
                    // gömülü payload: i16 dokuSayısı + i16 device (d3d8=1)
                    RwUInt8 buf[8];
                    RwUInt32 got = RwStudioAssetReadEmbedded(
                        pFile, &asset, buf, sizeof(buf));
                    CHECK(got == 8);
                    int numTex = buf[0] | (buf[1] << 8);
                    int device = buf[2] | (buf[3] << 8);
                    CHECK(numTex >= 1 && numTex <= 64);
                    CHECK(device == 1); // d3d8
                    std::printf("  payload: numTex=%d device=%d\n", numTex,
                                device);
                } else {
                    RwUInt8 buf[8];
                    RwStudioAssetReadEmbedded(pFile, &asset, buf,
                                              sizeof(buf));
                }
            } else if (std::strcmp(asset.rwIdName,
                                   "wbID_STATICWORLDCOLLISION") == 0) {
                ++c.colAsset;
            } else if (std::strcmp(asset.rwIdName, "wbID_WORLDGEOMETRY") ==
                       0) {
                ++c.dffAsset;
            }
        } else if (asset.chunkType == rwstudioID_MANIFEST) {
            ++c.manifest;
        } else if (asset.chunkType == rwstudioID_OBJECTRECORD) {
            ++c.object;
        }
    }
    RwStudioFileClose(pFile);
    return c;
}

int main(int argc, char **argv) {
    if (argc < 4) {
        std::printf("usage: rwstudio_test <mainmenu> <global> <viking01>\n");
        return 2;
    }
    // librw motoru — RwStream dosya açılışı için şart (NULL platform)
    CHECK(RwEngineInit(0, 0, 0) == TRUE);
    CHECK(RwEngineOpen() == TRUE);
    CHECK(RwEngineStart() == TRUE);

    // ---- MAINMENU: detay ----
    {
        RwStudioFile *pFile = RwStudioFileOpen(argv[1]);
        CHECK(pFile != 0);
        if (pFile) {
            RwStudioAsset asset;
            CHECK(RwStudioFileNextAsset(pFile, &asset) == TRUE);
            CHECK(asset.chunkType == rwstudioID_ASSETRECORD);
            CHECK(std::strcmp(asset.typeName, "Texture Dictionary") == 0);
            CHECK(std::strcmp(asset.rwIdName, "rwID_TEXDICTIONARY") == 0);
            CHECK(asset.embeddedType == rwstudioEMBED_TEXDICT);
            CHECK(asset.embeddedLibId == 0x1803FFFF); // RW 3.6.0.3
            CHECK(asset.embeddedLength > 100000);
            CHECK(std::strstr(asset.sourcePath, ".txd") != 0);
            // gömülü payload okuması (akış hizası korunur)
            RwUInt8 buf[8];
            RwUInt32 got =
                RwStudioAssetReadEmbedded(pFile, &asset, buf, sizeof(buf));
            CHECK(got == 8);
            int numTex = buf[0] | (buf[1] << 8);
            int device = buf[2] | (buf[3] << 8);
            CHECK(numTex >= 1 && numTex <= 64);
            CHECK(device == 1);

            // manifest bul ve sınıf listesini doğrula
            bool foundManifest = false;
            while (RwStudioFileNextAsset(pFile, &asset)) {
                if (asset.chunkType == rwstudioID_MANIFEST) {
                    foundManifest = true;
                    break;
                }
            }
            CHECK(foundManifest);
            CHECK(RwStudioFileGetClassCount(pFile) == 4);
            CHECK(std::strcmp(RwStudioFileGetClassName(pFile, 0),
                              "CWbRenderBehavior") == 0);
            CHECK(std::strcmp(RwStudioFileGetClassName(pFile, 3),
                              "CMenuBehavior") == 0);
            RwStudioFileClose(pFile);
        }
    }

    // ---- nesne şeması (0x0704 — Faz 4.6f): MAINMENU obj0/1/2 ----
    {
        RwStudioFile *pFile = RwStudioFileOpen(argv[1]);
        CHECK(pFile != 0);
        if (pFile) {
            RwStudioAsset asset;
            int objSeen = 0;
            while (RwStudioFileNextAsset(pFile, &asset)) {
                if (asset.chunkType != rwstudioID_OBJECTRECORD) {
                    continue;
                }
                const RwStudioObject *pObj = RwStudioFileGetObject(pFile);
                CHECK(pObj != 0);
                if (!pObj) {
                    break;
                }
                if (objSeen == 0) {
                    // CWbRenderBehavior: 13 property (id 0..0x0C)
                    CHECK(pObj->behaviourCount == 2);
                    CHECK(std::strcmp(pObj->behaviours[0].className,
                                      "CWbRenderBehavior") == 0);
                    CHECK(pObj->behaviours[0].propCount == 13);
                    CHECK(pObj->behaviours[0].props[0].id == 0);
                    CHECK(pObj->behaviours[0].props[0].value[0] == 0xFF &&
                          pObj->behaviours[0].props[0].value[3] == 0xFF);
                    CHECK(AsFloat(pObj->behaviours[0].props[1].value) ==
                          1000.0f);
                    CHECK(AsFloat(pObj->behaviours[0].props[2].value) ==
                          0.5f);
                    CHECK(AsFloat(pObj->behaviours[0].props[12].value) ==
                          30.0f);
                    CHECK(std::strcmp(pObj->behaviours[1].className,
                                      "CAttributeHandler") == 0);
                    CHECK(pObj->behaviours[1].propCount == 1);
                } else if (objSeen == 1) {
                    // CWbCameraBehavior: vektör property (size 20) + 60.0
                    CHECK(pObj->behaviourCount >= 1);
                    CHECK(std::strcmp(pObj->behaviours[0].className,
                                      "CWbCameraBehavior") == 0);
                    bool foundFov = false;
                    for (RwUInt32 i = 0; i < pObj->behaviours[0].propCount;
                         ++i) {
                        if (pObj->behaviours[0].props[i].id == 4 &&
                            AsFloat(pObj->behaviours[0].props[i].value) ==
                                60.0f) {
                            foundFov = true;
                        }
                    }
                    CHECK(foundFov);
                } else if (objSeen == 2) {
                    // CAudioStreamEntity: string segment + 5 property
                    CHECK(pObj->behaviourCount >= 1);
                    CHECK(std::strcmp(pObj->behaviours[0].className,
                                      "CAudioStreamEntity") == 0);
                    CHECK(std::strcmp(pObj->behaviours[0].name,
                                      "Audio_MenuMusic") == 0);
                    CHECK(pObj->behaviours[0].propCount == 5);
                    CHECK(AsFloat(pObj->behaviours[0].props[3].value) ==
                          5.0f);
                }
                ++objSeen;
                if (objSeen >= 3) {
                    break;
                }
            }
            CHECK(objSeen == 3);
            RwStudioFileClose(pFile);
        }
    }

    // ---- envanterler ----
    Counts mainC = Scan(argv[1], false);
    std::printf("MAINMENU: asset=%d object=%d manifest=%d txd=%d col=%d dff=%d\n",
                mainC.asset, mainC.object, mainC.manifest, mainC.txdEmbed,
                mainC.colAsset, mainC.dffAsset);
    CHECK(mainC.asset == 5);      // hexdump: 5×0x0716
    CHECK(mainC.manifest == 1);   // 1×0x071C
    CHECK(mainC.object == 4);     // 4×0x0704
    CHECK(mainC.txdEmbed == 1);
    CHECK(mainC.colAsset == 1);   // StaticCollisionData
    CHECK(mainC.dffAsset == 1);   // WorldGeometry

    Counts globalC = Scan(argv[2], false);
    std::printf("GLOBAL: asset=%d object=%d manifest=%d txd=%d\n",
                globalC.asset, globalC.object, globalC.manifest,
                globalC.txdEmbed);
    // GLOBAL: 1 gömülü txd (525KB menü/font sözlüğü) + .dff/.col/.path/
    // .texref/PPB — 7 asset toplam (chunkinv envanteriyle doğrulandı)
    CHECK(globalC.asset == 7);
    CHECK(globalC.txdEmbed == 1);
    CHECK(globalC.manifest == 1);
    CHECK(globalC.object == 48);

    Counts vikingC = Scan(argv[3], false);
    std::printf("VIKING01: asset=%d object=%d manifest=%d txd=%d\n",
                vikingC.asset, vikingC.object, vikingC.manifest,
                vikingC.txdEmbed);
    CHECK(vikingC.asset == 119);
    CHECK(vikingC.object == 497);
    CHECK(vikingC.manifest == 1);

    // VIKING01 ilk nesne sağlamlığı: behavior sınıf adları parse edilebiliyor
    {
        RwStudioFile *pFile = RwStudioFileOpen(argv[3]);
        CHECK(pFile != 0);
        if (pFile) {
            RwStudioAsset asset;
            bool sawObject = false;
            while (RwStudioFileNextAsset(pFile, &asset)) {
                if (asset.chunkType != rwstudioID_OBJECTRECORD) {
                    continue;
                }
                const RwStudioObject *pObj = RwStudioFileGetObject(pFile);
                CHECK(pObj != 0);
                if (pObj) {
                    CHECK(pObj->behaviourCount >= 1);
                    CHECK(pObj->behaviours[0].className[0] != '\0');
                }
                sawObject = true;
                break;
            }
            CHECK(sawObject);
            RwStudioFileClose(pFile);
        }
    }

    // Faz 4.6p — VIKING01 obj[264] dünya kamerası: çok davranışlı kayıt
    // (CWbCameraBehavior + CSystemCommands + CWbCameraBehavior) + flags=0
    // 16B blob segmentleri (GUID) sonrası 32 property; id0 pos, id4 FOV.
    {
        RwStudioFile *pFile = RwStudioFileOpen(argv[3]);
        CHECK(pFile != 0);
        if (pFile) {
            RwStudioAsset asset;
            bool sawCam = false;
            bool sawSystemCommands = false;
            while (RwStudioFileNextAsset(pFile, &asset)) {
                if (asset.chunkType != rwstudioID_OBJECTRECORD) {
                    continue;
                }
                const RwStudioObject *pObj = RwStudioFileGetObject(pFile);
                if (!pObj) {
                    continue;
                }
                bool hasCamProps = false;
                for (RwUInt32 i = 0; i < pObj->behaviourCount; ++i) {
                    const RwStudioBehaviour *pBeh = &pObj->behaviours[i];
                    if (std::strcmp(pBeh->className, "CSystemCommands") ==
                        0) {
                        sawSystemCommands = true;
                    }
                    if (std::strcmp(pBeh->className, "CWbCameraBehavior") ==
                            0 &&
                        pBeh->propCount == 32) {
                        hasCamProps = true;
                        // id0 VEC3 pos = (0.35, 7.0, 0.0) — dünya kamerası
                        const RwStudioProperty *pProp0 = 0;
                        const RwStudioProperty *pProp4 = 0;
                        for (RwUInt32 j = 0; j < pBeh->propCount; ++j) {
                            if (pBeh->props[j].id == 0x0) {
                                pProp0 = &pBeh->props[j];
                            } else if (pBeh->props[j].id == 0x4) {
                                pProp4 = &pBeh->props[j];
                            }
                        }
                        CHECK(pProp0 != 0);
                        if (pProp0) {
                            CHECK(pProp0->size == 20);
                            float v[3] = {0.0f, 0.0f, 0.0f};
                            std::memcpy(v, pProp0->value, 12);
                            CHECK(v[0] == 0.35f);
                            CHECK(v[1] == 7.0f);
                            CHECK(v[2] == 0.0f);
                        }
                        CHECK(pProp4 != 0);
                        if (pProp4) {
                            float fov = 0.0f;
                            std::memcpy(&fov, pProp4->value, 4);
                            CHECK(fov == 40.0f);
                        }
                    }
                }
                if (hasCamProps) {
                    sawCam = true;
                    // çok davranışlı: ortada CSystemCommands kaydı
                    CHECK(pObj->behaviourCount >= 3);
                    break;
                }
            }
            CHECK(sawCam);
            CHECK(sawSystemCommands);
            RwStudioFileClose(pFile);
        }
    }

    CHECK(RwEngineStop() == TRUE);
    CHECK(RwEngineClose() == TRUE);
    CHECK(RwEngineTerm() == TRUE);

    std::printf("rwstudio_test: %d checks, %d failures\n", g_checks,
                g_failures);
    return g_failures == 0 ? 0 : 1;
}
