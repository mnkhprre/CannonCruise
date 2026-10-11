// SDK-independent regression test for the restored font metrics system.
// Covers: FontSystemInit quad index table, METRICS1 parsing (UV/aspect/size),
// ASCII + extended glyph lookup, ';' fallback list, CRLF tolerance,
// texture-required failure path, Get() cache, teardown lifecycle,
// METRICS3 vector contours (parse + UTF-16) and sub_4D54C0 stroke raster.
// Binary refs: FUN_004cd660 / FUN_004cd9a0 / FUN_004cdbc0 / FUN_004ce000 /
//              FUN_004cee90 / FUN_004cd580 / FUN_004cef90 / FUN_004ce870 /
//              FUN_004d54c0
#include "FontSystem.h"

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#if defined(_WIN32)
#include <direct.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif

#define CHECK(condition)                                                     \
    do {                                                                     \
        if (!(condition)) {                                                  \
            std::fprintf(stderr, "FAIL at line %d: %s\n", __LINE__,          \
                         #condition);                                        \
            return 1;                                                        \
        }                                                                    \
    } while (0)

// rwstubs.cpp çizim sayaçları — DrawText batch flush doğrulaması
RwUInt32 RwStub_GetIm2DRenderCount(void);
RwUInt32 RwStub_GetIm2DIndexedCount(void);
void RwStub_ResetIm2DRenderCount(void);

namespace
{
    // Sahte doku okuyucu — binary doku raster boyutlarını UV için okur
    // (sub_5154D0); testler boyutları doğrudan verir. DrawText
    // RwTextureGetRaster çağırdığı için gerçek (ama boş) bir RwTexture
    // tahsis edilir — sabit sahte adres dereference edilemezdi.
    bool g_texFail = false;
    int g_texCalls = 0;
    char g_lastName[128] = {0};
    char g_lastMask[128] = {0};
    bool g_lastMaskNull = false;
    RwTexture *g_lastTex = 0;

    RwTexture *FakeTextureReader(const char *pName, const char *pMask,
                                 RwInt32 *pWidth, RwInt32 *pHeight) {
        ++g_texCalls;
        std::snprintf(g_lastName, sizeof(g_lastName), "%s", pName ? pName : "");
        g_lastMaskNull = (pMask == 0);
        std::snprintf(g_lastMask, sizeof(g_lastMask), "%s", pMask ? pMask : "");
        if (g_texFail) {
            *pWidth = 0;
            *pHeight = 0;
            return 0;
        }
        *pWidth = 256;
        *pHeight = 128;
        g_lastTex = reinterpret_cast<RwTexture *>(std::calloc(1, 128));
        return g_lastTex;
    }

    bool Near(float a, float b) { return std::fabs(a - b) < 1e-5f; }

    bool WriteFile(const char *pPath, const char *pContents) {
        std::FILE *pFile = std::fopen(pPath, "wb");
        if (!pFile) {
            return false;
        }
        std::fwrite(pContents, 1, std::strlen(pContents), pFile);
        std::fclose(pFile);
        return true;
    }
} // namespace

int main() {
#if defined(_WIN32)
    _mkdir("fonts");
#else
    mkdir("fonts", 0755);
#endif

    // Test .met dosyaları
    CHECK(WriteFile("fonts/CCF_T1.met",
                    "METRICS1\r\n" // CRLF — StripNewline doğrulaması
                    "CCF_T1\r\n"
                    "16\r\n"
                    "65 0 0 8 16\r\n"    // 'A'
                    "66 8 0 16 16\r\n"   // 'B'
                    "178 16 0 24 16\r\n" // 0xB2 — genişletilmiş
                    "400 0 16 8 32\r\n")); // 0x190 — genişletilmiş (son satır)
    CHECK(WriteFile("fonts/CCF_T2.met",
                    "METRICS1\nCCF_T2\n8\n65 0 0 4 8\n"));
    CHECK(WriteFile("fonts/CCF_T3.met",
                    "METRICS1\nCCF_T3 mymask\n8\n65 0 0 4 8\n"));
    CHECK(WriteFile("fonts/CCF_T4.met",
                    "METRICS1\nCCF_T4\n8\n65 0 0 4 8\n"));
    CHECK(WriteFile("fonts/mCCF_T4", "")); // maske yoklama dosyası
    CHECK(WriteFile("fonts/CCF_M2.met", "METRICS2\nCCF_M2\n8\n"));
    // METRICS3 — vektör konturlar + UTF-8 glif kodu ('é' = U+00E9 →
    // geniş tablo + UTF-16LE modu). Biçim: '<önek><UTF-8 kod>' kod satırı
    // (binary sub_4CE870 buf[1]'den okur — önek yok sayılır).
    CHECK(WriteFile("fonts/CCF_V3.met",
                    "METRICS3\n"
                    "CCF_V3\n"
                    "'A\n"
                    "begin\n"
                    "moveto 0 0\n"
                    "lineto 10 0\n"
                    "lineto 10 12\n"
                    "closepath\n"
                    "end\n"
                    "'\xC3\xA9\n"
                    "begin\n"
                    "moveto 0 0\n"
                    "curveto 2 0 4 6 6 6\n"
                    "lineto 7 6\n"
                    "end\n"));

    FontSystem_SetTextureReader(&FakeTextureReader);

    // ------------------------------------------------------------------
    // init — sub_4CD660 çeyrek-dörtgen tablosu
    // ------------------------------------------------------------------
    CHECK(FontSystemAllocBuffers()); // sub_4CEFD0
    CHECK(FontSystemInit());         // sub_4CD660

    const RwUInt16(*quad)[6] = FontSystem_GetQuadIndices();
    CHECK(quad[0][0] == 0 && quad[0][1] == 2 && quad[0][2] == 3);
    CHECK(quad[0][3] == 0 && quad[0][4] == 3 && quad[0][5] == 1);
    CHECK(quad[1][0] == 4 && quad[1][1] == 6 && quad[1][2] == 7);
    CHECK(quad[1][5] == 5);
    CHECK(quad[63][0] == 252 && quad[63][2] == 255 && quad[63][5] == 253);

    // ------------------------------------------------------------------
    // METRICS1 ayrıştırma — sub_4CDBC0
    // ------------------------------------------------------------------
    g_texCalls = 0;
    CCFont *pFont = FontSystemLoad("CCF_T1");
    CHECK(pFont != 0);
    CHECK(g_texCalls == 1);
    CHECK(std::strcmp(g_lastName, "CCF_T1") == 0);
    CHECK(g_lastMaskNull); // tek token, maske yoklama dosyası henüz yok
    CHECK(pFont->glyphCount == 4);
    CHECK(pFont->textures[0] == g_lastTex);
    CHECK(Near(pFont->lineExtent, 16.0f)); // son satır: alt-üst = 32-16
    CHECK(Near(pFont->sizeScale, 1.0f));   // boyut 16 / 16

    // UV: (piksel + 0.5) / doku boyutu (256×128)
    const CCFontGlyph *pGlyph = FontSystemGetGlyph(pFont, 65); // 'A'
    CHECK(pGlyph != 0);
    CHECK(Near(pGlyph->u0, 0.5f / 256.0f));
    CHECK(Near(pGlyph->v0, 0.5f / 128.0f));
    CHECK(Near(pGlyph->u1, 8.5f / 256.0f));
    CHECK(Near(pGlyph->v1, 16.5f / 128.0f));
    CHECK(Near(pGlyph->aspect, 0.5f)); // (8-0)/(16-0)
    CHECK(pGlyph->texture == g_lastTex);

    // ASCII doğrudan harita: 'B' → glif 1; dosyada olmayan 'Z' → NULL
    pGlyph = FontSystemGetGlyph(pFont, 66);
    CHECK(pGlyph != 0 && Near(pGlyph->u0, 8.5f / 256.0f));
    CHECK(FontSystemGetGlyph(pFont, 90) == 0);
    CHECK(pFont->ascii[65] == 0);
    CHECK(pFont->ascii[90] == 0xFFFF);

    // Genişletilmiş harita — sub_4CE000: min=178, aralık=400-178+1
    CHECK(pFont->extMin == 178);
    CHECK(pFont->extCount == 400 - 178 + 1);
    CHECK(pFont->extTable != 0);
    pGlyph = FontSystemGetGlyph(pFont, 178);
    CHECK(pGlyph != 0 && pFont->extTable[0] == 2);
    pGlyph = FontSystemGetGlyph(pFont, 400);
    CHECK(pGlyph != 0 && pFont->extTable[400 - 178] == 3);
    CHECK(FontSystemGetGlyph(pFont, 177) == 0); // aralık altı
    CHECK(FontSystemGetGlyph(pFont, 500) == 0); // aralık üstü
    CHECK((pFont->glyphFlags & 2) != 0);        // [esi+0x10] |= 2

    // ------------------------------------------------------------------
    // iki token başlık — (ad, maske) doğrudan sub_5154D0'ye gider
    // ------------------------------------------------------------------
    CCFont *pFont3 = FontSystemLoad("CCF_T3");
    CHECK(pFont3 != 0);
    CHECK(std::strcmp(g_lastName, "CCF_T3") == 0);
    CHECK(!g_lastMaskNull && std::strcmp(g_lastMask, "mymask") == 0);

    // tek token + varlık dosyası → maske "m<ad>" (binary: "%sm%s")
    CCFont *pFont4 = FontSystemLoad("CCF_T4");
    CHECK(pFont4 != 0);
    CHECK(!g_lastMaskNull && std::strcmp(g_lastMask, "mCCF_T4") == 0);

    // ------------------------------------------------------------------
    // ';' yedek zinciri — sub_4CD9A0 aday döngüsü
    // ------------------------------------------------------------------
    CCFont *pFontFb = FontSystemLoad("CCF_NOPE;CCF_T2");
    CHECK(pFontFb != 0);
    CHECK(pFontFb->glyphCount == 1);
    CHECK(FontSystemLoad("CCF_NOPE") == 0);

    // ------------------------------------------------------------------
    // başarısızlıklar: METRICS2 (restor edilmedi) + doku yok
    // ------------------------------------------------------------------
    CHECK(FontSystemLoad("CCF_M2") == 0);
    g_texFail = true;
    CHECK(FontSystemLoad("CCF_T1") == 0);
    g_texFail = false;

    FontSystem_SetTextureReader(0); // varsayılan rota: RwTextureRead → NULL
    CHECK(FontSystemLoad("CCF_T1") == 0);
    FontSystem_SetTextureReader(&FakeTextureReader);

    // ------------------------------------------------------------------
    // ad önbelleği — sub_4CEE90
    // ------------------------------------------------------------------
    CCFontNode *pNodeA = FontSystemGet("CCF_T1");
    CHECK(pNodeA != 0 && pNodeA->pFont != 0);
    CHECK(pNodeA->pName != 0 && std::strcmp(pNodeA->pName, "CCF_T1") == 0);
    CCFontNode *pNodeB = FontSystemGet("CCF_T1");
    CHECK(pNodeA == pNodeB); // önbellek isabeti — yeniden yükleme yok
    CHECK(FontSystemGet("CCF_YOK") == 0);

    // ------------------------------------------------------------------
    // ölçüm — sub_4CD7E0 (FontSystem_MeasureText)
    // CCF_T1: 'A'/'B' aspect 0.5, spacing 0; 0xB2/0x190 geniş glifleri
    // nedeniyle glyphFlags&2 → metinler UTF-16LE (binary sözleşmesi).
    // ------------------------------------------------------------------
    const char wT1AB[] = {'A', 0, 'B', 0, 0, 0};
    const char wT1Z[] = {'Z', 0, 0, 0};
    const char wT1AZA[] = {'A', 0, 'Z', 0, 'A', 0, 0, 0};
    const char wT1Empty[] = {0, 0};
    const char wT1A[] = {'A', 0, 0, 0};
    CHECK(Near(FontSystem_MeasureText(pFont, wT1AB, 1.0f), 1.0f));
    CHECK(Near(FontSystem_MeasureText(pFont, wT1AB, 16.0f), 16.0f));
    CHECK(Near(FontSystem_MeasureText(pFont, wT1Z, 16.0f), 0.0f)); // yok sayılan
    CHECK(Near(FontSystem_MeasureText(pFont, wT1AZA, 16.0f), 16.0f));
    CHECK(Near(FontSystem_MeasureText(pFont, wT1Empty, 16.0f), 0.0f));
    CHECK(Near(FontSystem_MeasureText(0, wT1A, 16.0f), 0.0f));

    // spacing (font+0x0C) ilerlemeye eklenir — binary fadd [esi+0Ch]
    pFont->spacing = 0.25f;
    CHECK(Near(FontSystem_MeasureText(pFont, wT1AB, 1.0f), 1.5f));
    pFont->spacing = 0.0f;

    // ------------------------------------------------------------------
    // çizim — sub_4CCC40 (FontSystem_DrawText): düz materyal yolu
    // ------------------------------------------------------------------
    RwStub_ResetIm2DRenderCount();

    CCFontMaterial mat;
    std::memset(&mat, 0, sizeof(mat));
    mat.red = 200; mat.green = 100; mat.blue = 50; mat.alpha = 255;
    mat.flags = 0; // düz renk yolu

    RwReal posX = 100.0f;
    RwReal posY = 200.0f;
    RwReal advanced = FontSystem_DrawText(pFont, wT1AB, 16.0f, &posX, &posY,
                                          &mat);
    CHECK(Near(advanced, 16.0f));
    CHECK(Near(posX, 116.0f)); // binary: *pPos[0] += pen × scale
    CHECK(Near(posY, 200.0f));
    CHECK(RwStub_GetIm2DIndexedCount() == 1); // tek doku → tek flush

    RwInt32 vertCount = 0;
    const RwIm2DVertex *pVerts = FontSystem_GetDrawVertices(&vertCount);
    CHECK(pVerts != 0);
    CHECK(vertCount == 8); // 2 çeyrek-dörtgen × 4 vertex

    // 'A' quad'ı — birim uzay y ∈ {0,1} × ölçek; aspect 0.5 × 16 = 8 px
    CHECK(Near(pVerts[0].x, 100.0f)); // V0 sol-üst (y=0)
    CHECK(Near(pVerts[0].y, 200.0f));
    CHECK(Near(pVerts[0].u, 0.5f / 256.0f));   // u0
    CHECK(Near(pVerts[0].v, 16.5f / 128.0f));  // v1 (alt UV — binary sırası)
    CHECK(Near(pVerts[1].x, 100.0f)); // V1 sol-alt (y=1)
    CHECK(Near(pVerts[1].y, 216.0f));
    CHECK(Near(pVerts[1].v, 0.5f / 128.0f));   // v0 (üst UV)
    CHECK(Near(pVerts[2].x, 108.0f)); // V2 sağ-üst — pen + aspect×scale
    CHECK(Near(pVerts[2].u, 8.5f / 256.0f));  // u1
    CHECK(Near(pVerts[3].x, 108.0f));
    CHECK(Near(pVerts[3].y, 216.0f));

    // düz renk — materyal baytları
    CHECK(pVerts[0].color.red == 200 && pVerts[0].color.green == 100 &&
          pVerts[0].color.blue == 50 && pVerts[0].color.alpha == 255);
    CHECK(pVerts[3].color.red == 200);

    // 'B' quad'ı — pen ilerlemesi (aspect 0.5 × 16 = 8)
    CHECK(Near(pVerts[4].x, 108.0f));
    CHECK(Near(pVerts[6].x, 116.0f));

    // ------------------------------------------------------------------
    // çizim — materyal NULL → düz beyaz (port varsayılanı)
    // ------------------------------------------------------------------
    posX = 0.0f; posY = 0.0f;
    advanced = FontSystem_DrawText(pFont, wT1A, 1.0f, &posX, &posY, 0);
    CHECK(Near(advanced, 0.5f));
    pVerts = FontSystem_GetDrawVertices(&vertCount);
    CHECK(vertCount == 4);
    CHECK(pVerts[0].color.red == 255 && pVerts[0].color.alpha == 255);

    // ------------------------------------------------------------------
    // çizim — aydınlatma/degrade yolu (materyal +0x64 & 1)
    // genlik: invW = 1/measure(text,1.0) = 1/1.0 = 1
    // 'A': gradL = 0, gradR = 0.5 → V0/V1 = ambient, V2/V3 = ambient+light×0.5
    // ------------------------------------------------------------------
    CCFontMaterial litMat;
    std::memset(&litMat, 0, sizeof(litMat));
    litMat.topAmbient[0] = 10.0f;    // +0x00
    litMat.topAmbient[1] = 20.0f;
    litMat.topAmbient[2] = 30.0f;
    litMat.topAmbient[3] = 40.0f;
    litMat.topLight[0] = 100.0f;     // +0x18
    litMat.bottomAmbient[0] = 1.0f;  // +0x30
    litMat.bottomAmbient[1] = 2.0f;
    litMat.bottomAmbient[2] = 3.0f;
    litMat.bottomAmbient[3] = 4.0f;
    litMat.bottomLight[0] = 200.0f;  // +0x48
    litMat.flags = 1;

    posX = 0.0f; posY = 0.0f;
    advanced = FontSystem_DrawText(pFont, wT1AB, 1.0f, &posX, &posY, &litMat);
    CHECK(Near(advanced, 1.0f));
    pVerts = FontSystem_GetDrawVertices(&vertCount);
    CHECK(vertCount == 8);
    // V0 (taban, gradL=0): ambient (1,2,3,4)
    CHECK(pVerts[0].color.red == 1 && pVerts[0].color.green == 2 &&
          pVerts[0].color.blue == 3 && pVerts[0].color.alpha == 4);
    // V1 (tepe, gradL=0): topAmbient (10,20,30,40)
    CHECK(pVerts[1].color.red == 10 && pVerts[1].color.green == 20 &&
          pVerts[1].color.blue == 30 && pVerts[1].color.alpha == 40);
    // V2 (taban, gradR=0.5): 1 + 200×0.5 = 101
    CHECK(pVerts[2].color.red == 101 && pVerts[2].color.green == 2);
    // V3 (tepe, gradR=0.5): 10 + 100×0.5 = 60
    CHECK(pVerts[3].color.red == 60 && pVerts[3].color.green == 20);
    // 'B' V0 (taban, gradL=0.5): 1 + 200×0.5 = 101
    CHECK(pVerts[4].color.red == 101);

    // ------------------------------------------------------------------
    // batch taşması — binary flush eşiği 63 çeyrek-dörtgen (ebx > 0x17A)
    // 70 × 'A' → 63 + 7 = 2 flush (CCF_T1 geniş mod → UTF-16LE dize)
    // ------------------------------------------------------------------
    char longText[142]; // 70 × ('A',0) + 00 00 sonlandırıcı
    for (int i = 0; i < 70; ++i) {
        longText[i * 2] = 'A';
        longText[i * 2 + 1] = 0;
    }
    longText[140] = 0;
    longText[141] = 0;
    RwStub_ResetIm2DRenderCount();
    posX = 0.0f; posY = 0.0f;
    advanced = FontSystem_DrawText(pFont, longText, 1.0f, &posX, &posY, &mat);
    CHECK(Near(advanced, 70.0f * 0.5f));
    CHECK(RwStub_GetIm2DIndexedCount() == 2); // 63 + 7 quad
    pVerts = FontSystem_GetDrawVertices(&vertCount);
    CHECK(vertCount == 7 * 4); // son batch

    // ------------------------------------------------------------------
    // METRICS3 vektör konturları — sub_4CE870 + UTF-16LE — glyphFlags&2
    // ------------------------------------------------------------------
    CCFont *pVec = FontSystemLoad("CCF_V3");
    CHECK(pVec != 0);
    CHECK(pVec->glyphCount == 2);
    CHECK(pVec->textures[0] == 0);   // vektör fontu — doku yok
    CHECK((pVec->glyphFlags & 2) != 0); // U+00E9 → geniş tablo + UTF-16
    CHECK(Near(pVec->spacing, 0.0f));
    CHECK(Near(pVec->lineExtent, 1.0f)); // M3 lineExtent'i güncellemez

    const CCFontGlyph *pGlyphA = FontSystemGetGlyph(pVec, 'A');
    CHECK(pGlyphA != 0 && pGlyphA->texture == 0 && pGlyphA->data != 0);
    CHECK(Near(pGlyphA->aspect, 10.0f)); // son lineto x (var_A0)
    const CCFontContour *pContA =
        static_cast<const CCFontContour *>(pGlyphA->data);
    CHECK(pContA->pointCount == 3); // moveto + 2×lineto
    CHECK(pContA->closed);          // closepath (sub_4D0220)
    CHECK(pContA->pNext == 0);
    CHECK(pContA->points[0].type == 0); // moveto
    CHECK(pContA->points[1].type == 1 && Near(pContA->points[1].x, 10.0f));
    CHECK(pContA->points[2].type == 1 && Near(pContA->points[2].y, 12.0f));

    const CCFontGlyph *pGlyphE9 = FontSystemGetGlyph(pVec, 0xE9);
    CHECK(pGlyphE9 != 0 && pGlyphE9->texture == 0 && pGlyphE9->data != 0);
    CHECK(Near(pGlyphE9->aspect, 7.0f)); // son lineto x
    const CCFontContour *pContE9 =
        static_cast<const CCFontContour *>(pGlyphE9->data);
    CHECK(pContE9->pointCount == 3); // moveto + curveto + lineto
    CHECK(!pContE9->closed);
    CHECK(pContE9->points[1].type == 2); // curveto
    CHECK(Near(pContE9->points[1].x, 2.0f) && Near(pContE9->points[1].y, 0.0f));
    CHECK(Near(pContE9->points[1].cx1, 4.0f) && Near(pContE9->points[1].cy1, 6.0f));
    CHECK(Near(pContE9->points[1].cx2, 6.0f) && Near(pContE9->points[1].cy2, 6.0f));

    // UTF-16LE ölçü (geniş mod): {'A',0,'B',0,0,0} — 'B' glifi yok →
    // atla, 10; {'é',0,0,0} → 7; ölçek 2.0 × ('A'+'é') = 34
    // NOT: glyphFlags&2 setken çağıran UTF-16LE geçmeli (binary sözleşmesi).
    const char wAB[] = {'A', 0, 'B', 0, 0, 0};
    CHECK(Near(FontSystem_MeasureText(pVec, wAB, 1.0f), 10.0f));
    const char wE9[] = {static_cast<char>(0xE9), 0, 0, 0};
    CHECK(Near(FontSystem_MeasureText(pVec, wE9, 1.0f), 7.0f));
    const char wAE9[] = {'A', 0, static_cast<char>(0xE9), 0, 0, 0};
    CHECK(Near(FontSystem_MeasureText(pVec, wAE9, 2.0f), 34.0f));

    // ------------------------------------------------------------------
    // vektör kontur raster — sub_4D54C0 portu
    // 'A' kapanışlı: seg (0,0)→(10,0) n=(0,-1); (10,0)→(10,12) n=(1,0);
    // kapanış (10,12)→(0,0) n=(-12,10)/sqrt(244)
    // toplam = 10 + 12 + sqrt(244); w = strokeWidth (+0x70) = 2
    // şerit: moveto dejenere çift + 3 segment çifti = 4 çift = 8 vert
    // ------------------------------------------------------------------
    CCFontMaterial strokeMat;
    std::memset(&strokeMat, 0, sizeof(strokeMat));
    strokeMat.red = 10;
    strokeMat.green = 20;
    strokeMat.blue = 30;
    strokeMat.alpha = 255;
    strokeMat.strokeWidth = 2.0f;

    const char wA[] = {'A', 0, 0, 0};
    RwStub_ResetIm2DRenderCount();
    posX = 0.0f; posY = 0.0f;
    advanced = FontSystem_DrawText(pVec, wA, 1.0f, &posX, &posY, &strokeMat);
    CHECK(Near(advanced, 10.0f)); // ilerleme (stroke ama ilerleme aynı)
    CHECK(Near(posX, 10.0f));     // inout pen
    CHECK(RwStub_GetIm2DIndexedCount() == 1); // tek şerit flush
    pVerts = FontSystem_GetDrawVertices(&vertCount);
    CHECK(vertCount == 8); // 4 çift
    // P0 moveto — normal 0: A ve B base'de (0,0)
    CHECK(Near(pVerts[0].x, 0.0f) && Near(pVerts[0].y, 0.0f));
    CHECK(Near(pVerts[1].x, 0.0f) && Near(pVerts[1].y, 0.0f));
    // P1 base (10,0), n=(0,-1): A = base − n×w = (10, 2); B = (10, −2)
    CHECK(Near(pVerts[2].x, 10.0f) && Near(pVerts[2].y, 2.0f));
    CHECK(Near(pVerts[3].x, 10.0f) && Near(pVerts[3].y, -2.0f));
    // P2 base (10,12), n=(1,0): A = (8, 12); B = (12, 12)
    CHECK(Near(pVerts[4].x, 8.0f) && Near(pVerts[4].y, 12.0f));
    CHECK(Near(pVerts[5].x, 12.0f) && Near(pVerts[5].y, 12.0f));
    // P3 kapanış base (0,0): n = (-12, 10)/sqrt(244)
    const float n3x = -12.0f / std::sqrt(244.0f);
    const float n3y = 10.0f / std::sqrt(244.0f);
    CHECK(Near(pVerts[6].x, -n3x * 2.0f));
    CHECK(Near(pVerts[6].y, -n3y * 2.0f));
    CHECK(Near(pVerts[7].x, n3x * 2.0f));
    CHECK(Near(pVerts[7].y, n3y * 2.0f));
    // düz renk — tüm vertexler (10,20,30,255)
    CHECK(pVerts[0].color.red == 10 && pVerts[0].color.green == 20 &&
          pVerts[0].color.blue == 30 && pVerts[0].color.alpha == 255);
    CHECK(pVerts[7].color.red == 10 && pVerts[7].color.alpha == 255);

    // ölçek + kaydırma: scale=2, pos=(100,50) → P1 A = (120, 54), B = (120, 46)
    posX = 100.0f; posY = 50.0f;
    FontSystem_DrawText(pVec, wA, 2.0f, &posX, &posY, &strokeMat);
    pVerts = FontSystem_GetDrawVertices(&vertCount);
    CHECK(vertCount == 8);
    CHECK(Near(pVerts[2].x, 120.0f) && Near(pVerts[2].y, 54.0f));
    CHECK(Near(pVerts[3].x, 120.0f) && Near(pVerts[3].y, 46.0f));

    // aydınlatma yolu — grad = acc × (1/total) (var_64 reciprocal);
    // tepe +0x00/+0x18, taban +0x30/+0x48: P0 grad=0 → (10,..)/(1,..)
    litMat.strokeWidth = 2.0f;
    posX = 0.0f; posY = 0.0f;
    FontSystem_DrawText(pVec, wA, 1.0f, &posX, &posY, &litMat);
    pVerts = FontSystem_GetDrawVertices(&vertCount);
    CHECK(vertCount == 8);
    const float totalA = 10.0f + 12.0f + std::sqrt(244.0f);
    const float grad1 = 10.0f * (1.0f / totalA); // acc=10
    const float grad3 = totalA * (1.0f / totalA); // acc=total
    CHECK(pVerts[0].color.red == 10 && pVerts[1].color.red == 1); // grad 0
    CHECK(pVerts[2].color.red == static_cast<int>(10.0f + 100.0f * grad1));
    CHECK(pVerts[3].color.red == static_cast<int>(1.0f + 200.0f * grad1));
    // P3 grad≈1 — reciprocal zinciri testte de aynı (bit-level uyum)
    CHECK(pVerts[6].color.red == static_cast<int>(10.0f + 100.0f * grad3) &&
          pVerts[7].color.red == static_cast<int>(1.0f + 200.0f * grad3));

    // strokeWidth 0 (materyal NULL) — dejenere çiftler base'de
    posX = 0.0f; posY = 0.0f;
    FontSystem_DrawText(pVec, wA, 1.0f, &posX, &posY, 0);
    pVerts = FontSystem_GetDrawVertices(&vertCount);
    CHECK(vertCount == 8);
    CHECK(Near(pVerts[2].x, 10.0f) && Near(pVerts[2].y, 0.0f));
    CHECK(Near(pVerts[3].x, 10.0f) && Near(pVerts[3].y, 0.0f));

    // 'é' eğri — render'da 8 alt segment + 1 lineto → 10 çift = 20 vert
    const char wE9only[] = {static_cast<char>(0xE9), 0, 0, 0};
    RwStub_ResetIm2DRenderCount();
    posX = 0.0f; posY = 0.0f;
    advanced = FontSystem_DrawText(pVec, wE9only, 1.0f, &posX, &posY,
                                   &strokeMat);
    CHECK(Near(advanced, 7.0f));
    CHECK(RwStub_GetIm2DIndexedCount() == 1);
    pVerts = FontSystem_GetDrawVertices(&vertCount);
    CHECK(vertCount == 20);

    // iki glif → iki şerit flush ('A' sonra 'é'); ilerleme 17
    RwStub_ResetIm2DRenderCount();
    posX = 0.0f; posY = 0.0f;
    advanced = FontSystem_DrawText(pVec, wAE9, 1.0f, &posX, &posY, &strokeMat);
    CHECK(Near(advanced, 17.0f)); // 10 + 7
    CHECK(Near(posX, 17.0f));     // inout pen
    CHECK(RwStub_GetIm2DIndexedCount() == 2); // strip başına 1 flush

    // M3 yüklemesi doku okuyucu çağırmaz (vektör — doku çözümü yok)
    const int texCallsBeforeV3 = g_texCalls;
    FontSystemTeardown();
    FontSystemShutdown();
    CHECK(FontSystemAllocBuffers());
    CHECK(FontSystemInit());
    CCFont *pVec2 = FontSystemLoad("CCF_V3");
    CHECK(pVec2 != 0 && pVec2->glyphCount == 2);
    CHECK(g_texCalls == texCallsBeforeV3);
    FontSystemTeardown();
    FontSystemShutdown();
    CHECK(FontSystemAllocBuffers());
    CHECK(FontSystemInit());

    // ------------------------------------------------------------------
    // stream font yükleyici — sub_4CC690 / sub_4D11D0
    // Bellek akışı imleci + blob kurucu; iki yol (doku refCount==0,
    // vektör refCount≠0) + zincir (flags&4) + hata yolları.
    // ------------------------------------------------------------------
    struct MemStream {
        const unsigned char *pData;
        size_t size;
        size_t pos;
    };
    struct StreamCtx {
        MemStream mem;
        CCFontStream stream;
    };
    // (impl. aşağıda lambda benzeri statik yerine lokal fonksiyonlar —
    //  C++14: ana fonksiyon içinde functor kullanılır)
    struct MemOps {
        static RwBool Read(CCFontStream *pSelf, void *pBuf, RwInt32 size) {
            MemStream *pM = static_cast<MemStream *>(pSelf->pUser);
            if (size < 0 || pM->pos + static_cast<size_t>(size) > pM->size) {
                return FALSE;
            }
            std::memcpy(pBuf, pM->pData + pM->pos, static_cast<size_t>(size));
            pM->pos += static_cast<size_t>(size);
            return TRUE;
        }
        static RwBool ReadExact(CCFontStream *pSelf, void *pBuf,
                                RwInt32 size) {
            return Read(pSelf, pBuf, size); // tamamı okunduysa TRUE
        }
        static RwBool FindChunk(CCFontStream *pSelf, RwUInt32 type) {
            MemStream *pM = static_cast<MemStream *>(pSelf->pUser);
            if (pM->pos + 4 > pM->size) {
                return FALSE;
            }
            RwUInt32 cur = 0;
            std::memcpy(&cur, pM->pData + pM->pos, 4);
            if (cur != type) {
                return FALSE;
            }
            pM->pos += 4;
            return TRUE;
        }
    };
    // blob kurucu — küçük yardımcı
    struct Blob {
        unsigned char bytes[512];
        size_t len;
        Blob() : len(0) {}
        void U32(RwUInt32 v) {
            std::memcpy(bytes + len, &v, 4);
            len += 4;
        }
        void F32(float v) {
            std::memcpy(bytes + len, &v, 4);
            len += 4;
        }
        void Raw(const void *p, size_t n) {
            std::memcpy(bytes + len, p, n);
            len += n;
        }
        void Pad(size_t n) {
            std::memset(bytes + len, 0, n);
            len += n;
        }
        CCFont *Load() {
            StreamCtx ctx;
            ctx.mem.pData = bytes;
            ctx.mem.size = len;
            ctx.mem.pos = 0;
            ctx.stream.pUser = &ctx.mem;
            ctx.stream.read = &MemOps::Read;
            ctx.stream.readExact = &MemOps::ReadExact;
            ctx.stream.findChunk = &MemOps::FindChunk;
            return FontSystem_LoadFromStream(&ctx.stream);
        }
    };

    // --- doku yolu (refCount==0): 'A'/'B' iki glif, tek doku ---
    g_texFail = false;
    {
        Blob b;
        b.U32(0x01000000);        // tip etiketi — (>>24) == 1
        b.U32(0);                 // refCount = 0 → doku yolu
        b.F32(16.0f);             // lineExtent
        b.F32(1.0f);              // sizeScale
        b.F32(0.5f);              // spacing
        b.U32(0);                 // glyphFlags
        b.U32(0);                 // extMin
        b.U32(2);                 // glyphCount
        b.U32(0);                 // extCount
        for (int i = 0; i < 128; ++i) { // ascii 0x100: A→0, B→1
            RwUInt16 idx = 0xFFFF;
            if (i == 65) idx = 0;
            if (i == 66) idx = 1;
            b.Raw(&idx, 2);
        }
        // glif 0: u0..v1 + aspect + flags
        b.F32(0.1f); b.F32(0.2f); b.F32(0.3f); b.F32(0.4f);
        b.F32(0.5f);              // aspect
        unsigned char fl0 = 0;
        b.Raw(&fl0, 1);
        // glif 1
        b.F32(0.5f); b.F32(0.6f); b.F32(0.7f); b.F32(0.8f);
        b.F32(0.75f);
        b.Raw(&fl0, 1);
        b.U32(1);                 // texCount = 1
        b.Raw("STREAMT", 7);      // 0x20 ad (NUL dolgulu)
        b.Pad(0x20 - 7);
        CCFont *pSF = b.Load();
        CHECK(pSF != 0);
        CHECK(pSF->refCount == 0);
        CHECK(Near(pSF->lineExtent, 16.0f) && Near(pSF->sizeScale, 1.0f));
        CHECK(Near(pSF->spacing, 0.5f));
        CHECK(pSF->glyphCount == 2 && pSF->drawHandler == 0);
        CHECK(std::strcmp(g_lastName, "STREAMT") == 0 && g_lastMaskNull);
        CHECK(pSF->textures[0] == g_lastTex);
        CHECK(pSF->glyphs[0].texture == g_lastTex);
        CHECK(pSF->glyphs[1].texture == g_lastTex);
        CHECK(Near(pSF->glyphs[0].u0, 0.1f) && Near(pSF->glyphs[0].v1, 0.4f));
        CHECK(Near(pSF->glyphs[0].aspect, 0.5f));
        CHECK(Near(pSF->glyphs[1].aspect, 0.75f));
        CHECK(pSF->glyphs[0].data == 0 && pSF->glyphs[1].data == 0);
        CHECK(FontSystemGetGlyph(pSF, 'A') == &pSF->glyphs[0]);
        CHECK(FontSystemGetGlyph(pSF, 'B') == &pSF->glyphs[1]);
        CHECK(FontSystemGetGlyph(pSF, 'C') == 0);
        // ölçü: (0.5+0.5) + (0.75+0.5) = 2.25
        CHECK(Near(FontSystem_MeasureText(pSF, "AB", 1.0f), 2.25f));
    }

    // --- vektör yolu (refCount≠0): type-5 kapalı kontur + çizim ---
    {
        Blob b;
        b.U32(0x01000000);
        b.U32(1);                 // refCount = 1 → vektör yolu
        b.F32(1.0f);              // lineExtent
        b.F32(0.0f);              // sizeScale
        b.F32(0.0f);              // spacing
        b.U32(2);                 // glyphFlags (ext)
        b.U32(0xE9);             // extMin
        b.U32(1);                 // glyphCount
        b.U32(1);                 // extCount
        RwUInt16 ext0 = 0;
        b.Raw(&ext0, 2);          // extTable[0] = glif 0
        for (int i = 0; i < 128; ++i) { // ascii: hepsi boş
            RwUInt16 idx = 0xFFFF;
            b.Raw(&idx, 2);
        }
        b.U32(0);                 // atık (binary arg_0)
        b.F32(10.0f);             // aspect
        b.U32(5);                 // tip 5 → kontur
        b.U32(0x1A1);             // findChunk(0x1A1) işareti
        // kontur başlığı: sürüm 1, 3 kayıt, flags = bit1 (closed)
        b.U32(1); b.U32(3); b.U32(2); b.U32(0);
        // kayıtlar 0x18: type/x/y/nx/ny/acc
        b.U32(0); b.F32(0.0f); b.F32(0.0f);
        b.F32(0.0f); b.F32(0.0f); b.F32(0.0f);
        b.U32(1); b.F32(10.0f); b.F32(0.0f);
        b.F32(0.0f); b.F32(-1.0f); b.F32(10.0f);
        b.U32(1); b.F32(10.0f); b.F32(12.0f);
        b.F32(1.0f); b.F32(0.0f); b.F32(22.0f);
        b.U32(0);                 // post-kontur atık
        b.U32(0);                 // ortak atık (loc_4CC939)
        CCFont *pVS = b.Load();
        CHECK(pVS != 0);
        CHECK(pVS->refCount == 1 && pVS->glyphCount == 1);
        CHECK((pVS->glyphFlags & 2) != 0 && pVS->extMin == 0xE9);
        CHECK(pVS->extCount == 1 && pVS->extTable != 0 && pVS->extTable[0] == 0);
        CHECK(FontSystemGetGlyph(pVS, 0xE9) == &pVS->glyphs[0]);
        CHECK(pVS->glyphs[0].texture == 0 && pVS->drawHandler == 0);
        CHECK(Near(pVS->glyphs[0].aspect, 10.0f));
        const CCFontContour *pSC =
            static_cast<const CCFontContour *>(pVS->glyphs[0].data);
        CHECK(pSC != 0 && pSC->pointCount == 3 && pSC->closed);
        CHECK(pSC->pNext == 0);
        CHECK(pSC->points[0].type == 0 && Near(pSC->points[0].x, 0.0f));
        CHECK(pSC->points[1].type == 1 && Near(pSC->points[1].x, 10.0f));
        CHECK(pSC->points[2].type == 1 && Near(pSC->points[2].y, 12.0f));
        // çizim — CCF_V3 'A' ile aynı geometri (8 vert, 1 flush)
        strokeMat.strokeWidth = 2.0f;
        RwStub_ResetIm2DRenderCount();
        posX = 0.0f; posY = 0.0f;
        advanced = FontSystem_DrawText(pVS, wE9only, 1.0f, &posX, &posY,
                                       &strokeMat);
        CHECK(Near(advanced, 10.0f));
        CHECK(RwStub_GetIm2DIndexedCount() == 1);
        pVerts = FontSystem_GetDrawVertices(&vertCount);
        CHECK(vertCount == 8);
        CHECK(Near(pVerts[2].x, 10.0f) && Near(pVerts[2].y, 2.0f));
    }

    // --- zincir (flags&4): head (tek moveto) + child (moveto+lineto) ---
    {
        Blob b;
        b.U32(0x01000000);
        b.U32(1);                 // vektör yolu
        b.F32(1.0f); b.F32(0.0f); b.F32(0.0f);
        b.U32(0); b.U32(0);       // glyphFlags, extMin
        b.U32(1); b.U32(0);       // glyphCount, extCount
        for (int i = 0; i < 128; ++i) {
            RwUInt16 idx = 0xFFFF;
            if (i == 65) idx = 0; // 'A' → glif 0
            b.Raw(&idx, 2);
        }
        b.U32(0);                 // atık
        b.F32(1.0f);              // aspect
        b.U32(5);                 // tip 5
        b.U32(0x1A1);
        // head: 1 kayıt, flags = bit1|bit4 (closed + zincir)
        b.U32(1); b.U32(1); b.U32(6); b.U32(0);
        b.U32(0); b.F32(5.0f); b.F32(7.0f);
        b.F32(0.0f); b.F32(0.0f); b.F32(0.0f);
        b.U32(0x1A1);             // child chunk işareti
        // child: 2 kayıt, closed
        b.U32(1); b.U32(2); b.U32(2); b.U32(0);
        b.U32(0); b.F32(0.0f); b.F32(0.0f);
        b.F32(0.0f); b.F32(0.0f); b.F32(0.0f);
        b.U32(1); b.F32(3.0f); b.F32(4.0f);
        b.F32(0.0f); b.F32(0.0f); b.F32(5.0f);
        b.U32(0);                 // post-kontur atık
        b.U32(0);                 // ortak atık
        CCFont *pCS = b.Load();
        CHECK(pCS != 0 && pCS->glyphCount == 1);
        const CCFontContour *pHead =
            static_cast<const CCFontContour *>(pCS->glyphs[0].data);
        CHECK(pHead != 0 && pHead->pointCount == 1 && pHead->closed);
        CHECK(Near(pHead->points[0].x, 5.0f) && Near(pHead->points[0].y, 7.0f));
        const CCFontContour *pChild = pHead->pNext;
        CHECK(pChild != 0 && pChild->pointCount == 2 && pChild->closed);
        CHECK(pChild->pNext == 0);
        CHECK(Near(pChild->points[1].x, 3.0f) &&
              Near(pChild->points[1].y, 4.0f));
        // çizim: head tek moveto (segment yok → flush yok), child
        // moveto + lineto + kapanış → 3 çift = 6 vert, 1 flush
        RwStub_ResetIm2DRenderCount();
        const char wChainA[] = {'A', 0, 0, 0};
        posX = 0.0f; posY = 0.0f;
        FontSystem_DrawText(pCS, wChainA, 1.0f, &posX, &posY, &strokeMat);
        CHECK(RwStub_GetIm2DIndexedCount() == 1);
        pVerts = FontSystem_GetDrawVertices(&vertCount);
        CHECK(vertCount == 6);
    }

    // --- hata yolları ---
    {
        Blob bad;                 // tip baytı 0 → fail
        bad.U32(0x00000000);
        CHECK(bad.Load() == 0);
        Blob trunc;               // etiket sonrası kesik
        trunc.U32(0x01000000);
        CHECK(trunc.Load() == 0);
        Blob badVer;              // vektür kontur sürüm 2 → fail
        badVer.U32(0x01000000);
        badVer.U32(1); badVer.F32(1.0f); badVer.F32(0.0f); badVer.F32(0.0f);
        badVer.U32(0); badVer.U32(0); badVer.U32(1); badVer.U32(0);
        for (int i = 0; i < 128; ++i) {
            RwUInt16 idx = 0xFFFF;
            badVer.Raw(&idx, 2);
        }
        badVer.U32(0); badVer.F32(1.0f); badVer.U32(5);
        badVer.U32(0x1A1);
        badVer.U32(2);            // sürüm ≠ 1
        badVer.U32(1); badVer.U32(0); badVer.U32(0);
        CHECK(badVer.Load() == 0);
        Blob noChunk;             // type-5 ama findChunk başarısız
        noChunk.U32(0x01000000);
        noChunk.U32(1); noChunk.F32(1.0f); noChunk.F32(0.0f);
        noChunk.F32(0.0f); noChunk.U32(0); noChunk.U32(0);
        noChunk.U32(1); noChunk.U32(0);
        for (int i = 0; i < 128; ++i) {
            RwUInt16 idx = 0xFFFF;
            noChunk.Raw(&idx, 2);
        }
        noChunk.U32(0); noChunk.F32(1.0f); noChunk.U32(5);
        // 0x1A1 işareti YOK
        noChunk.U32(0);
        CHECK(noChunk.Load() == 0);
        // doku yolu: okuyucu fail
        Blob texFail;
        texFail.U32(0x01000000);
        texFail.U32(0); texFail.F32(16.0f); texFail.F32(1.0f);
        texFail.F32(0.0f); texFail.U32(0); texFail.U32(0);
        texFail.U32(1); texFail.U32(0);
        for (int i = 0; i < 128; ++i) {
            RwUInt16 idx = 0xFFFF;
            if (i == 65) idx = 0;
            texFail.Raw(&idx, 2);
        }
        texFail.F32(0.5f); texFail.F32(0.0f); texFail.F32(8.5f);
        texFail.F32(16.5f); texFail.F32(0.5f);
        unsigned char fl = 0;
        texFail.Raw(&fl, 1);
        texFail.U32(1);
        texFail.Raw("FAILTEX", 7);
        texFail.Pad(0x20 - 7);
        g_texFail = true;
        CHECK(texFail.Load() == 0);
        g_texFail = false;
    }

    // ------------------------------------------------------------------
    // ömür döngüsü — sub_4CD580 / sub_4CEF90 ve yeniden başlatma
    // ------------------------------------------------------------------
    FontSystemTeardown(); // sub_4CD580 (__ioterm+62)
    FontSystemShutdown(); // sub_4CEF90 (__ioterm+67)

    CHECK(FontSystemAllocBuffers());
    CHECK(FontSystemInit());
    CCFont *pFontAgain = FontSystemLoad("CCF_T2");
    CHECK(pFontAgain != 0 && pFontAgain->glyphCount == 1);
    FontSystemTeardown();
    FontSystemShutdown();

    std::printf("fontsystem_test: OK\n");
    return 0;
}
