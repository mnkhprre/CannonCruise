// ============================================================================
// CannonCruise - Font Metrics System (FontSystem.cpp)
// Original path: D:\Projects\CannonCruisePC\code\FontSystem.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_004cefd0 — FontSystemAllocBuffers (dword_66D5C4 vertex tamponu 0x2400
//                  + dword_66D5C0 0x78'lik çizim havuzu)
//   FUN_004cd660 — FontSystemInit ("fonts\" yol tamponu dword_66D5CC,
//                  unk_66D2C4..0x66D5C4 çeyrek-dörtgen indeks tablosu,
//                  dword_66D5DC/E0 liste başı, dword_66DCC0 = yükleyici)
//   FUN_004cd9a0 — FontSystemLoad ("fonts\<ad>.met", METRICS1)
//   FUN_004cd4d0 — FontAlloc (havuz dword_66D5C8, 0x144 nesne)
//   FUN_004cd8c0 — FontFree (glif verisi sub_4D0C20, doku sub_515300)
//   FUN_004cdbc0 — ParseMetrics1
//   FUN_004ce870 — ParseMetrics3 (METRICS3 vektör kontur biçimi — begin/
//                  moveto/lineto/curveto/closepath/end; kontur çizimi
//                  sub_4CCAF0 → raster sub_4D54C0 Faz 4.4b TODO)
//   FUN_004ce000 — FontFinalizeExtMap (dword_66DE1C genişletilmiş harita)
//   FUN_004cee90 — FontSystemGet (dword_66D5D8 ad listesi)
//   FUN_004cd580 — FontSystemTeardown (__ioterm+62)
//   FUN_004cef90 — FontSystemShutdown (__ioterm+67)
//   FUN_004cd7e0 — FontSystem_MeasureText (metin genişliği; glyphFlags&2
//                  → UTF-16LE üniteleri)
//   FUN_004ccc40 — FontSystem_DrawText (font->drawHandler çizim yolu;
//                  matris yığını sub_4CC2F0/4CC4D0/4CC480 ve engine
//                  vcall'leri Faz 4.4b — port ekran uzayında birebir UV/
//                  ilerleme/batch semantiğiyle üretir; UTF-16LE desteği
//                  MeasureText ile aynı)
//
// Motor bağımlılığı olmayan parçalar (bellek, dize, dosya) taşınabilir C ile
// modellenir; engine vtable eşlemeleri:
//   +0x120 strlen, +0xF8 strcpy, +0x100 strcat, +0x130 sscanf,
//   +0x114 strcmp, +0x134 malloc, +0x13C realloc, +0x138 free,
//   +0x144 havuz tahsisi, +0x148 havuz serbest bırakma
// ============================================================================

#include "FontSystem.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <cmath>

namespace
{
    // dword_66D5CC + dword_66D5D0 (kapasite 0x51) — binary Windows "fonts\"
    // kullanır; yol ayracı olarak '/' hem Windows hem POSIX'te fopen'da
    // çalıştığı için birleşik yazılır.
    const char kFontPath[] = "fonts/";

    // unk_66D2C4..(0x66D5C4) — 64 çeyrek-dörtgen × 6 × uint16 = 0x300 bayt.
    // Binary döngüsü: ecx=3, adım 4; her 12 baytlık blok:
    //   {ecx-3, ecx-1, ecx, ecx-3, ecx, ecx-2}
    RwUInt16 s_QuadIndices[64][6];

    // dword_66D5D8 — font adı önbellek listesi başı (sub_4CEE90 / sub_4CD580)
    CCFontNode *s_pFontListHead = 0;

    // dword_66D5DC / dword_66D5E0 — sub_515130 liste başı düğümü.
    // Binary dairesel düğüm + motor listesine ekler; portta basit işaretçi.
    void *s_pListNode = 0;

    // dword_66D5C4 — 0x2400 baytlık glyph vertex tamponu (sub_4CEFD0)
    void *s_pVertexBuffer = 0;

    // dword_66DE1C / dword_66DE20 — genişletilmiş kod çizme dizisi (önbellek).
    // Glif indeksi → kod eşlemesi; bitişte 0xFFFFFFFF ile sıfırlanır.
    RwInt32 *s_pExtScratch = 0;
    RwInt32  s_extScratchCap = 0;

    // sub_5154D0 doku okuyucu (test enjeksiyonu) — 0 = varsayılan rota
    FontTextureReader s_pTextureReader = 0;

    // dword_66DCC0 — yükleme geri çağrısı (sub_4CD660 kaydeder;
    // sub_4CEE90 buradan çağırır). Portta doğrudan FontSystemLoad kullanılır.
    typedef CCFont *(*FontLoaderCallback)(const char *);
    FontLoaderCallback s_pLoader = &FontSystemLoad;

    // sub_51A0A0/sub_51A230 — havuz/geri-boyama karşılığı: doğrudan malloc.
    // TODO(Faz 4.4b): engine havuz semantiği (engine+0x144/+0x148 freelist).
    void *PoolAlloc(size_t bytes) { return std::malloc(bytes); }
    void  PoolFree(void *p) { std::free(p); }

    // unk_66DDCC — başlık ayrıştırma için sprintf/scratch tamponu
    char s_Scratch[256];

    // sub_517CF0 / sub_517DB0 — global dize tamponu Set/Get karşılığı
    // (binary: dword_65DDFC[+4] tampon, [+8] kapasite). Portta statik dize.
    char s_StringBuf[512];
    void ScratchString_Set(const char *pStr) {
        std::snprintf(s_StringBuf, sizeof(s_StringBuf), "%s", pStr ? pStr : "");
    }
    const char *ScratchString_Get(void) { return s_StringBuf; }

    // sub_5154D0 — (ad, maske) → doku. Varsayılan rota rwcore.h'de bildirilen
    // RwTextureRead; rwstubs gerçek doku yokluğunda NULL döndürür (binary'de
    // doku bulunamazsa font yüklemesi başarısız olur).
    RwTexture *ReadTexture(const char *pName, const char *pMask,
                           RwInt32 *pWidth, RwInt32 *pHeight) {
        *pWidth = 0;
        *pHeight = 0;
        if (s_pTextureReader) {
            return s_pTextureReader(pName, pMask, pWidth, pHeight);
        }
        RwTexture *pTex = RwTextureRead(pName, pMask);
        if (pTex) {
            RwRaster *pRaster = RwTextureGetRaster(pTex);
            if (pRaster) {
                *pWidth = RwRasterGetWidth(pRaster);
                *pHeight = RwRasterGetHeight(pRaster);
            }
        }
        return pTex;
    }

    // sub_515300 — doku serbest bırakma (refcount azaltma karşılığı)
    void TextureRelease(RwTexture *pTex) {
        if (pTex) {
            RwTextureDestroy(pTex);
        }
    }

    // ctx vtable[0] — dosya varlık yoklaması (tek token doku maskesi için)
    RwBool FileExists(const char *pPath) {
        std::FILE *pFile = std::fopen(pPath, "rb");
        if (!pFile) {
            return FALSE;
        }
        std::fclose(pFile);
        return TRUE;
    }

    // fgets sonundaki \r\n'yi kırpır (binary motor dize karşılaştırması
    // engine+0x114 kullanır; port CRLF/LF dosyalarda aynı sonucu verir).
    void StripNewline(char *pLine) {
        size_t len = std::strlen(pLine);
        while (len > 0 && (pLine[len - 1] == '\n' || pLine[len - 1] == '\r')) {
            pLine[--len] = '\0';
        }
    }

    // sub_4CE000 — genişletilmiş (>=0x80) kod haritasını derler.
    // dword_66DE1C[] (glif indeksi → kod) taranır; min..max aralığında
    // uint16 tablo üretilir (boş girdiler 0xFFFF — binary: or eax,-1 dolgu).
    void FontFinalizeExtMap(CCFont *pFont) {
        RwInt32 count = pFont->glyphCount;
        if (count <= 0 || !s_pExtScratch) {
            return;
        }

        RwInt32 minCode = 0xFFFF; // binary: esi = 0FFFFh
        RwInt32 maxCode = 0;
        RwBool  found = FALSE;
        for (RwInt32 i = 0; i < count; ++i) {
            RwInt32 code = s_pExtScratch[i];
            if (code > 0) {
                if (code < minCode) {
                    minCode = code;
                }
                if (code > maxCode) {
                    maxCode = code;
                }
                found = TRUE;
            }
        }
        if (!found) {
            return;
        }

        RwInt32 range = maxCode - minCode + 1;
        RwUInt16 *pTable = static_cast<RwUInt16 *>(PoolAlloc(range * sizeof(RwUInt16)));
        if (!pTable) {
            return;
        }
        for (RwInt32 i = 0; i < range; ++i) {
            pTable[i] = 0xFFFF;
        }
        for (RwInt32 i = 0; i < count; ++i) {
            RwInt32 code = s_pExtScratch[i];
            if (code > 0) {
                pTable[code - minCode] = static_cast<RwUInt16>(i);
            }
        }

        pFont->extMin = minCode;
        pFont->extCount = range;
        pFont->extTable = pTable;
    }

    // ---------------------------------------------------------------------------
    // Glif kodu kaydı — sub_4CDBC0 (M1) ve sub_4CE870 (M3) ortak bloğu:
    // <0x80 → ascii[]; >=0x80 → glyphFlags|=2 + dword_66DE1C çizme dizisi
    // (kapasite +0x20 büyür; her eleman başta 0xFFFFFFFF).
    // ---------------------------------------------------------------------------
    RwBool RegisterGlyphCode(CCFont *pFont, RwInt32 code, RwInt32 glyphIndex) {
        if (code < 0x80) {
            pFont->ascii[code] = static_cast<RwUInt16>(glyphIndex);
            return TRUE;
        }
        if (glyphIndex >= s_extScratchCap) {
            RwInt32 newCap = s_extScratchCap + 0x20;
            RwInt32 *pNew = static_cast<RwInt32 *>(
                PoolAlloc(static_cast<size_t>(newCap) * sizeof(RwInt32)));
            if (!pNew) {
                return FALSE;
            }
            for (RwInt32 i = 0; i < newCap; ++i) {
                pNew[i] = -1;
            }
            for (RwInt32 i = 0; i < s_extScratchCap; ++i) {
                pNew[i] = s_pExtScratch[i];
            }
            PoolFree(s_pExtScratch);
            s_pExtScratch = pNew;
            s_extScratchCap = newCap;
        }
        pFont->glyphFlags |= 2;                 // [esi+0x10] |= 2
        s_pExtScratch[glyphIndex] = code;       // dword_66DE1C[idx] = kod
        return TRUE;
    }

    // ---------------------------------------------------------------------------
    // Vektör kontur yardımcıları — sub_4D0C90 (alt yol tahsisi), sub_4CFE80/
    // 4CFF40/4D0040 (nokta ekleme), sub_4D0220 (closepath), sub_4D0BA0 +
    // sub_4D0DB0 (sonlandırma). Binary engine havuz/array (sub_51A0A0/
    // sub_51A230) kullanır; port düz malloc ile aynı zincir semantiğini
    // modelleyen çağırıcı sözleşmesi kurar:
    //   *ppHead → zincir başı; moveto (aktif yol doluysa) sonraki alt yola
    //     geçer; PPHead güncellenir.
    // ---------------------------------------------------------------------------
    CCFontContour **ContourCurrent(CCFontContour **ppHead) {
        CCFontContour **ppCur = ppHead;
        while (*ppCur && (*ppCur)->pNext) {
            ppCur = &(*ppCur)->pNext;
        }
        return ppCur;
    }

    CCFontContour *ContourAlloc(void) {
        CCFontContour *pC = static_cast<CCFontContour *>(PoolAlloc(sizeof(CCFontContour)));
        if (!pC) {
            return 0;
        }
        pC->points = 0;
        pC->pointCount = 0;
        pC->capacity = 0;
        pC->closed = FALSE;
        pC->pNext = 0;
        return pC;
    }

    // sub_4CFE80 moveto: aktif yol boşsa nokta ekle; doluysa (binary: kök
    // dizi count!=0) yeni alt yol başlat ve oraya ekle.
    RwBool ContourMoveTo(CCFontContour **ppHead, RwReal x, RwReal y) {
        CCFontContour **ppCur = ContourCurrent(ppHead);
        if (!*ppCur) {
            *ppCur = ContourAlloc();
            if (!*ppCur) {
                return FALSE;
            }
        } else if ((*ppCur)->pointCount > 0) {
            CCFontContour *pNew = ContourAlloc();
            if (!pNew) {
                return FALSE;
            }
            (*ppCur)->pNext = pNew;
            ppCur = &(*ppCur)->pNext;
        }
        CCFontContour *pC = *ppCur;
        if (pC->pointCount >= pC->capacity) {
            RwInt32 newCap = pC->capacity > 0 ? pC->capacity * 2 : 8;
            CCFontPathPoint *pNewPts = static_cast<CCFontPathPoint *>(
                PoolAlloc(static_cast<size_t>(newCap) * sizeof(CCFontPathPoint)));
            if (!pNewPts) {
                return FALSE;
            }
            if (pC->points) {
                std::memcpy(pNewPts, pC->points,
                            static_cast<size_t>(pC->pointCount) *
                                sizeof(CCFontPathPoint));
                PoolFree(pC->points);
            }
            pC->points = pNewPts;
            pC->capacity = newCap;
        }
        CCFontPathPoint &pt = pC->points[pC->pointCount++];
        std::memset(&pt, 0, sizeof(pt));
        pt.type = 0; // binary: [eax] = 0 (moveto)
        pt.x = x;
        pt.y = y;
        return TRUE;
    }

    // nokta ekleme (lineto/curveto) — aktif yolun son alt yoluna; yoksa
    // moveto gibi başlat (binary arg yoklama — port sağlamlık sapması).
    RwBool ContourAddPoint(CCFontContour **ppHead, RwUInt32 type, const RwReal *pF,
                           RwInt32 floatCount) {
        CCFontContour **ppCur = ContourCurrent(ppHead);
        if (!*ppCur) {
            if (!ContourMoveTo(ppHead, 0.0f, 0.0f)) {
                return FALSE;
            }
            ppCur = ContourCurrent(ppHead);
        }
        CCFontContour *pC = *ppCur;
        if (pC->pointCount >= pC->capacity) {
            RwInt32 newCap = pC->capacity > 0 ? pC->capacity * 2 : 8;
            CCFontPathPoint *pNewPts = static_cast<CCFontPathPoint *>(
                PoolAlloc(static_cast<size_t>(newCap) * sizeof(CCFontPathPoint)));
            if (!pNewPts) {
                return FALSE;
            }
            if (pC->points) {
                std::memcpy(pNewPts, pC->points,
                            static_cast<size_t>(pC->pointCount) *
                                sizeof(CCFontPathPoint));
                PoolFree(pC->points);
            }
            pC->points = pNewPts;
            pC->capacity = newCap;
        }
        CCFontPathPoint &pt = pC->points[pC->pointCount++];
        std::memset(&pt, 0, sizeof(pt));
        pt.type = type;
        for (RwInt32 i = 0; i < floatCount && i < 8; ++i) {
            (&pt.x)[i] = pF[i];
        }
        return TRUE;
    }

    // sub_4D0C20 / sub_4D0BA0 + sub_4D0DB0 — glif kontur zincirini bırak.
    void ContourFreeChain(CCFontContour *pHead) {
        while (pHead) {
            CCFontContour *pNext = pHead->pNext;
            if (pHead->points) {
                PoolFree(pHead->points);
            }
            PoolFree(pHead);
            pHead = pNext;
        }
    }

    // ---------------------------------------------------------------------------
    // ParseMetrics3 (sub_4CE870) — METRICS3 vektör kontur biçimi:
    //   METRICS3\n                       — FontSystemLoad tüketir (dispatch)
    //   <ad>\n                           — "%[^\n]" (yerel; port okur ve atlar)
    //   <önek><UTF-8 kod>\n              — pending==0 satırı: binary buf[1]'den
    //                                      UTF-8 (1..6 bayt, eski 5/6 bayt
    //                                      formları dâhil) çözümleyip pending
    //                                      kodunu kurar; önek buf[0] yok
    //                                      sayılır (port biçimi: ')
    //   begin\n / end\n                  — alt yol başlangıcı / glif sonu
    //   moveto %f %f / lineto %f %f / curveto %f×6 / closepath\n
    // Glif sonunda: aspect = son path-op'un ilk float'ı (var_A0 — binary
    // quirk), data = kontur zinciri, doku = 0; çizim sub_4CCAF0 (vektör).
    // ---------------------------------------------------------------------------
    RwBool ParseMetrics3(CCFont *pFont, std::FILE *pFile) {
        char line[256];

        // 1) ad satırı — binary var_80'e okur (kullanımı gözlenmedi); port
        //    atlar.
        if (!std::fgets(line, sizeof(line), pFile)) {
            return FALSE;
        }

        // 2) glif dizisi — kapasite [font+0x130] (taze nesnede 0 → 0x100)
        RwInt32 capacity = pFont->glyphCount > 0 ? pFont->glyphCount : 0x100;
        pFont->glyphs = static_cast<CCFontGlyph *>(
            PoolAlloc(static_cast<size_t>(capacity) * sizeof(CCFontGlyph)));
        if (!pFont->glyphs) {
            return FALSE;
        }
        std::memset(pFont->glyphs, 0, static_cast<size_t>(capacity) * sizeof(CCFontGlyph));

        RwInt32 glyphCount = 0;   // var_B0
        RwInt32 pendingCode = 0;  // var_B8 — 0 = kod satırı bekleniyor
        CCFontContour *pContour = 0;
        // var_A0 — son path-op'un ilk float'ı (glif aspect'i). Binary'de
        // yığın slottur ve glifler arasında SIFIRLANMAZ (yapışkan); port
        // belirsizliği gidermek için başta 0 başlatır.
        RwReal lastFirstFloat = 0.0f;
        RwBool failed = FALSE;

        while (std::fgets(line, sizeof(line), pFile)) {
            // binary ReadLine \n'yi korur ve komutlar "begin\n" gibi tam
            // satırla strcmp edilir; port StripNewline sonrası eşler.
            StripNewline(line);

            if (pendingCode == 0) {
                // kod satırı — UTF-8 buf[1]'den (binary: 1..6 bayt eski form)
                const unsigned char *p = reinterpret_cast<const unsigned char *>(line) + 1;
                RwUInt32 code = 0;
                unsigned char b0 = p[0];
                if (b0 == 0) {
                    failed = TRUE; // önek sonrası boş satır — sert hata
                    break;
                }
                if ((b0 & 0x80u) == 0) {
                    code = b0;
                } else if ((b0 & 0xE0u) == 0xC0) {
                    code = (static_cast<RwUInt32>(b0 & 0x1Fu) << 6) |
                           (p[1] & 0x3Fu);
                } else if ((b0 & 0xF0u) == 0xE0) {
                    code = (static_cast<RwUInt32>(b0 & 0x0Fu) << 12) |
                           (static_cast<RwUInt32>(p[1] & 0x3Fu) << 6) |
                           (p[2] & 0x3Fu);
                } else if ((b0 & 0xF8u) == 0xF0) {
                    code = (static_cast<RwUInt32>(b0 & 0x07u) << 18) |
                           (static_cast<RwUInt32>(p[1] & 0x3Fu) << 12) |
                           (static_cast<RwUInt32>(p[2] & 0x3Fu) << 6) |
                           (p[3] & 0x3Fu);
                } else if ((b0 & 0xFCu) == 0xF8) {
                    code = (static_cast<RwUInt32>(b0 & 0x03u) << 24) |
                           (static_cast<RwUInt32>(p[1] & 0x3Fu) << 18) |
                           (static_cast<RwUInt32>(p[2] & 0x3Fu) << 12) |
                           (static_cast<RwUInt32>(p[3] & 0x3Fu) << 6) |
                           (p[4] & 0x3Fu);
                } else if ((b0 & 0xFEu) == 0xFC) {
                    code = (static_cast<RwUInt32>(b0 & 0x01u) << 30) |
                           (static_cast<RwUInt32>(p[1] & 0x3Fu) << 24) |
                           (static_cast<RwUInt32>(p[2] & 0x3Fu) << 18) |
                           (static_cast<RwUInt32>(p[3] & 0x3Fu) << 12) |
                           (static_cast<RwUInt32>(p[4] & 0x3Fu) << 6) |
                           (p[5] & 0x3Fu);
                } else {
                    failed = TRUE; // geçersiz önek — binary: var_B4=0 → fail
                    break;
                }
                if (code > 0xFFFF) {
                    failed = TRUE;
                    break;
                }
                pendingCode = static_cast<RwInt32>(code);
                continue;
            }

            RwReal fx[6];
            if (std::strcmp(line, "begin") == 0) {
                if (pContour) {
                    ContourFreeChain(pContour);
                }
                pContour = ContourAlloc();
                if (!pContour) {
                    failed = TRUE;
                    break;
                }
            } else if (std::strcmp(line, "end") == 0) {
                // glif kaydı — binary sub_4D0BA0 + sub_4D0DB0 (sonlandır),
                // kod kaydı, glyph[data]=contour, glyph[aspect]=var_A0
                if (glyphCount >= capacity) {
                    capacity += 0x20;
                    CCFontGlyph *pNewGlyphs = static_cast<CCFontGlyph *>(
                        PoolAlloc(static_cast<size_t>(capacity) * sizeof(CCFontGlyph)));
                    if (!pNewGlyphs) {
                        failed = TRUE;
                        break;
                    }
                    std::memcpy(pNewGlyphs, pFont->glyphs,
                                static_cast<size_t>(glyphCount) * sizeof(CCFontGlyph));
                    std::memset(pNewGlyphs + glyphCount, 0,
                                static_cast<size_t>(capacity - glyphCount) *
                                    sizeof(CCFontGlyph));
                    PoolFree(pFont->glyphs);
                    pFont->glyphs = pNewGlyphs;
                }
                if (!RegisterGlyphCode(pFont, pendingCode, glyphCount)) {
                    failed = TRUE;
                    break;
                }
                CCFontGlyph &glyph = pFont->glyphs[glyphCount];
                glyph.aspect = lastFirstFloat; // var_A0 — son op ilk float
                glyph.texture = 0;             // vektör glif — doku yok
                glyph.data = pContour;
                glyph.flags = 0;
                pContour = 0;
                pendingCode = 0;
                ++glyphCount;
            } else if (std::sscanf(line, "moveto %f %f", &fx[0], &fx[1]) == 2) {
                lastFirstFloat = fx[0]; // var_A0
                if (!ContourMoveTo(&pContour, fx[0], fx[1])) {
                    failed = TRUE;
                    break;
                }
            } else if (std::sscanf(line, "lineto %f %f", &fx[0], &fx[1]) == 2) {
                lastFirstFloat = fx[0];
                if (!ContourAddPoint(&pContour, 1, fx, 2)) {
                    failed = TRUE;
                    break;
                }
            } else if (std::sscanf(line, "curveto %f %f %f %f %f %f", &fx[0],
                                   &fx[1], &fx[2], &fx[3], &fx[4], &fx[5]) == 6) {
                lastFirstFloat = fx[0];
                if (!ContourAddPoint(&pContour, 2, fx, 6)) {
                    failed = TRUE;
                    break;
                }
            } else if (std::strcmp(line, "closepath") == 0) {
                // sub_4D0220 — binary ilk/son nokta farkını ölçüp kapanış
                // ekler; port kapatma bayrağını işaretler (raster TODO).
                CCFontContour **ppCur = ContourCurrent(&pContour);
                if (*ppCur) {
                    (*ppCur)->closed = TRUE;
                }
            }
            // bilinmeyen satır — binary strcmp/sscanf zinciri sessizce
            // düşer; port da atlar.
        }

        if (failed) {
            ContourFreeChain(pContour);
            return FALSE;
        }
        if (pendingCode != 0) {
            // kod satırı bloksuz kaldı — binary blok beklemede takılıp
            // EOF'te glyphsiz döner; port yarım glifi atlar.
            ContourFreeChain(pContour);
        }

        // bitiş — sub_4CE870 son blok
        pFont->glyphCount = glyphCount;
        FontFinalizeExtMap(pFont);
        if (s_pExtScratch) {
            for (RwInt32 i = 0; i < s_extScratchCap; ++i) {
                s_pExtScratch[i] = -1;
            }
        }
        // [font+0x138] = &sub_4CCAF0 (vektör işleyici) — portta tek yol
        // FontSystem_DrawText; raster sub_4D54C0 Faz 4.4b TODO.
        pFont->drawHandler = 0;
        return TRUE;
    }

    // ---------------------------------------------------------------------------
    // ParseMetrics1 (sub_4CDBC0) — METRICS1 metin biçimi:
    //   METRICS1\n
    //   <doku-ad> [maske]\n     — "%s %s" (maske opsiyonel)
    //   <boyut>\n               — "%d"
    //   <kod> <sol> <üst> <sağ> <alt>  — "%i %i %i %i %i" (EOF'a kadar)
    // ---------------------------------------------------------------------------
    RwBool ParseMetrics1(CCFont *pFont, std::FILE *pFile) {
        char line[256];

        // 1) başlık satırı — token1 = doku adı, token2 = maske (opsiyonel)
        if (!std::fgets(line, sizeof(line), pFile)) {
            return FALSE;
        }
        StripNewline(line);
        char token1[128] = {0};
        char token2[128] = {0};
        int tokens = std::sscanf(line, "%127s %127s", token1, token2);
        if (tokens < 1) {
            return FALSE;
        }

        // 2) doku çözümü
        const char *pMask = 0;
        if (tokens == 1) {
            // Binary: sprintf(scratch, "%sm%s", "fonts\", ad) → varlık yoklaması;
            // bulunursa maske = "m<ad>", bulunmazsa NULL.
            std::snprintf(s_Scratch, sizeof(s_Scratch), "%sm%s", kFontPath, token1);
            if (FileExists(s_Scratch)) {
                std::snprintf(s_Scratch, sizeof(s_Scratch), "m%s", token1);
                pMask = s_Scratch;
            }
        } else {
            pMask = token2;
        }

        RwInt32 texWidth = 0;
        RwInt32 texHeight = 0;
        RwTexture *pTex = ReadTexture(token1, pMask, &texWidth, &texHeight);
        if (!pTex || texWidth <= 0 || texHeight <= 0) {
            return FALSE; // binary: doku yoksa tüm yükleme başarısız
        }
        pFont->textures[0] = pTex;

        // 3) boyut satırı
        if (!std::fgets(line, sizeof(line), pFile)) {
            return FALSE;
        }
        StripNewline(line);
        int sizePx = 0;
        if (std::sscanf(line, "%d", &sizePx) != 1) {
            return FALSE;
        }

        const RwReal recipWidth = 1.0f / static_cast<RwReal>(texWidth);
        const RwReal recipHeight = 1.0f / static_cast<RwReal>(texHeight);

        // 4) glif dizisi — kapasite [esi+0x130] (taze nesnede 0 → varsayılan 256)
        RwInt32 capacity = pFont->glyphCount > 0 ? pFont->glyphCount : 0x100;
        pFont->refCount = 0; // sub_4CDBC0: dword ptr [esi] = 0
        pFont->glyphs = static_cast<CCFontGlyph *>(
            PoolAlloc(static_cast<size_t>(capacity) * sizeof(CCFontGlyph)));
        if (!pFont->glyphs) {
            return FALSE;
        }
        std::memset(pFont->glyphs, 0, static_cast<size_t>(capacity) * sizeof(CCFontGlyph));

        RwInt32 glyphCount = 0;              // var_30
        RwInt32 lastTop = 0;                 // var_28 (son satırın üst değeri)
        RwInt32 lastBottom = 0;              // var_2C (son satırın alt değeri)
        RwBool  failed = FALSE;              // var_38 == font imleci → başarılı

        while (std::fgets(line, sizeof(line), pFile)) {
            StripNewline(line);

            RwInt32 code, left, top, right, bottom;
            if (std::sscanf(line, "%i %i %i %i %i", &code, &left, &top, &right,
                            &bottom) != 5) {
                continue; // binary: 5 alan değilse satır atlanır
            }
            if (code > 0xFFFF) {
                failed = TRUE; // binary: var_38 = 0 → sert hata
                break;
            }

            // genişletilmiş kod çizme dizisi — her glif için kapasite
            // denetimi (ortak kayıt bloğu RegisterGlyphCode)
            if (!RegisterGlyphCode(pFont, code, glyphCount)) {
                failed = TRUE;
                break;
            }

            // UV + en-boy — piksel merkezi düzeltmesi 0.5 (flt_5EF624)
            CCFontGlyph &glyph = pFont->glyphs[glyphCount];
            glyph.u0 = (static_cast<RwReal>(left) + 0.5f) * recipWidth;
            glyph.v0 = (static_cast<RwReal>(top) + 0.5f) * recipHeight;
            glyph.u1 = (static_cast<RwReal>(right) + 0.5f) * recipWidth;
            glyph.v1 = (static_cast<RwReal>(bottom) + 0.5f) * recipHeight;
            glyph.texture = pTex;
            glyph.aspect = static_cast<RwReal>(right - left) /
                           static_cast<RwReal>(bottom - top);
            glyph.flags = 0;
            lastTop = top;
            lastBottom = bottom;

            ++glyphCount;
            if (glyphCount >= capacity) {
                capacity += 0x20;
                CCFontGlyph *pNewGlyphs = static_cast<CCFontGlyph *>(
                    PoolAlloc(static_cast<size_t>(capacity) * sizeof(CCFontGlyph)));
                if (!pNewGlyphs) {
                    failed = TRUE;
                    break;
                }
                std::memcpy(pNewGlyphs, pFont->glyphs,
                            static_cast<size_t>(glyphCount) * sizeof(CCFontGlyph));
                std::memset(pNewGlyphs + glyphCount, 0,
                            static_cast<size_t>(capacity - glyphCount) *
                                sizeof(CCFontGlyph));
                PoolFree(pFont->glyphs);
                pFont->glyphs = pNewGlyphs;
            }
        }

        if (failed) {
            return FALSE;
        }

        // bitiş — sub_4CDBC0 son blok
        pFont->glyphCount = glyphCount;
        FontFinalizeExtMap(pFont);
        if (s_pExtScratch) {
            // binary: genişletilmiş çizme dizisini 0xFFFFFFFF ile sıfırla
            for (RwInt32 i = 0; i < s_extScratchCap; ++i) {
                s_pExtScratch[i] = -1;
            }
        }
        pFont->spacing = 0.0f; // [esi+0Ch] = 0 — glif arası boşluk (M1)
        pFont->lineExtent = static_cast<RwReal>(lastBottom - lastTop);
        pFont->sizeScale = pFont->lineExtent != 0.0f
                               ? static_cast<RwReal>(sizePx) / pFont->lineExtent
                               : 0.0f;
        return TRUE;
    }

    // sub_4CD4D0 — font nesnesi tahsisi (0x144 havuz + alan başlangıçları)
    CCFont *FontAlloc(void) {
        CCFont *pFont = static_cast<CCFont *>(PoolAlloc(sizeof(CCFont)));
        if (!pFont) {
            return 0;
        }
        pFont->refCount = 1;
        pFont->lineExtent = 1.0f; // [esi+4] = 3F800000h
        pFont->sizeScale = 0.0f;
        pFont->spacing = 0.0f; // [esi+0Ch] (M1 bitişte yazılır)
        pFont->glyphFlags = 0;
        for (int i = 0; i < 4; ++i) {
            pFont->textures[i] = 0;
        }
        pFont->extMin = 0;
        pFont->extCount = 0;
        pFont->extTable = 0;
        pFont->glyphCount = 0;
        pFont->glyphs = static_cast<CCFontGlyph *>(
            PoolAlloc(0x20 * sizeof(CCFontGlyph))); // sub_51A0A0(0x20, 0x30190)
        pFont->reserved1 = 0;
        pFont->reserved2 = 0;
        // +0x138 — binary offset sub_4CCC40 (per-font çizim işleyicisi).
        // TODO(Faz 2): sub_4CCC40 gövdesi restor edildiğinde bağlanır.
        pFont->drawHandler = 0;
        for (int i = 0; i < 128; ++i) {
            pFont->ascii[i] = 0xFFFF; // sub_4CD4D0: rep stosd -1
        }
        if (!pFont->glyphs) {
            PoolFree(pFont);
            return 0;
        }
        std::memset(pFont->glyphs, 0, 0x20 * sizeof(CCFontGlyph));
        return pFont;
    }

    // sub_4CD8C0 — font serbest bırakma
    RwBool FontFree(CCFont *pFont) {
        if (!pFont) {
            return FALSE;
        }
        if (pFont->glyphs) {
            for (RwInt32 i = 0; i < pFont->glyphCount; ++i) {
                if (pFont->glyphs[i].data) {
                    // sub_4D0C20 — glif kontur verisi (M3: CCFontContour*
                    // alt yol zinciri; M1'de hiç dolmaz)
                    ContourFreeChain(
                        static_cast<CCFontContour *>(pFont->glyphs[i].data));
                    pFont->glyphs[i].data = 0;
                }
            }
            PoolFree(pFont->glyphs);
            pFont->glyphs = 0;
        }
        if (pFont->extTable) {
            PoolFree(pFont->extTable);
            pFont->extTable = 0;
        }
        for (int i = 0; i < 4; ++i) {
            TextureRelease(pFont->textures[i]);
            pFont->textures[i] = 0;
        }
        PoolFree(pFont);
        return TRUE;
    }
} // namespace

// ============================================================================
// FontSystemAllocBuffers (FUN_004CEFD0)
// Binary: engine+0x134(0x2400, 0x40190) → dword_66D5C4;
//         sub_50DBF0(0x78, ...) → dword_66D5C0; dönüş değeri 1.
// 0x2400 vertex tamponu glyph çizim adımı (sub_4CC690, Faz 2) içindir.
// ============================================================================
RwBool FontSystemAllocBuffers(void) {
    if (!s_pVertexBuffer) {
        s_pVertexBuffer = PoolAlloc(0x2400);
    }
    // dword_66D5C0 0x78'lik havuz — POD statik (TODO: freelist semantiği)
    return s_pVertexBuffer != 0;
}

// ============================================================================
// FontSystemInit (FUN_004CD660)
// Binary sıra: yol tamponu kopyası ("fonts\"), 64×çeyrek-dörtgen indeks
// tablosu, dword_66D5D8=0, sub_515130 liste başı, dword_66DCC0 = &sub_4CD9A0.
// ============================================================================
RwBool FontSystemInit(void) {
    // indeks tablosu — binary döngüsü (ecx=3, adım 4, 64 blok)
    RwUInt16 *pOut = &s_QuadIndices[0][0];
    RwUInt32 base = 3;
    for (RwUInt32 block = 0; block < 64; ++block, base += 4) {
        pOut[0] = static_cast<RwUInt16>(base - 3);
        pOut[1] = static_cast<RwUInt16>(base - 1);
        pOut[2] = static_cast<RwUInt16>(base);
        pOut[3] = static_cast<RwUInt16>(base - 3);
        pOut[4] = static_cast<RwUInt16>(base);
        pOut[5] = static_cast<RwUInt16>(base - 2);
        pOut += 6;
    }

    s_pFontListHead = 0; // dword_66D5D8
    if (!s_pListNode) {
        s_pListNode = PoolAlloc(sizeof(RwUInt32)); // sub_515130 karşılığı
    }
    ScratchString_Set(kFontPath); // sub_517CF0(dword_66D5CC) — "fonts\"
    s_pLoader = &FontSystemLoad;  // dword_66DCC0 = offset sub_4CD9A0
    return s_pListNode != 0;
}

// ============================================================================
// FontSystemLoad (FUN_004CD9A0)
// Binary: "fonts\<ad>.met" tamponu kurulur, ';' ile ayrılmış adaylar sırayla
// denenir; başlık METRICS1/METRICS2/METRICS3 kontrolü parser'e devreder.
// NOT (port): binary dword_66D5CC tamponunu engine dize operatörleriyle
// kopyalar (strcat + ';' döngüsü); port ad listesini doğrudan böler —
// gözlemlenen davranış (önceki adaylar başarısızsa sonraki denenir) aynıdır.
// METRICS1/METRICS3 ayrıştırıcıları bağlı; METRICS2 (sub_004ce0d0) port bloklu.
// ============================================================================
CCFont *FontSystemLoad(const char *pFontList) {
    if (!pFontList || !pFontList[0]) {
        return 0;
    }

    CCFont *pFont = FontAlloc(); // sub_4CD4D0
    if (!pFont) {
        return 0;
    }

    const char *pCandidate = pFontList;
    for (;;) {
        char name[192];
        size_t nameLen = 0;
        while (pCandidate[0] && pCandidate[0] != ';' &&
               nameLen + 1 < sizeof(name)) {
            name[nameLen++] = *pCandidate++;
        }
        name[nameLen] = '\0';

        char path[sizeof(kFontPath) + sizeof(name) + 8];
        std::snprintf(path, sizeof(path), "%s%s.met", kFontPath, name);

        std::FILE *pFile = std::fopen(path, "r");
        if (pFile) {
            char header[64] = {0};
            RwBool parsed = FALSE;
            if (std::fgets(header, sizeof(header), pFile)) {
                StripNewline(header);
                if (std::strcmp(header, "METRICS1") == 0) {
                    parsed = ParseMetrics1(pFont, pFile);
                } else if (std::strcmp(header, "METRICS3") == 0) {
                    parsed = ParseMetrics3(pFont, pFile);
                } else {
                    // TODO(Faz 4.4b): METRICS2 (FUN_004ce0d0) — raster
                    // piksel taramasına dayalı; port bloklu (ROADMAP.md)
                    parsed = FALSE;
                }
            }
            std::fclose(pFile);
            if (parsed) {
                return pFont;
            }
        }

        if (pCandidate[0] != ';') {
            break; // NUL — aday listesi tükendi (binary: jz çıkış)
        }
        ++pCandidate; // ';' — sonraki adayı dene (fallback zinciri)
    }

    FontFree(pFont);
    return 0;
}

// ---------------------------------------------------------------------------
// FontSystem_LoadFromStream (FUN_004CC690) + LoadContourFromStream
// (FUN_004D11D0) — binary stream font yükleyici (sub_491700 zinciri).
//
// Akış: tip etiketi ((etiket>>24) >= 1), font başlığı (refCount YOL
// SEÇER: 0 → doku, ≠0 → vektör), extTable ham, ascii 0x100, glifler.
// SAPMALAR: doku bağlamı kaydet/geri al (sub_5154C0/5154A0) no-op; hazır
// stroke kayıtları (0x18) yol noktalarına çevrilir (type0 → moveto,
// diğer → lineto — curveto zaten parse'da düzleşmiş); normal/uzunluk
// render'da yeniden (Aşama 3b ile aynı sapma); kontur[+0x10] vestigial;
// doku indeksi [0,3] + texCount ≤ 4 port doğrulamaları (binary
// doğrulamaz); drawHandler = 0 (DrawText tek yol).
// ---------------------------------------------------------------------------

// sub_4D11D0 — type-5 kontur kaydı: 0x10 başlık {sürüm=1, pointCount,
// flags, extra} + pointCount×0x18 hazır stroke kaydı (type/x/y/normal/
// accLen); flags&4 → findChunk(0x1A1) + zincir alt kontur ([head+0x14]).
static CCFontContour *LoadContourFromStream(CCFontStream *pStream) {
    RwUInt32 header[4];
    if (!pStream->readExact(pStream, header, 0x10)) {
        return 0;
    }
    if (header[0] != 1) {
        return 0; // binary: dec eax / jnz fail — sürüm 1
    }
    const RwUInt32 pointCount = header[1];
    const RwUInt32 flags = header[2];
    // header[3] → binary node+0x10 (var_88 — render'da strokeWidth'a
    // eklenir; portta 0 varsayılır, okunur ama yok sayılır)
    CCFontContour *pContour = ContourAlloc();
    if (!pContour) {
        return 0;
    }
    pContour->closed = ((flags >> 1) & 1) ? TRUE : FALSE; // bit1

    for (RwUInt32 i = 0; i < pointCount; ++i) {
        struct StrokeRecord {
            RwUInt32 type;
            RwReal x, y;
            RwReal nx, ny;
            RwReal accLen;
        } rec;
        if (!pStream->readExact(pStream, &rec, 0x18)) {
            ContourFreeChain(pContour);
            return 0;
        }
        // type0 → moveto; diğer (1/2 — curveto zaten düzleştirilmiş)
        // → lineto; normal/accLen render'da yeniden hesaplanır
        const RwReal f[2] = {rec.x, rec.y};
        if (!ContourAddPoint(&pContour, (rec.type == 0) ? 0u : 1u, f, 2)) {
            ContourFreeChain(pContour);
            return 0;
        }
    }
    if (flags & 4) {
        // zincir alt kontur — binary: [head+0x14] = child
        if (!pStream->findChunk(pStream, 0x1A1)) {
            ContourFreeChain(pContour);
            return 0;
        }
        CCFontContour *pChild = LoadContourFromStream(pStream);
        if (!pChild) {
            ContourFreeChain(pContour);
            return 0;
        }
        pContour->pNext = pChild;
    }
    return pContour;
}

CCFont *FontSystem_LoadFromStream(CCFontStream *pStream) {
    if (!pStream || !pStream->read || !pStream->readExact ||
        !pStream->findChunk) {
        return 0;
    }
    CCFont *pFont = FontAlloc(); // sub_4CD4D0
    if (!pFont) {
        return 0;
    }

    RwUInt32 tag = 0;
    if (!pStream->read(pStream, &tag, 4)) {
        FontFree(pFont);
        return 0;
    }
    if ((static_cast<RwInt32>(tag) >> 24) < 1) {
        FontFree(pFont); // binary: shr 18h / jge — tip baytı ≥ 1
        return 0;
    }
    // font başlığı — refCount akıştan gelir ve yol seçer
    if (!pStream->read(pStream, &pFont->refCount, 4) ||
        !pStream->read(pStream, &pFont->lineExtent, 4) ||
        !pStream->read(pStream, &pFont->sizeScale, 4) ||
        !pStream->read(pStream, &pFont->spacing, 4) ||
        !pStream->read(pStream, &pFont->glyphFlags, 4) ||
        !pStream->read(pStream, &pFont->extMin, 4) ||
        !pStream->read(pStream, &pFont->glyphCount, 4) ||
        !pStream->read(pStream, &pFont->extCount, 4)) {
        FontFree(pFont);
        return 0;
    }
    if (pFont->glyphCount < 0) { // port sağlamlık (binary doğrulamaz)
        FontFree(pFont);
        return 0;
    }
    // extTable — ham (extCount ×2 bayt); finalize gereksiz (harita hazır)
    if (pFont->extCount > 0) {
        pFont->extTable = static_cast<RwUInt16 *>(PoolAlloc(
            static_cast<size_t>(pFont->extCount) * sizeof(RwUInt16)));
        if (!pFont->extTable) {
            FontFree(pFont);
            return 0;
        }
        std::memset(pFont->extTable, 0,
                    static_cast<size_t>(pFont->extCount) * sizeof(RwUInt16));
        if (!pStream->readExact(pStream, pFont->extTable,
                                 pFont->extCount * 2)) {
            FontFree(pFont);
            return 0;
        }
    }
    // ascii harita — 0x100 bayt ham (128 × RwUInt16)
    if (!pStream->read(pStream, pFont->ascii, 0x100)) {
        FontFree(pFont);
        return 0;
    }
    // glif dizisi — glyphCount × 0x20 (sub_51A230 resize; FontAlloc'ın
    // 32'lik ön tahsisi bırakılır)
    const RwInt32 glyphCount = pFont->glyphCount;
    PoolFree(pFont->glyphs);
    pFont->glyphs = 0;
    if (glyphCount > 0) {
        pFont->glyphs = static_cast<CCFontGlyph *>(PoolAlloc(
            static_cast<size_t>(glyphCount) * sizeof(CCFontGlyph)));
        if (!pFont->glyphs) {
            pFont->glyphCount = 0;
            FontFree(pFont);
            return 0;
        }
        std::memset(pFont->glyphs, 0,
                    static_cast<size_t>(glyphCount) * sizeof(CCFontGlyph));
    }

    if (pFont->refCount == 0) {
        // ----- doku yolu (loc_4CC993) -----
        // sub_5154C0/5154A0 doku bağlamı — port ReadTexture bağımsız (no-op)
        for (RwInt32 i = 0; i < glyphCount; ++i) {
            CCFontGlyph &glyph = pFont->glyphs[i];
            if (!pStream->readExact(pStream, &glyph.u0, 0x10) || // u0,v0,u1,v1
                !pStream->read(pStream, &glyph.aspect, 4) ||
                !pStream->readExact(pStream, &glyph.flags, 1)) {
                FontFree(pFont);
                return 0;
            }
        }
        RwUInt32 texCount = 0;
        if (!pStream->read(pStream, &texCount, 4)) {
            FontFree(pFont);
            return 0;
        }
        if (texCount > 4) { // port sağlamlık (binary textures[4] doğrulamaz)
            FontFree(pFont);
            return 0;
        }
        for (RwUInt32 i = 0; i < texCount; ++i) {
            char name[0x21] = {0}; // 0x20 + NUL emniyeti
            if (!pStream->readExact(pStream, name, 0x20)) {
                FontFree(pFont);
                return 0;
            }
            RwInt32 texW = 0;
            RwInt32 texH = 0;
            pFont->textures[i] = ReadTexture(name, 0, &texW, &texH);
            if (!pFont->textures[i]) {
                FontFree(pFont);
                return 0;
            }
        }
        for (RwInt32 i = 0; i < glyphCount; ++i) {
            CCFontGlyph &glyph = pFont->glyphs[i];
            const int idx =
                static_cast<int>(static_cast<RwInt8>(glyph.flags));
            if (idx < 0 || idx > 3 || !pFont->textures[idx]) {
                FontFree(pFont); // binary: [tex]==0 → fail; indeks[port]
                return 0;
            }
            glyph.texture = pFont->textures[idx];
        }
        pFont->reserved1 = 0;
        pFont->reserved2 = 0;
        pFont->drawHandler = 0; // sub_4CCC40 — port: DrawText tek yol
    } else {
        // ----- vektör yolu -----
        for (RwInt32 i = 0; i < glyphCount; ++i) {
            CCFontGlyph &glyph = pFont->glyphs[i];
            RwUInt32 scratch = 0;
            RwUInt32 type = 0;
            if (!pStream->read(pStream, &scratch, 4) || // atık (arg_0)
                !pStream->read(pStream, &glyph.aspect, 4) ||
                !pStream->read(pStream, &type, 4)) {
                FontFree(pFont);
                return 0;
            }
            if (type == 5) {
                if (!pStream->findChunk(pStream, 0x1A1)) {
                    FontFree(pFont);
                    return 0;
                }
                CCFontContour *pContour = LoadContourFromStream(pStream);
                if (!pContour) {
                    FontFree(pFont);
                    return 0;
                }
                glyph.data = pContour;
                if (!pStream->read(pStream, &scratch, 4)) {
                    FontFree(pFont); // post-kontur atık okuması
                    return 0;
                }
            }
            if (!pStream->read(pStream, &scratch, 4)) {
                FontFree(pFont); // ortak atık (loc_4CC939)
                return 0;
            }
        }
        pFont->drawHandler = 0; // sub_4CCAF0 — port: DrawText tek yol
    }
    return pFont;
}

// ============================================================================
// FontSystemGet (FUN_004CEE90)
// Binary: dword_66D5D8 listesinde strcmp (engine+0x114) ile arar; bulunamazsa
// dword_66DCC0 (FontSystemLoad) ile yükler, havuz dword_66D5D4'den düğüm
// alıp başa ekler (düğüm: +0 name strdup, +4 font, +8 eski baş).
// ============================================================================
CCFontNode *FontSystemGet(const char *pFontName) {
    if (!pFontName) {
        return 0;
    }

    for (CCFontNode *pNode = s_pFontListHead; pNode; pNode = pNode->pNext) {
        if (pNode->pName && std::strcmp(pFontName, pNode->pName) == 0) {
            return pNode;
        }
    }

    CCFont *pFont = s_pLoader(pFontName);
    if (!pFont) {
        return 0;
    }

    CCFontNode *pNode = static_cast<CCFontNode *>(PoolAlloc(sizeof(CCFontNode)));
    if (!pNode) {
        FontFree(pFont);
        return 0;
    }
    size_t nameLen = std::strlen(pFontName) + 1;
    pNode->pName = static_cast<char *>(PoolAlloc(nameLen));
    if (!pNode->pName) {
        PoolFree(pNode);
        FontFree(pFont);
        return 0;
    }
    std::memcpy(pNode->pName, pFontName, nameLen);
    pNode->pFont = pFont;
    pNode->pNext = s_pFontListHead;
    s_pFontListHead = pNode;
    return pNode;
}

// ============================================================================
// FontSystemGetGlyph
// Binary karşılığı çizim yolunda (sub_4CC690, Faz 2); alan düzeni
// sub_4CD4D0/sub_4CDBC0 ile doğrudan okunur:
//   0..127  → font+0x24 (uint16[128], 0xFFFF = yok)
//   >=0x80  → font+0x12C tablosu (extMin aralığı, 0xFFFF = yok)
// ============================================================================
const CCFontGlyph *FontSystemGetGlyph(const CCFont *pFont, RwUInt32 code) {
    if (!pFont || !pFont->glyphs) {
        return 0;
    }

    RwUInt16 index = 0xFFFF;
    if (code < 0x80) {
        index = pFont->ascii[code];
    } else if (pFont->extTable &&
               static_cast<RwInt32>(code) >= pFont->extMin &&
               static_cast<RwInt32>(code) < pFont->extMin + pFont->extCount) {
        index = pFont->extTable[code - pFont->extMin];
    }

    if (index == 0xFFFF || index >= pFont->glyphCount) {
        return 0;
    }
    return &pFont->glyphs[index];
}

// ============================================================================
// FontSystemTeardown (FUN_004CD580 — __ioterm+62)
// Binary: ad listesi düğümü zinciri (name free + sub_4CD8C0 + havuz free),
// sub_5151D0 liste başı, sub_50DC00 havuz ×2, dword_66DE1C free,
// dword_66D5CC yol tamponu free; hepsi sıfırlanır.
// ============================================================================
void FontSystemTeardown(void) {
    CCFontNode *pNode = s_pFontListHead;
    while (pNode) {
        CCFontNode *pNext = pNode->pNext;
        if (pNode->pName) {
            PoolFree(pNode->pName);
        }
        if (pNode->pFont) {
            FontFree(pNode->pFont);
        }
        PoolFree(pNode);
        pNode = pNext;
    }
    s_pFontListHead = 0;

    if (s_pListNode) {
        PoolFree(s_pListNode); // sub_5151D0
        s_pListNode = 0;
    }
    if (s_pExtScratch) {
        PoolFree(s_pExtScratch); // dword_66DE1C
        s_pExtScratch = 0;
        s_extScratchCap = 0;
    }
    // yol tamponu (dword_66D5CC) portta sabit dize → no-op;
    // havuzlar (dword_66D5C8/D4) POD → no-op (TODO: freelist semantiği)
}

// ============================================================================
// FontSystemShutdown (FUN_004CEF90 — __ioterm+67)
// Binary: sub_50DC00(dword_66D5C0) havuz + engine+0x138(dword_66D5C4) free.
// ============================================================================
void FontSystemShutdown(void) {
    if (s_pVertexBuffer) {
        PoolFree(s_pVertexBuffer); // dword_66D5C4
        s_pVertexBuffer = 0;
    }
    // dword_66D5C0 0x78'lik havuz POD → no-op
}

// ============================================================================
// FontSystem_SetTextureReader — sub_5154D0 test enjeksiyonu (NULL = varsayılan
// RwTextureRead rotası).
// ============================================================================
void FontSystem_SetTextureReader(FontTextureReader pReader) {
    s_pTextureReader = pReader;
}

// ============================================================================
// FontSystem_GetQuadIndices — unk_66D2C4..0x66D5C4 tablosu (sub_4CD660 üretir)
// ============================================================================
const RwUInt16 (*FontSystem_GetQuadIndices(void))[6] {
    return &s_QuadIndices[0];
}

namespace
{
    // Son batch vertex sayısı — FontSystem_GetDrawVertices test erişimi
    // (sub_4CCC40 portunun batch flush doğrulaması).
    RwInt32 s_lastBatchVerts = 0;

    // sub_4CCC40 batch flush — binary sub_518EC0 (vertex aralığı) +
    // sub_518FC0 (indeksler unk_66D2C0, tip 3) + sub_518F90 (çizim).
    // Port: doğrudan RwIm2DRenderIndexedPrimitive (rwPRIMTYPETRILIST).
    // Her flush'ta batch quad 0'dan başlar → tablonun ilk quadCount
    // bloğu kullanılır.
    void FlushQuadBatch(RwInt32 quadCount) {
        if (quadCount <= 0) {
            return;
        }
        const RwUInt16(*quadIdx)[6] = FontSystem_GetQuadIndices();
        RwIm2DRenderIndexedPrimitive(rwPRIMTYPETRILIST, &quadIdx[0][0],
                                     quadCount * 6);
        s_lastBatchVerts = quadCount * 4;
    }

    // fistp + or ah,0Ch (yön sıfıra) karşılığı; port 0..255 kırpar.
    RwUInt8 ChopColorByte(RwReal value) {
        int v = static_cast<int>(value);
        if (v < 0) {
            v = 0;
        }
        if (v > 255) {
            v = 255;
        }
        return static_cast<RwUInt8>(v);
    }

    void FillTextVertex(RwIm2DVertex *pV, RwReal x, RwReal y, RwReal u,
                        RwReal v) {
        RwIm2DVertexSetScreenX(pV, x);
        RwIm2DVertexSetScreenY(pV, y);
        RwIm2DVertexSetScreenZ(
            pV, 1.0f); // dword_66D7A0 — sub_4CC1F0 başlangıç değeri
        RwIm2DVertexSetRecipCameraZ(pV, 1.0f); // rhw — TODO Faz 4.4b
        pV->u = u;
        pV->v = v;
    }

    // Aydınlatma yolu (+0x64&1): renk = ambient + ışık × degrade (binary
    // sub_4CCC40 V0/V2 taban +0x30/+0x48, V1/V3 tepe +0x00/+0x18).
    // Düz yolda renk materyal baytları (+0x60..63); materyal NULL → beyaz.
    void GetVertexColor(const CCFontMaterial *pMat, RwBool lit, RwBool bottom,
                        RwReal gradient, RwUInt8 *pR, RwUInt8 *pG,
                        RwUInt8 *pB, RwUInt8 *pA) {
        if (!lit) {
            *pR = pMat ? pMat->red   : 255;
            *pG = pMat ? pMat->green : 255;
            *pB = pMat ? pMat->blue  : 255;
            *pA = pMat ? pMat->alpha : 255;
            return;
        }
        const RwReal *pAmb = bottom ? pMat->bottomAmbient : pMat->topAmbient;
        const RwReal *pLight = bottom ? pMat->bottomLight : pMat->topLight;
        *pR = ChopColorByte(pAmb[0] + pLight[0] * gradient);
        *pG = ChopColorByte(pAmb[1] + pLight[1] * gradient);
        *pB = ChopColorByte(pAmb[2] + pLight[2] * gradient);
        *pA = ChopColorByte(pAmb[3] + pLight[3] * gradient);
    }

    // ---------------------------------------------------------------------------
    // Vektör kontur raster (sub_4D54C0 / thunk sub_4D54B0) — stroke quad
    // şeritleri. Binary her kontur kaydını (0x18: taban x/y + birim normal
    // + birikimli uzunluk — parse'da sub_4CFE80/4CFF40/4D0040 üretir) ızgaraya
    // base ∓ normal×w çifti olarak yazar; w = materyal strokeWidth (+0x70) +
    // kontur[+0x10] (portta her zaman 0). Şerit = TRIANGLEFAN tip 4; port
    // TRIALIST'e çevirir. degrade grad = birikimli/toplam yol uzunluğu
    // (var_64 = 1/son kayıt uzunluğu).
    // Sapmalar (belgeli): miter eklem yerine segment normali (sub_4CFF40
    // bir sonraki segment geldiğinde önceki normali harmanlıyor — port
    // yapmıyor); bezier parse'da düzleşmek yerine render'da sabit 8 alt
    // segmentle düzleştirilir; matris yığını yerine ekran uzayında ölçek
    // (T(pos+pen)×S(scale)); z=0/rhw=1; doku format durumları (1/9/2)
    // atlanır.
    // ---------------------------------------------------------------------------
    const RwInt32 kStrokeMaxVerts = 254; // binary: cmp eax,0FEh
    const RwInt32 kStrokeCurveSegs = 8;  // bezier alt segment (port sabiti)

    struct StrokeSeg {
        RwReal x0, y0, x1, y1;
    };

    StrokeSeg s_SegScratch[512];
    RwUInt16  s_StrokeIndices[kStrokeMaxVerts * 6];

    struct StrokeBatch {
        RwIm2DVertex *pVerts;
        RwInt32  vertCount;  // var_80
        RwInt32  pairCount;
        RwReal   scale;
        RwReal   posX;
        RwReal   posY;
        RwReal   w;                       // var_84 — strokeWidth + kontur[+0x10]
        const CCFontMaterial *pMaterial;
    };

    // sub_518EC0 + sub_518FC0 + sub_518F90 — şerit flush (tip 4 → port
    // indeksli TRIANGLELIST).
    void FlushStrokeStrip(StrokeBatch *pBatch) {
        if (pBatch->pairCount <= 1) {
            pBatch->vertCount = 0;
            pBatch->pairCount = 0;
            return;
        }
        RwInt32 idx = 0;
        for (RwInt32 i = 0; i + 1 < pBatch->pairCount; ++i) {
            const RwUInt16 a = static_cast<RwUInt16>(i * 2);
            const RwUInt16 b = static_cast<RwUInt16>(i * 2 + 1);
            const RwUInt16 c = static_cast<RwUInt16>(i * 2 + 2);
            const RwUInt16 d = static_cast<RwUInt16>(i * 2 + 3);
            s_StrokeIndices[idx++] = a;
            s_StrokeIndices[idx++] = b;
            s_StrokeIndices[idx++] = c;
            s_StrokeIndices[idx++] = b;
            s_StrokeIndices[idx++] = d;
            s_StrokeIndices[idx++] = c;
        }
        RwIm2DRenderIndexedPrimitive(rwPRIMTYPETRILIST, s_StrokeIndices, idx);
        s_lastBatchVerts = pBatch->vertCount;
        pBatch->vertCount = 0;
        pBatch->pairCount = 0;
    }

    // bir stroke çifti: A = base − n×w (tepe), B = base + n×w (taban);
    // transform T(pos)×S(scale) (binary matris yığını karşılığı).
    void EmitStrokePair(StrokeBatch *pBatch, RwReal bx, RwReal by, RwReal nx,
                        RwReal ny, RwReal w, RwReal grad,
                        const CCFontMaterial *pMat) {
        if (pBatch->vertCount + 2 > kStrokeMaxVerts) {
            FlushStrokeStrip(pBatch);
        }
        RwIm2DVertex *pA = &pBatch->pVerts[pBatch->vertCount];
        RwIm2DVertex *pB = pA + 1;

        const RwReal ax = (bx - nx * w) * pBatch->scale + pBatch->posX;
        const RwReal ay = (by - ny * w) * pBatch->scale + pBatch->posY;
        const RwReal cx = (bx + nx * w) * pBatch->scale + pBatch->posX;
        const RwReal cy = (by + ny * w) * pBatch->scale + pBatch->posY;

        FillTextVertex(pA, ax, ay, 0.0f, 0.0f);
        FillTextVertex(pB, cx, cy, 0.0f, 0.0f);

        const RwBool lit =
            (pMat != 0 && (pMat->flags & 1)) ? TRUE : FALSE;
        RwUInt8 r, g, b, a;
        GetVertexColor(pMat, lit, FALSE, grad, &r, &g, &b, &a);
        RwIm2DVertexSetIntRGBA(pA, r, g, b, a);
        GetVertexColor(pMat, lit, TRUE, grad, &r, &g, &b, &a);
        RwIm2DVertexSetIntRGBA(pB, r, g, b, a);

        if (pMat && (pMat->flags & 2)) {
            // dokulu stroke: u = uStart + uDelta×grad (sub_4D54C0)
            pA->u = pMat->uTop + pMat->uTopDelta * grad;
            pA->v = pMat->vTop + pMat->vTopDelta * grad;
            pB->u = pMat->uBottom + pMat->uBottomDelta * grad;
            pB->v = pMat->vBottom + pMat->vBottomDelta * grad;
        }

        pBatch->vertCount += 2;
        ++pBatch->pairCount;
    }

    void AddStrokeSeg(RwInt32 *pCount, RwReal x0, RwReal y0, RwReal x1,
                      RwReal y1) {
        if (*pCount >= static_cast<RwInt32>(sizeof(s_SegScratch) /
                                            sizeof(s_SegScratch[0]))) {
            return; // taşma — sabit tampon (belgeli sınır)
        }
        StrokeSeg &seg = s_SegScratch[(*pCount)++];
        seg.x0 = x0;
        seg.y0 = y0;
        seg.x1 = x1;
        seg.y1 = y1;
    }

    // tek kontur (alt yol) — şerit üretimi + flush; w/materyal batch'te
    void RenderContourStrip(const CCFontContour *pContour,
                            StrokeBatch *pBatch) {
        if (pContour->pointCount <= 0) {
            return;
        }
        const RwReal w = pBatch->w;
        const CCFontMaterial *pMat = pBatch->pMaterial;
        // başlangıç — binary her alt yol dizisi type-0 moveto kaydı ile
        // başlar (sub_4CFE80)
        const RwReal movetoX = pContour->points[0].x;
        const RwReal movetoY = pContour->points[0].y;
        RwReal penX = movetoX;
        RwReal penY = movetoY;
        RwInt32 segCount = 0;

        for (RwInt32 i = 1; i < pContour->pointCount; ++i) {
            const CCFontPathPoint &pt = pContour->points[i];
            if (pt.type == 1) { // lineto
                AddStrokeSeg(&segCount, penX, penY, pt.x, pt.y);
                penX = pt.x;
                penY = pt.y;
            } else if (pt.type == 2) { // curveto — sabit K alt segment
                const RwReal p0x = penX, p0y = penY;
                const RwReal c1x = pt.x, c1y = pt.y;
                const RwReal c2x = pt.cx1, c2y = pt.cy1;
                const RwReal p1x = pt.cx2, p1y = pt.cy2;
                RwReal prevX = p0x, prevY = p0y;
                for (RwInt32 s = 1; s <= kStrokeCurveSegs; ++s) {
                    const RwReal t = static_cast<RwReal>(s) /
                                     static_cast<RwReal>(kStrokeCurveSegs);
                    const RwReal u = 1.0f - t;
                    const RwReal x = u * u * u * p0x + 3.0f * u * u * t * c1x +
                                     3.0f * u * t * t * c2x + t * t * t * p1x;
                    const RwReal y = u * u * u * p0y + 3.0f * u * u * t * c1y +
                                     3.0f * u * t * t * c2y + t * t * t * p1y;
                    AddStrokeSeg(&segCount, prevX, prevY, x, y);
                    prevX = x;
                    prevY = y;
                }
                penX = p1x;
                penY = p1y;
            }
        }
        if (pContour->closed && segCount > 0) {
            // closepath — sub_4D0220 kapanış segmenti ekler
            AddStrokeSeg(&segCount, penX, penY, movetoX, movetoY);
        }
        if (segCount <= 0) {
            return;
        }

        // uzunluklar + toplam (var_64 = 1/son kayıt uzunluğu)
        RwReal total = 0.0f;
        for (RwInt32 i = 0; i < segCount; ++i) {
            const StrokeSeg &seg = s_SegScratch[i];
            const RwReal dx = seg.x1 - seg.x0;
            const RwReal dy = seg.y1 - seg.y0;
            total += std::sqrt(dx * dx + dy * dy);
        }
        const RwReal invTotal = (total > 0.0f) ? (1.0f / total) : 0.0f;

        // moveto kaydı — normal 0, birikim 0 (sub_4CFE80 ilk kayıt)
        EmitStrokePair(pBatch, movetoX, movetoY, 0.0f, 0.0f, w, 0.0f, pMat);
        RwReal acc = 0.0f;
        for (RwInt32 i = 0; i < segCount; ++i) {
            const StrokeSeg &seg = s_SegScratch[i];
            const RwReal dx = seg.x1 - seg.x0;
            const RwReal dy = seg.y1 - seg.y0;
            const RwReal len = std::sqrt(dx * dx + dy * dy);
            acc += len;
            RwReal nx = 0.0f, ny = 0.0f;
            if (len > 0.0f) {
                nx = dy / len;   // sub_4CFF40: n = (dy, −dx) / len
                ny = -dx / len;
            }
            EmitStrokePair(pBatch, seg.x1, seg.y1, nx, ny, w,
                           acc * invTotal, pMat);
        }
        FlushStrokeStrip(pBatch);
    }

    // zincir — sub_4D54C0 [contour+0x14] recursiyonu
    void RenderContourChain(const CCFontContour *pContour,
                            StrokeBatch *pBatch) {
        for (const CCFontContour *pC = pContour; pC; pC = pC->pNext) {
            RenderContourStrip(pC, pBatch);
        }
    }

    // glif girişi — sub_4D54B0 (thunk) + sub_4D54C0 giriş: materyal w,
    // doku durumu, vertex tamponu; zincir recursion. DrawText çağırır,
    // ilerleme DrawText'te (penX += aspect + spacing) uygulanır.
    void RenderContourGlyph(const CCFontGlyph *pGlyph, RwReal scale,
                            RwReal posX, RwReal posY,
                            const CCFontMaterial *pMaterial) {
        const CCFontContour *pContour =
            static_cast<const CCFontContour *>(pGlyph->data);
        if (!pContour) {
            return;
        }
        if (pMaterial && pMaterial->texture) {
            RwRenderStateSet(rwRENDERSTATETEXTURERASTER,
                             (void *)RwTextureGetRaster(pMaterial->texture));
        }
        StrokeBatch batch;
        batch.pVerts = static_cast<RwIm2DVertex *>(s_pVertexBuffer);
        batch.vertCount = 0;
        batch.pairCount = 0;
        batch.scale = scale;
        batch.posX = posX;
        batch.posY = posY;
        // w = mat[+0x70] + kontur[+0x10] (sub_4D54C0 var_88 — portta 0)
        batch.w = pMaterial ? pMaterial->strokeWidth : 0.0f;
        batch.pMaterial = pMaterial;
        RenderContourChain(pContour, &batch);
    }
} // namespace

// ============================================================================
// FontSystem_MeasureText (FUN_004CD7E0)
// Binary: glif arama (ASCII +0x24 / ext +0x124..); bulunamayan kod js ile
// atlanır; bulunanlarda toplam += glif.aspect + [font+0x0C] (spacing);
// dönüş = toplam × arg_8 (ölçek). glyphFlags&2 ise metin UTF-16LE — ünite
// 2 bayt ((p[1]&mask)<<8|p[0], mask=0xFF), imleç signed <=0 ile durur.
// ============================================================================
RwReal FontSystem_MeasureText(const CCFont *pFont, const char *pText,
                              RwReal scale) {
    if (!pFont || !pText) {
        return 0.0f;
    }
    const RwBool wide = (pFont->glyphFlags & 2) != 0; // binary: test al,2
    const int step = wide ? 2 : 1;
    RwReal total = 0.0f;
    for (const char *p = pText;; p += step) {
        RwUInt32 code;
        if (wide) {
            code = (static_cast<RwUInt32>(
                        static_cast<unsigned char>(p[1]))
                    << 8) |
                   static_cast<RwUInt32>(static_cast<unsigned char>(p[0]));
        } else {
            code = static_cast<RwUInt32>(static_cast<unsigned char>(p[0]));
        }
        if (static_cast<RwInt32>(code) <= 0) {
            break; // binary: or ecx,eax / jle — NUL ünitesi (signed)
        }
        const CCFontGlyph *pGlyph = FontSystemGetGlyph(pFont, code);
        if (!pGlyph) {
            continue; // binary: or ecx,ecx / jl — ilerleme yok
        }
        total += pGlyph->aspect + pFont->spacing;
    }
    return total * scale;
}

// ============================================================================
// FontSystem_DrawText (FUN_004CCC40 — font->drawHandler)
// Binary akışı: measure → invW; döngüde glif lookup, doku değişimi/63 quad
// flush, birim uzay quad (y ∈ {0,1}), UV (u0,v1)(u0,v0)(u1,v1)(u1,v0),
// ilerleme pen += aspect + spacing; son *pPos[0] += pen × scale.
// Sapma: matris yığını (sub_4CC2F0/4CC4D0/4CC480) yerine ekran uzayında
// doğrudan ölçek; z/rhw sabit (Faz 4.4b TODO). glyphFlags&2 → UTF-16LE
// üniteleri (MeasureText ile aynı). Doku'suz (M3 vektör) gliflerde quad
// yerine kontur stroke şeridi çizilir (sub_4D54C0 portu — RenderContourGlyph).
// ============================================================================
RwReal FontSystem_DrawText(const CCFont *pFont, const char *pText,
                           RwReal scale, RwReal *pPosX, RwReal *pPosY,
                           const CCFontMaterial *pMaterial) {
    if (!pFont || !pText || !pPosX || !pPosY) {
        return 0.0f;
    }
    if (!s_pVertexBuffer || !pFont->glyphs) {
        return 0.0f; // AllocBuffers çağrısız çizim yok
    }

    RwIm2DVertex *pVerts = static_cast<RwIm2DVertex *>(s_pVertexBuffer);
    const RwInt32 kFlushQuads = 63; // binary: cmp ebx,17Ah (378 indeks)
    const RwBool lit = (pMaterial != 0 && (pMaterial->flags & 1)) ? TRUE : FALSE;

    // invW = 1 / measure(font, text, 1.0f) — degrade normalizasyonu
    const RwReal textWidth = FontSystem_MeasureText(pFont, pText, 1.0f);
    const RwReal invW = (textWidth != 0.0f) ? (1.0f / textWidth) : 0.0f;

    const RwBool wide = (pFont->glyphFlags & 2) != 0; // binary: test al,2
    const int step = wide ? 2 : 1;

    RwReal penX = 0.0f;
    RwInt32 quadCount = 0;
    RwTexture *pCurTexture = 0;
    s_lastBatchVerts = 0;

    for (const char *p = pText;; p += step) {
        RwUInt32 code;
        if (wide) {
            code = (static_cast<RwUInt32>(
                        static_cast<unsigned char>(p[1]))
                    << 8) |
                   static_cast<RwUInt32>(static_cast<unsigned char>(p[0]));
        } else {
            code = static_cast<RwUInt32>(static_cast<unsigned char>(p[0]));
        }
        if (static_cast<RwInt32>(code) <= 0) {
            break; // binary: or eax,edx / jg — NUL ünitesi (signed)
        }
        const CCFontGlyph *pGlyph = FontSystemGetGlyph(pFont, code);
        if (!pGlyph) {
            continue;
        }

        if (!pGlyph->texture) {
            // vektör glif (METRICS3, sub_4CCAF0 → sub_4D54C0) — kontur
            // stroke şeridi; quad batch bu yazıma ait değil (M3 fontlarında
            // doku yok) — emniyet için flush, sonra raster + ilerleme.
            FlushQuadBatch(quadCount);
            quadCount = 0;
            RenderContourGlyph(pGlyph, scale, *pPosX + penX * scale,
                               *pPosY, pMaterial);
            penX += pGlyph->aspect + pFont->spacing;
            continue;
        }

        if (pCurTexture != pGlyph->texture) {
            FlushQuadBatch(quadCount); // binary: doku değişimi → flush
            quadCount = 0;
            pCurTexture = pGlyph->texture;
            RwRenderStateSet(rwRENDERSTATETEXTURERASTER,
                             (void *)RwTextureGetRaster(pCurTexture));
        } else if (quadCount >= kFlushQuads) {
            FlushQuadBatch(quadCount); // binary: ebx > 0x17A → flush
            quadCount = 0;
        }

        const RwReal x0 = *pPosX + penX * scale;
        const RwReal x1 = *pPosX + (penX + pGlyph->aspect) * scale;
        const RwReal y0 = *pPosY;          // birim y = 0
        const RwReal y1 = *pPosY + scale;  // birim y = 1 × ölçek

        RwIm2DVertex *pV0 = &pVerts[quadCount * 4 + 0];
        RwIm2DVertex *pV1 = &pVerts[quadCount * 4 + 1];
        RwIm2DVertex *pV2 = &pVerts[quadCount * 4 + 2];
        RwIm2DVertex *pV3 = &pVerts[quadCount * 4 + 3];

        // UV sırası binary sub_4CCC40 ile birebir:
        // V0(u0,v1) V1(u0,v0) V2(u1,v1) V3(u1,v0)
        RwUInt8 r, g, b, a;
        const RwReal gradL = penX * invW;
        const RwReal gradR = (penX + pGlyph->aspect) * invW;

        GetVertexColor(pMaterial, lit, TRUE, gradL, &r, &g, &b, &a);
        FillTextVertex(pV0, x0, y0, pGlyph->u0, pGlyph->v1);
        RwIm2DVertexSetIntRGBA(pV0, r, g, b, a);

        GetVertexColor(pMaterial, lit, FALSE, gradL, &r, &g, &b, &a);
        FillTextVertex(pV1, x0, y1, pGlyph->u0, pGlyph->v0);
        RwIm2DVertexSetIntRGBA(pV1, r, g, b, a);

        GetVertexColor(pMaterial, lit, TRUE, gradR, &r, &g, &b, &a);
        FillTextVertex(pV2, x1, y0, pGlyph->u1, pGlyph->v1);
        RwIm2DVertexSetIntRGBA(pV2, r, g, b, a);

        GetVertexColor(pMaterial, lit, FALSE, gradR, &r, &g, &b, &a);
        FillTextVertex(pV3, x1, y1, pGlyph->u1, pGlyph->v0);
        RwIm2DVertexSetIntRGBA(pV3, r, g, b, a);

        ++quadCount;
        penX += pGlyph->aspect + pFont->spacing;
    }

    FlushQuadBatch(quadCount);

    const RwReal advanced = penX * scale;
    *pPosX += advanced; // binary: fmul arg_8; fadd dword ptr [eax]
    return advanced;
}

// ============================================================================
// FontSystem_GetDrawVertices — test erişimi (sub_4CCC40 portu batch vertex'leri)
// Son flush'ın vertex sayısı s_lastBatchVerts; tampon AllocBuffers'ın 0x2400
// alanı (birden fazla batch'te son batch geçerli — vertex'ler yeniden yazılır).
// ============================================================================
const RwIm2DVertex *FontSystem_GetDrawVertices(RwInt32 *pVertexCount) {
    if (pVertexCount) {
        *pVertexCount = s_lastBatchVerts;
    }
    return s_pVertexBuffer ? static_cast<const RwIm2DVertex *>(s_pVertexBuffer)
                           : 0;
}

// Birim testler için yerleşim denetimi — binary 32-bit (Win32) düzeninde;
// 64-bit işaretçi genişliğinde alan boyutları doğal olarak değişir.
#if UINTPTR_MAX == 0xFFFFFFFFu
static_assert(sizeof(CCFontGlyph) == 0x20, "CCFontGlyph must be 0x20 bytes");
static_assert(sizeof(CCFont) == 0x144, "CCFont must be 0x144 bytes");
static_assert(sizeof(CCFontNode) == 0x0C, "CCFontNode must be 0x0C bytes");
#endif
