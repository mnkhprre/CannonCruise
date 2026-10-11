// ============================================================================
// CannonCruise - Show Text Box Entity (ShowText.cpp)
// Original path: D:\Projects\CannonCruisePC\code\ShowText.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_004b38d0 — CShowText constructor & registration
//   FUN_004b4270 — CShowText::Show
//   FUN_004b4dcf — CShowText::Render
// ============================================================================

#include "ShowText.h"
#include "RenderPlatform.h"
#include "framework/core/attributehandler/cclassfactory.h"
#include "FontSystem.h"
#include <cassert>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

RWS_REGISTER_CLASS(CShowText, []() -> void* { return new CShowText(); }, sizeof(CShowText));

// ---------------------------------------------------------------------------
// Gömülü 5x7 bitmap font — ASCII 32..126
//
// Not: Orijinal oyun custom bir font sistemi kullanır (fonts\*.met metrik
// dosyaları, FUN_004cd660 / FUN_004cd9a0). Aşama 2'de Render önce
// FontSystem'e (fonts/<ad>.met + doku) bakar; bulunamazsa bu gömülü 5x7
// tablo fallback olarak kalır.
//
// Her karakter 7 satır; bit 4 (0x10) en sol piksel → bit 0 en sağ piksel.
// ---------------------------------------------------------------------------
namespace
{
    const RwUInt8 kFont5x7[][7] =
    {
        {0x00,0x00,0x00,0x00,0x00,0x00,0x00}, // 32 ' '
        {0x04,0x04,0x04,0x04,0x04,0x00,0x04}, // 33 '!'
        {0x0A,0x0A,0x00,0x00,0x00,0x00,0x00}, // 34 '"'
        {0x0A,0x1F,0x0A,0x0A,0x1F,0x0A,0x00}, // 35 '#'
        {0x0E,0x15,0x14,0x0E,0x05,0x15,0x0E}, // 36 '$'
        {0x18,0x19,0x02,0x04,0x08,0x13,0x03}, // 37 '%'
        {0x0C,0x12,0x14,0x08,0x15,0x12,0x0D}, // 38 '&'
        {0x06,0x04,0x08,0x00,0x00,0x00,0x00}, // 39 '''
        {0x02,0x04,0x08,0x08,0x08,0x04,0x02}, // 40 '('
        {0x08,0x04,0x02,0x02,0x02,0x04,0x08}, // 41 ')'
        {0x00,0x0A,0x04,0x1F,0x04,0x0A,0x00}, // 42 '*'
        {0x00,0x04,0x04,0x1F,0x04,0x04,0x00}, // 43 '+'
        {0x00,0x00,0x00,0x00,0x06,0x04,0x08}, // 44 ','
        {0x00,0x00,0x00,0x1F,0x00,0x00,0x00}, // 45 '-'
        {0x00,0x00,0x00,0x00,0x00,0x0C,0x0C}, // 46 '.'
        {0x01,0x02,0x04,0x08,0x10,0x00,0x00}, // 47 '/'
        {0x0E,0x11,0x13,0x15,0x19,0x11,0x0E}, // 48 '0'
        {0x04,0x0C,0x04,0x04,0x04,0x04,0x0E}, // 49 '1'
        {0x0E,0x11,0x01,0x02,0x04,0x08,0x1F}, // 50 '2'
        {0x1F,0x02,0x04,0x02,0x01,0x11,0x0E}, // 51 '3'
        {0x02,0x06,0x0A,0x12,0x1F,0x02,0x02}, // 52 '4'
        {0x1F,0x10,0x1E,0x01,0x01,0x11,0x0E}, // 53 '5'
        {0x06,0x08,0x10,0x1E,0x11,0x11,0x0E}, // 54 '6'
        {0x1F,0x01,0x02,0x04,0x08,0x08,0x08}, // 55 '7'
        {0x0E,0x11,0x11,0x0E,0x11,0x11,0x0E}, // 56 '8'
        {0x0E,0x11,0x11,0x0F,0x01,0x02,0x0C}, // 57 '9'
        {0x00,0x0C,0x0C,0x00,0x0C,0x0C,0x00}, // 58 ':'
        {0x00,0x0C,0x0C,0x00,0x0C,0x04,0x08}, // 59 ';'
        {0x02,0x04,0x08,0x10,0x08,0x04,0x02}, // 60 '<'
        {0x00,0x00,0x1F,0x00,0x1F,0x00,0x00}, // 61 '='
        {0x08,0x04,0x02,0x01,0x02,0x04,0x08}, // 62 '>'
        {0x0E,0x11,0x01,0x02,0x04,0x00,0x04}, // 63 '?'
        {0x0E,0x11,0x17,0x15,0x17,0x10,0x0E}, // 64 '@'
        {0x0E,0x11,0x11,0x1F,0x11,0x11,0x11}, // 65 'A'
        {0x1E,0x11,0x11,0x1E,0x11,0x11,0x1E}, // 66 'B'
        {0x0E,0x11,0x10,0x10,0x10,0x11,0x0E}, // 67 'C'
        {0x1C,0x12,0x11,0x11,0x11,0x12,0x1C}, // 68 'D'
        {0x1F,0x10,0x10,0x1E,0x10,0x10,0x1F}, // 69 'E'
        {0x1F,0x10,0x10,0x1E,0x10,0x10,0x10}, // 70 'F'
        {0x0E,0x11,0x10,0x17,0x11,0x11,0x0F}, // 71 'G'
        {0x11,0x11,0x11,0x1F,0x11,0x11,0x11}, // 72 'H'
        {0x0E,0x04,0x04,0x04,0x04,0x04,0x0E}, // 73 'I'
        {0x07,0x02,0x02,0x02,0x02,0x12,0x0C}, // 74 'J'
        {0x11,0x12,0x14,0x18,0x14,0x12,0x11}, // 75 'K'
        {0x10,0x10,0x10,0x10,0x10,0x10,0x1F}, // 76 'L'
        {0x11,0x1B,0x15,0x15,0x11,0x11,0x11}, // 77 'M'
        {0x11,0x19,0x15,0x13,0x11,0x11,0x11}, // 78 'N'
        {0x0E,0x11,0x11,0x11,0x11,0x11,0x0E}, // 79 'O'
        {0x1E,0x11,0x11,0x1E,0x10,0x10,0x10}, // 80 'P'
        {0x0E,0x11,0x11,0x1E,0x14,0x12,0x11}, // 81 'Q'
        {0x1E,0x11,0x11,0x1E,0x14,0x12,0x11}, // 82 'R'
        {0x0F,0x10,0x10,0x0E,0x01,0x01,0x1E}, // 83 'S'
        {0x1F,0x04,0x04,0x04,0x04,0x04,0x04}, // 84 'T'
        {0x11,0x11,0x11,0x11,0x11,0x11,0x0E}, // 85 'U'
        {0x11,0x11,0x11,0x11,0x11,0x0A,0x04}, // 86 'V'
        {0x11,0x11,0x11,0x15,0x15,0x15,0x0A}, // 87 'W'
        {0x11,0x11,0x0A,0x04,0x0A,0x11,0x11}, // 88 'X'
        {0x11,0x11,0x0A,0x04,0x04,0x04,0x04}, // 89 'Y'
        {0x1F,0x01,0x02,0x04,0x08,0x10,0x1F}, // 90 'Z'
        {0x0E,0x08,0x08,0x08,0x08,0x08,0x0E}, // 91 '['
        {0x10,0x08,0x04,0x02,0x01,0x00,0x00}, // 92 '\'
        {0x0E,0x02,0x02,0x02,0x02,0x02,0x0E}, // 93 ']'
        {0x04,0x0A,0x11,0x00,0x00,0x00,0x00}, // 94 '^'
        {0x00,0x00,0x00,0x00,0x00,0x00,0x1F}, // 95 '_'
        {0x08,0x04,0x02,0x00,0x00,0x00,0x00}, // 96 '`'
        {0x00,0x00,0x0E,0x01,0x0F,0x11,0x0F}, // 97 'a'
        {0x10,0x10,0x1E,0x11,0x11,0x11,0x1E}, // 98 'b'
        {0x00,0x00,0x0E,0x11,0x10,0x11,0x0E}, // 99 'c'
        {0x01,0x01,0x0F,0x11,0x11,0x11,0x0F}, // 100 'd'
        {0x00,0x00,0x0E,0x11,0x1F,0x10,0x0E}, // 101 'e'
        {0x06,0x08,0x1C,0x08,0x08,0x08,0x08}, // 102 'f'
        {0x00,0x0F,0x11,0x11,0x0F,0x01,0x0E}, // 103 'g'
        {0x10,0x10,0x1E,0x11,0x11,0x11,0x11}, // 104 'h'
        {0x04,0x00,0x0C,0x04,0x04,0x04,0x0E}, // 105 'i'
        {0x02,0x00,0x06,0x02,0x02,0x12,0x0C}, // 106 'j'
        {0x10,0x10,0x12,0x14,0x18,0x14,0x12}, // 107 'k'
        {0x0C,0x04,0x04,0x04,0x04,0x04,0x0E}, // 108 'l'
        {0x00,0x00,0x1A,0x15,0x15,0x15,0x15}, // 109 'm'
        {0x00,0x00,0x1E,0x11,0x11,0x11,0x11}, // 110 'n'
        {0x00,0x00,0x0E,0x11,0x11,0x11,0x0E}, // 111 'o'
        {0x00,0x00,0x1E,0x11,0x1E,0x10,0x10}, // 112 'p'
        {0x00,0x00,0x0F,0x11,0x0F,0x01,0x01}, // 113 'q'
        {0x00,0x00,0x16,0x19,0x10,0x10,0x10}, // 114 'r'
        {0x00,0x00,0x0F,0x10,0x0E,0x01,0x1E}, // 115 's'
        {0x08,0x08,0x1C,0x08,0x08,0x09,0x06}, // 116 't'
        {0x00,0x00,0x11,0x11,0x11,0x13,0x0D}, // 117 'u'
        {0x00,0x00,0x11,0x11,0x11,0x0A,0x04}, // 118 'v'
        {0x00,0x00,0x11,0x11,0x15,0x15,0x0A}, // 119 'w'
        {0x00,0x00,0x11,0x0A,0x04,0x0A,0x11}, // 120 'x'
        {0x00,0x00,0x11,0x11,0x0F,0x01,0x0E}, // 121 'y'
        {0x00,0x00,0x1F,0x02,0x04,0x08,0x1F}, // 122 'z'
        {0x02,0x04,0x04,0x08,0x04,0x04,0x02}, // 123 '{'
        {0x04,0x04,0x04,0x00,0x04,0x04,0x04}, // 124 '|'
        {0x08,0x04,0x04,0x02,0x04,0x04,0x08}, // 125 '}'
        {0x00,0x00,0x08,0x15,0x02,0x00,0x00}  // 126 '~'
    };

    const int kFontGlyphW = 5;   // Bir glifin genişliği (piksel)
    const int kFontGlyphH = 7;   // Bir glifin yüksekliği (piksel)
    const int kFontAdvance = 6;  // Karakterler arası yatay adım (piksel + 1 boşluk)
    const int kFontLineStep = 9; // Satırlar arası dikey adım

    // Tek pikseli ekran koordinatında Im2D quad olarak çizer.
    // NOT: C stub RwIm2DRenderPrimitive GL3'te no-op (rwstubs.cpp) — bu yüzden
    // gerçek librw Im2D yoluna köprü kuran RenderPlatform kullanılır.
    void DrawPixel(RwReal x, RwReal y, RwReal size, RwReal recipZ, RwReal nearZ,
                   RwUInt8 r, RwUInt8 g, RwUInt8 b, RwUInt8 a)
    {
        (void)recipZ;
        (void)nearZ;
        const RwUInt32 argb = ((RwUInt32)a << 24) | ((RwUInt32)r << 16) |
                              ((RwUInt32)g << 8) | (RwUInt32)b;
        RenderPlatform_DrawOverlayQuad(x, y, size, size, argb);
    }

    // Aşama 2 — FontSystem metin yolu için satır sarmalama: '\n' ile
    // bölünür; taşan segmentler kelime kelime MeasureText ile sarmalanır.
    void SplitWrappedLines(const std::string& text, const CCFont* pFont,
                           RwReal maxWidthPx, std::vector<std::string>& out)
    {
        std::string segment;
        for (size_t i = 0; i <= text.size(); ++i)
        {
            const bool atEnd = (i == text.size());
            const char ch = atEnd ? '\n' : text[i];
            if (ch == '\n' || atEnd)
            {
                // segmenti kelime kelime sarmala
                std::string word;
                std::string line;
                for (size_t w = 0; w <= segment.size(); ++w)
                {
                    const bool segEnd = (w == segment.size());
                    const char wc = segEnd ? ' ' : segment[w];
                    if (wc == ' ' || segEnd)
                    {
                        if (!word.empty())
                        {
                            std::string candidate = line.empty()
                                                        ? word
                                                        : line + " " + word;
                            const RwReal width = FontSystem_MeasureText(
                                pFont, candidate.c_str(), 1.0f);
                            if (!line.empty() && width > maxWidthPx)
                            {
                                out.push_back(line);
                                line = word;
                            }
                            else
                            {
                                line = candidate;
                            }
                            word.clear();
                        }
                        if (segEnd)
                        {
                            break;
                        }
                    }
                    else
                    {
                        word += wc;
                    }
                }
                if (!line.empty())
                {
                    out.push_back(line);
                }
                else if (segment.empty())
                {
                    out.push_back(std::string()); // boş satır korunur
                }
                segment.clear();
            }
            else
            {
                segment += ch;
            }
        }
    }
} // namespace

/**
 * @brief Constructs a new CShowText instance.
 *
 * Sets default message display duration (3 seconds) and starts hidden.
 * Registers the MSG_SHOWTEXT_RENDER event id used by HandleEvents.
 *
 * @see FUN_004b38d0
 */
CShowText::CShowText()
    : m_fDuration(3.0f),
      m_fTimer(0.0f),
      m_bVisible(FALSE),
      m_RenderMsgId(RWS::CEventHandler::RegisterMsg("MSG_SHOWTEXT_RENDER"))
{
    // Aşama 2: FontSystem varsayılan font adı (fonts/Arial.met yoksa
    // Render 5x7 fallback'e düşer)
    std::snprintf(m_FontName, sizeof(m_FontName), "Arial");
}

/**
 * @brief Destructor for CShowText.
 */
CShowText::~CShowText()
{
}

/**
 * @brief Handles incoming event messages (e.g. MSG_SHOWTEXT_RENDER).
 * @param msg Event message container.
 */
void CShowText::HandleEvents(const RWS::CMsg& msg)
{
    // MSG_SHOWTEXT_RENDER → arka plan + metin çizimi
    if (msg.m_Id == m_RenderMsgId)
    {
        Render();
    }
}

/**
 * @brief Displays a given text string for a given duration.
 *
 * @param pText Character string to display on screen.
 * @param duration Total display time in seconds.
 * @see FUN_004b4270
 */
void CShowText::Show(const char* pText, RwReal duration)
{
    if (pText)
    {
        m_Text = pText;
        m_fDuration = duration;
        m_fTimer = 0.0f;
        m_bVisible = TRUE;
    }
}

/**
 * @brief Immediately hides the text box and clears visibility.
 */
void CShowText::Hide()
{
    m_bVisible = FALSE;
}

/**
 * @brief Sets the FontSystem font name for rendering (Aşama 2).
 * @param pFontName Requested font name; NULL resets to "Arial".
 */
void CShowText::SetFontName(const char* pFontName)
{
    std::snprintf(m_FontName, sizeof(m_FontName), "%s",
                  pFontName ? pFontName : "Arial");
}

/**
 * @brief Renders the 2D text box background and font glyphs.
 * @see FUN_004b4dcf
 */
void CShowText::Render()
{
    if (!m_bVisible) return;

    RwCamera* pCamera = RwCameraGetCurrentCamera();
    if (!pCamera) return;

    RwRaster* pRaster = RwCameraGetRaster(pCamera);
    if (!pRaster) return;

    RwReal screenWidth = (RwReal)RwRasterGetWidth(pRaster);
    RwReal screenHeight = (RwReal)RwRasterGetHeight(pRaster);
    RwReal recipZ = 1.0f / RwCameraGetNearClipPlane(pCamera);
    RwReal nearZ = RwCameraGetNearClipPlane(pCamera);

    // RenderStates ayarları
    RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void*)0);
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)FALSE);
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)FALSE);
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
    RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
    RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);

    // Alt yazılar için ekranın alt kısmında yatay siyah/yarı saydam bir şerit çiz
    RwReal boxHeight = screenHeight * 0.15f; // Ekranın %15'i yüksekliğinde
    RwReal boxY = screenHeight - boxHeight - (screenHeight * 0.05f); // Alt kenardan %5 boşluk

    // RenderPlatform köprüsü (librw Im2D) — C stub GL3'te no-op.
    RenderPlatform_DrawOverlayQuad(0.0f, boxY, screenWidth, boxHeight,
                                   0x96000000u); // Yarı saydam siyah

    // -----------------------------------------------------------------
    // Metin çizimi — önce FontSystem (fonts/*.met + glif UV'leri,
    // Aşama 2: FontSystem_DrawText sub_4CCC40 portu); font yoksa
    // gömülü 5x7 bitmap font fallback.
    // -----------------------------------------------------------------
    if (!m_Text.empty())
    {
        CCFontNode* pFontNode = FontSystemGet(m_FontName);
        const CCFont* pFont = (pFontNode != 0) ? pFontNode->pFont : 0;
        const RwReal textMarginX = screenWidth * 0.02f;
        const RwReal textMarginY = boxHeight * 0.15f;

        if (pFont != 0)
        {
            // Font sistemi yolu — satır yüksekliği ölçek olarak verilir
            const RwReal lineH = (screenHeight >= 720.0f) ? 24.0f : 16.0f;
            const RwReal maxWidthPx = screenWidth - 2.0f * textMarginX;

            CCFontMaterial mat;
            std::memset(&mat, 0, sizeof(mat));
            mat.red = 255; mat.green = 255; mat.blue = 255; mat.alpha = 255;

            std::vector<std::string> lines;
            SplitWrappedLines(m_Text, pFont, maxWidthPx, lines);

            RwReal textY = boxY + textMarginY;
            for (size_t i = 0; i < lines.size(); ++i)
            {
                RwReal textX = textMarginX;
                FontSystem_DrawText(pFont, lines[i].c_str(), lineH, &textX,
                                    &textY, &mat);
                textY += lineH;
            }
        }
        else
        {
            // 5x7 fallback — kFont5x7
            const int scale = (screenHeight >= 720.0f) ? 3 : 2;
            const RwReal advance   = (RwReal)(kFontAdvance * scale);
            const RwReal lineStep  = (RwReal)(kFontLineStep * scale);
            const RwReal pixelSize = (RwReal)scale;

            RwReal textX = textMarginX;
            RwReal textY = boxY + textMarginY;

            const int maxCols =
                (int)((screenWidth - 2.0f * textMarginX) / advance);
            int col = 0;

            for (size_t i = 0; i < m_Text.size(); ++i)
            {
                const char ch = m_Text[i];

                if (ch == '\n' || col >= maxCols)
                {
                    textX = textMarginX;
                    textY += lineStep;
                    col = 0;
                    if (ch == '\n')
                        continue;
                    // Taşan karakter aynı satırda çizilmeye devam eder
                }

                if (ch == '\n' || ch == '\r' || ch == '\t')
                    continue;

                // ASCII 32..126 dışındaki karakterler boş kutu (boşluk) çizilir
                const RwUInt8* glyph = 0;
                const unsigned char code = (unsigned char)ch;
                if (code >= 32 && code <= 126)
                    glyph = kFont5x7[code - 32];

                if (glyph)
                {
                    for (int row = 0; row < kFontGlyphH; ++row)
                    {
                        const RwUInt8 bits = glyph[row];
                        for (int bx = 0; bx < kFontGlyphW; ++bx)
                        {
                            // bit 4 = en sol piksel
                            if (bits & (RwUInt8)(0x10 >> bx))
                            {
                                DrawPixel(textX + (RwReal)(bx * scale),
                                          textY + (RwReal)(row * scale),
                                          pixelSize, recipZ, nearZ,
                                          255, 255, 255, 255);
                            }
                        }
                    }
                }

                textX += advance;
                ++col;
            }
        }
    }

    // Varsayılan duruma geri dön
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)TRUE);
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)TRUE);
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)FALSE);
}


// ---------------------------------------------------------------------------
// Gömülü 5x7 bitmap font ile düz metin çizimi (menü etiketleri / durum).
// CShowText kutusu olmadan doğrudan Im2D; DrawPixel deseni.
// ---------------------------------------------------------------------------
void ShowText_DrawBitmapText(const char *pText, RwReal x, RwReal y,
                             int scale, RwUInt8 r, RwUInt8 g, RwUInt8 b,
                             RwUInt8 a)
{
    if (pText == 0 || scale <= 0)
        return;

    // im2d nearZ sabiti (kamera erişimi yok — DrawPixel'e sabit verilir)
    const RwReal nearZ = 0.9997f;
    const RwReal recipZ = 1.0f;
    const RwReal pixelSize = (RwReal)scale;
    const RwReal advance = (RwReal)(kFontAdvance * scale);

    RwReal textX = x;
    for (const char *p = pText; *p != '\0'; ++p)
    {
        if (*p == '\n')
        {
            textX = x;
            y += (RwReal)(kFontLineStep * scale);
            continue;
        }
        const unsigned char code = (unsigned char)*p;
        if (code < 32 || code > 126)
        {
            textX += advance;
            continue;
        }
        const RwUInt8 *glyph = kFont5x7[code - 32];
        for (int row = 0; row < kFontGlyphH; ++row)
        {
            const RwUInt8 bits = glyph[row];
            for (int bx = 0; bx < kFontGlyphW; ++bx)
            {
                if (bits & (RwUInt8)(0x10 >> bx))
                {
                    DrawPixel(textX + (RwReal)(bx * scale),
                              y + (RwReal)(row * scale),
                              pixelSize, recipZ, nearZ, r, g, b, a);
                }
            }
        }
        textX += advance;
    }
}
