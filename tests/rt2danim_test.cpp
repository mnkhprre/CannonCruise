// ============================================================================
// CannonCruise — Rt2dMaestro .anm stream parser test (rt2danim_test)
//
// Fixtures: MAINMENU.RWS (Menu.anm, 32 düğüm) + MAPMENU.RWS
// (CAG_Menu.anm, 44 düğüm) — bilinen header/node değerleri birebir.
// ============================================================================

#include "Rt2dMaestro.h"
#include "Rt2dMaestroStream.h"
#include "rwcore.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

static int s_checks = 0;
static int s_failures = 0;

#define CHECK(cond)                                                            \
    do {                                                                       \
        ++s_checks;                                                            \
        if (!(cond)) {                                                         \
            ++s_failures;                                                      \
            std::printf("CHECK FAILED %s:%d: %s\n", __FILE__, __LINE__,        \
                        #cond);                                                \
        }                                                                      \
    } while (0)

// RWS içinde rwID_2DMAESTRO taşıyan 0x0716 chunk payload'unu bul
static std::vector<RwUInt8> LoadAnimAsset(const char *pPath)
{
    std::vector<RwUInt8> out;
    FILE *f = std::fopen(pPath, "rb");
    if (f == nullptr) {
        std::printf("open failed: %s\n", pPath);
        return out;
    }
    std::fseek(f, 0, SEEK_END);
    long size = std::ftell(f);
    std::fseek(f, 0, SEEK_SET);
    std::vector<RwUInt8> file((size_t)size);
    if (size > 0 && std::fread(file.data(), 1, (size_t)size, f) != (size_t)size) {
        std::fclose(f);
        return out;
    }
    std::fclose(f);

    // chunk tarama: [u32 type][u32 len][u32 libid] + payload
    static const char kId[] = "rwID_2DMAESTRO";
    size_t off = 0;
    while (off + 12 <= file.size()) {
        RwUInt32 type = 0;
        RwUInt32 len = 0;
        std::memcpy(&type, file.data() + off, 4);
        std::memcpy(&len, file.data() + off + 4, 4);
        if (len > file.size() - off - 12) {
            break;
        }
        if (type == 0x0716) {
            const RwUInt8 *payload = file.data() + off + 12;
            for (RwUInt32 j = 0; j + 14 <= len; ++j) {
                if (std::memcmp(payload + j, kId, 14) == 0) {
                    out.assign(payload, payload + len);
                    return out;
                }
            }
        }
        off += 12 + len;
    }
    return out;
}

static void CheckAnim(const std::vector<RwUInt8> &asset, const char *label,
                      RwUInt32 expectNodes, const char *firstName,
                      const char *buttonName, RwUInt32 expectButtonArg,
                      const char *commandName)
{
    RwUInt32 off = 0;
    RwUInt32 len = 0;
    CHECK(Rt2dMaestro_FindAnimInAsset(asset.data(), (RwUInt32)asset.size(),
                                      &off, &len) == TRUE);
    if (off == 0 || len < 44) {
        std::printf("%s: anim stream bulunamadı\n", label);
        return;
    }
    Rt2dMaestroStreamInfo info;
    CHECK(Rt2dMaestro_Inspect(asset.data() + off, len, &info) == TRUE);
    CHECK(info.canvasWidth == 640.0f);
    CHECK(info.canvasHeight == 480.0f);
    CHECK(info.field2 == 433);
    CHECK(info.nodeCount == expectNodes);
    CHECK(info.sceneNumRecords > 0);
    CHECK(info.sceneSizeA > 0 && info.sceneSizeB > 0);

    // ilk group + isim kontrolleri
    if (info.nodeCount > 0 && firstName != nullptr) {
        RwBool found = FALSE;
        for (RwUInt32 i = 0; i < info.nodeCount; ++i) {
            if (std::strcmp(info.nodes[i].name, firstName) == 0) {
                found = TRUE;
                break;
            }
        }
        CHECK(found == TRUE);
    }
    if (buttonName != nullptr) {
        RwBool found = FALSE;
        for (RwUInt32 i = 0; i < info.nodeCount; ++i) {
            if (std::strcmp(info.nodes[i].name, buttonName) == 0) {
                CHECK(info.nodes[i].type == rwrt2dNODE_BUTTON);
                CHECK(info.nodes[i].arg == expectButtonArg);
                found = TRUE;
                break;
            }
        }
        CHECK(found == TRUE);
    }
    if (commandName != nullptr) {
        RwBool found = FALSE;
        for (RwUInt32 i = 0; i < info.nodeCount; ++i) {
            if (std::strcmp(info.nodes[i].name, commandName) == 0) {
                CHECK(info.nodes[i].type == rwrt2dNODE_COMMAND);
                CHECK(info.nodes[i].arg == 0xFFFFFFFFu);
                found = TRUE;
                break;
            }
        }
        CHECK(found == TRUE);
    }
    std::printf("%s: nodes=%u sceneRecords=%u canvas=%.0fx%.0f\n", label,
                (unsigned)info.nodeCount, (unsigned)info.sceneNumRecords,
                (double)info.canvasWidth, (double)info.canvasHeight);
}

int main(int argc, char **argv)
{
    if (argc < 3) {
        std::printf("usage: rt2danim_test MAINMENU.RWS MAPMENU.RWS\n");
        return 2;
    }

    // Bozuk girdi
    Rt2dMaestroStreamInfo info;
    CHECK(Rt2dMaestro_Inspect(nullptr, 100, &info) == FALSE);
    CHECK(Rt2dMaestro_Inspect("x", 4, &info) == FALSE);

    std::vector<RwUInt8> menu = LoadAnimAsset(argv[1]);
    CHECK(menu.empty() == false);
    if (!menu.empty()) {
        CheckAnim(menu, "Menu.anm", 32, "/MusicSoundWave/", "btnUp", 98,
                  "QuitGame");
    }

    std::vector<RwUInt8> cag = LoadAnimAsset(argv[2]);
    CHECK(cag.empty() == false);
    if (!cag.empty()) {
        CheckAnim(cag, "CAG_Menu.anm", 44, nullptr, "btnUp", 50,
                  "MapMenuEnter");
    }

    // Rt2dMaestro nesne API — engine boot (Im2D render NULL no-op).
    if (RwEngineInit(0, 0, 0) != TRUE || RwEngineOpen() != TRUE ||
        RwEngineStart() != TRUE) {
        std::printf("engine boot failed\n");
        return 2;
    }

    CHECK(Rt2dMaestroCreateFromMemory(nullptr, 100) == nullptr);
    CHECK(Rt2dMaestroCreateFromMemory("x", 4) == nullptr);

    if (!menu.empty()) {
        RwUInt32 off = 0, len = 0;
        CHECK(Rt2dMaestro_FindAnimInAsset(menu.data(), (RwUInt32)menu.size(),
                                          &off, &len) == TRUE);
        Rt2dMaestro *pMaestro = Rt2dMaestroCreateFromMemory(
            menu.data() + off, len);
        CHECK(pMaestro != nullptr);
        if (pMaestro != nullptr) {
            CHECK(pMaestro->commandCount > 0);
            CHECK(pMaestro->selection == 0);
            const char *cmd0 = Rt2dMaestroGetSelectedCommand(pMaestro);
            CHECK(cmd0 != nullptr);
            if (cmd0 != nullptr) {
                std::printf("Menu first command: %s\n", cmd0);
            }
            // seçim yuvarlama: komut sayısı kadar Next → başa sar
            const RwInt32 n = pMaestro->commandCount;
            for (RwInt32 i = 0; i < n; ++i) {
                CHECK(Rt2dMaestroSelectNext(pMaestro) == TRUE);
            }
            CHECK(pMaestro->selection == 0);
            CHECK(Rt2dMaestroSelectPrev(pMaestro) == TRUE);
            CHECK(pMaestro->selection == n - 1);
            // Update biriktirir, Render no-op kare (NULL im2d)
            Rt2dMaestroUpdate(pMaestro, 0.5f);
            CHECK(pMaestro->time > 0.4f);
            Rt2dMaestroRender(pMaestro);
            CHECK(Rt2dMaestroDestroy(pMaestro) == TRUE);
        }
    }

    RwEngineStop();
    RwEngineClose();
    RwEngineTerm();

    std::printf("rt2danim_test: %d checks, %d failures\n", s_checks,
                s_failures);
    return (s_failures == 0) ? 0 : 1;
}
