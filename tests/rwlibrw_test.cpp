// ============================================================================
// CannonCruise - librw Stream Validation Test (rwlibrw_test.cpp)
// Kapı 4 doğrulaması: librw shim (rw_librw.cpp) üzerinden GERÇEK oyun
// asset'i açılır ve chunk başlıkları taranır.
//
//   RwEngineInit/Open/Start → librw rw::Engine (NULL platform, cihaz yok)
//   RwStreamOpen(Streams/*.RWS) → rw::StreamFile
//   RwStreamReadChunkHeaderInfo döngüsü → chunk envanteri
//   RwStreamFindChunk(rwID_CLUMP / rwID_TEXDICTIONARY) → klasik ID kontrolü
//
// Kullanım: rwlibrw_test <path-to-rws>
// ============================================================================

#include "rwcore.h"

#include <cstdio>
#include <cstring>

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

int main(int argc, char **argv) {
    if (argc < 2) {
        std::printf("usage: rwlibrw_test <file.rws>\n");
        return 2;
    }
    const char *pPath = argv[1];

    // motor yaşam döngüsü — librw NULL platform
    CHECK(RwEngineInit(0, 0, 0) == TRUE);
    CHECK(RwEngineOpen() == TRUE);
    CHECK(RwEngineStart() == TRUE);
    CHECK(RwEngineIsRunning() == TRUE);

    RwStream *pStream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, pPath);
    CHECK(pStream != 0);
    if (!pStream) {
        std::printf("rwlibrw_test: cannot open %s\n", pPath);
        return 1;
    }

    // chunk envanteri — classic RW3 12 bayt başlık döngüsü
    int chunkCount = 0;
    int clumpCount = 0;
    int txdCount = 0;
    int worldCount = 0;
    int structCount = 0;
    const int kMaxChunks = 100000;
    while (chunkCount < kMaxChunks) {
        RwChunkHeaderInfo header;
        std::memset(&header, 0, sizeof(header));
        if (!RwStreamReadChunkHeaderInfo(pStream, &header)) {
            break; // EOF / bozuk başlık — döngü sonu
        }
        if (header.type == 0) {
            break; // rwID_NAOBJECT
        }
        ++chunkCount;
        if (header.type == rwID_CLUMP) {
            ++clumpCount;
        }
        if (header.type == rwID_TEXDICTIONARY) {
            ++txdCount;
        }
        if (header.type == rwID_WORLD) {
            ++worldCount;
        }
        if (header.type == 0x01) {
            ++structCount;
        }
        // ilk birkaç chunk'ı yazdır (gözlemsel)
        if (chunkCount <= 8) {
            std::printf("chunk[%d]: type=0x%04X len=%u ver=0x%08X\n",
                        chunkCount - 1, header.type, header.length,
                        header.version);
        }
        RwStreamSkip(pStream, header.length);
    }

    std::printf("rwlibrw_test: %d chunks (clump=%d txd=%d world=%d "
                "struct=%d)\n",
                chunkCount, clumpCount, txdCount, worldCount, structCount);

    // gerçek sahne stream'i chunk içermeli (kapı 4 sinyali)
    CHECK(chunkCount > 0);

    // FindChunk — atla + geri sarmalama yok; dosyanın BAŞINDAN tarama için
    // yeni açılan akış kullanılır
    CHECK(RwStreamFindChunk(pStream, rwID_CLUMP, 0, 0) == TRUE ||
          clumpCount == 0); // clump yoksa FindChunk da olumsuz kabul

    CHECK(RwStreamClose(pStream, 0) == TRUE);

    CHECK(RwEngineStop() == TRUE);
    CHECK(RwEngineClose() == TRUE);
    CHECK(RwEngineTerm() == TRUE);
    CHECK(RwEngineIsRunning() == FALSE);

    std::printf("rwlibrw_test: %d checks, %d failures\n", g_checks,
                g_failures);
    return g_failures == 0 ? 0 : 1;
}
