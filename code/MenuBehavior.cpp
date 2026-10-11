// ============================================================================
// CannonCruise - 2D Maestro Menu Behavior (MenuBehavior.cpp)
// Original path: D:\Projects\CannonCruisePC\code\MenuBehavior.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_00443050 — CMenuBehavior constructor & registration
//   FUN_00443800 — CMenuBehavior::Update
//   FUN_00443c20 — CMenuBehavior::Render
// ============================================================================

#include "MenuBehavior.h"
#include "RenderPlatform.h"
#include "ShowText.h"
#include "TxdDecode.h"
#include "InputManager.h"
#include "framework/core/attributehandler/cclassfactory.h"
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>

RWS_REGISTER_CLASS(CMenuBehavior, []() -> void* { return new CMenuBehavior(); }, sizeof(CMenuBehavior));

/**
 * @brief Constructs a new CMenuBehavior instance.
 *
 * Allocates an input focus ID to intercept keyboard / gamepad menu navigation.
 *
 * @see FUN_00443050
 */
CMenuBehavior::CMenuBehavior()
    : m_pMaestro(0),
      m_iInputFocusID(-1),
      m_screen(MENUSCREEN_MAIN),
      m_selection(0),
      m_volVoices(7),
      m_volEffects(7),
      m_volMusic(7),
      m_difficulty(0),
      m_multiplayer(0),
      m_pendingAction(MENUACTION_NONE)
{
    CInputManager* pInputMgr = CInputManager::GetSingletonPtr();
    if (pInputMgr)
    {
        m_iInputFocusID = pInputMgr->AllocateFocusID();
    }
}

/**
 * @brief Destructor.
 *
 * Frees the input focus slot and destroys the 2D Maestro object.
 */
CMenuBehavior::~CMenuBehavior()
{
    if (m_iInputFocusID != -1)
    {
        CInputManager* pInputMgr = CInputManager::GetSingletonPtr();
        if (pInputMgr)
        {
            pInputMgr->FreeFocusID(m_iInputFocusID);
        }
        m_iInputFocusID = -1;
    }

    if (m_pMaestro)
    {
        Rt2dMaestroDestroy(m_pMaestro);
        m_pMaestro = 0;
    }
}

/**
 * @brief Handles menu rendering and interaction events.
 * @param msg Event message container.
 */
void CMenuBehavior::HandleEvents(const RWS::CMsg& msg)
{
    // MSG_MENU_RENDER_FRONT_RT2D
}

/**
 * @brief Advances RenderWare 2D Maestro animation timeline.
 *
 * @param deltaTime Frame time elapsed in seconds.
 * @see FUN_00443800
 */
void CMenuBehavior::Update(RwReal deltaTime)
{
    if (m_pMaestro)
    {
        Rt2dMaestroUpdate(m_pMaestro, deltaTime);
    }
}

/**
 * @brief Dispatches drawing calls for 2D Maestro UI canvas.
 * @see FUN_00443c20
 *
 * Video-kurgu: maestro placeholder yerine parşömen menü (SCR.mov
 * MAINMENU/OPTIONS/SOUND birebir düzeni; lang_eng.txt etiketleri).
 */
void CMenuBehavior::Render()
{
    // ekran pikseli — RenderPlatform_Initialize(1280, 720)
    const RwReal kScreenW = 1280.0f;
    const RwReal kScreenH = 720.0f;
    // parşömen + yanık kenar + koyu kahve mürekkep (video renkleri)
    const RwUInt32 kParchment = 0xFFC7A05E;
    const RwUInt32 kEdge      = 0xFF1A0F08;
    const RwUInt32 kInk       = 0xFF4A2810;
    const RwUInt32 kTitleInk  = 0xFF3A1E08;
    const RwUInt32 kHugoInk   = 0xFFB44E08;
    const RwUInt32 kBall      = 0xFF201008;
    const RwUInt32 kBarFull   = 0xFFD88A18;
    const RwUInt32 kBarEmpty  = 0xFF6A4A28;
    const RwUInt32 kKeyBox    = 0xFFE8D8B0;

    RenderPlatform_BeginOverlay2D();
    const RwBool textured = HasTextures();
    const RwBool mainBg =
        (RwBool)(textured == TRUE && m_screen == MENUSCREEN_MAIN &&
                 RenderPlatform_HasTexture("Menu_Main") == TRUE);
    if (mainBg == TRUE)
    {
        // Birleşik statik zemin (074 + logo kompoziti).
        RenderPlatform_DrawTexturedQuad(0.0f, 0.0f, kScreenW, kScreenH,
                                        "Menu_Main");
    }
    else if (textured == TRUE)
    {
        // Gerçek gömülü art: Menu_074 (640×480 parşömen zemin) tam ekran.
        RenderPlatform_DrawTexturedQuad(0.0f, 0.0f, kScreenW, kScreenH,
                                        "Menu_074");
    }
    else
    {
        // zemin + yanık çerçeve (dokusuz fallback)
        RenderPlatform_DrawOverlayQuad(0.0f, 0.0f, kScreenW, kScreenH,
                                       kParchment);
        RenderPlatform_DrawOverlayQuad(0.0f, 0.0f, kScreenW, 20.0f, kEdge);
        RenderPlatform_DrawOverlayQuad(0.0f, kScreenH - 20.0f, kScreenW, 20.0f,
                                       kEdge);
        RenderPlatform_DrawOverlayQuad(0.0f, 0.0f, 20.0f, kScreenH, kEdge);
        RenderPlatform_DrawOverlayQuad(kScreenW - 20.0f, 0.0f, 20.0f, kScreenH,
                                       kEdge);
    }

    static const char *kDiffNames[3] = {"EASY", "NORMAL", "HARD"};
    char line[128];
    const RwReal listX = 200.0f;
    const RwReal ballX = 148.0f;
    const RwReal rowH = 56.0f;

    if (m_screen == MENUSCREEN_MAIN)
    {
        if (mainBg != TRUE &&
            textured == TRUE &&
            RenderPlatform_HasTexture("Menu_076") == TRUE)
        {
            // Gerçek logo art (512×128 → overlay'de 2x). Not: alfalı
            // sprite'lar Im2D'de sorunlu olabilir (Menu_Main kompoziti
            // tercih edilir); fallback bitmap başlık korunur.
            RenderPlatform_DrawTexturedQuad(140.0f, 70.0f, 1024.0f, 256.0f,
                                            "Menu_076");
        }
        else if (mainBg != TRUE)
        {
            // başlık: "Hugo" + "CANNONCRUISE" (dokusuz fallback)
            ShowText_DrawBitmapText("Hugo", 150.0f, 84.0f, 5,
                                    0xB4, 0x4E, 0x08, 255);
            ShowText_DrawBitmapText("CANNONCRUISE", 140.0f, 150.0f, 7,
                                    0x3A, 0x1E, 0x08, 255);
        }
        static const char *kItems[5] = {
            "NEW GAME", "LOAD GAME", "OPTIONS", "CREDITS", "QUIT GAME"};
        const RwReal listY = 330.0f;
        for (RwInt32 i = 0; i < 5; ++i)
        {
            const RwReal y = listY + (RwReal)i * rowH;
            if (i == m_selection)
            {
                // topuz imleç (video: seçili satırın solunda gülle)
                RenderPlatform_DrawOverlayQuad(ballX, y - 4.0f, 26.0f, 30.0f,
                                               kBall);
            }
            ShowText_DrawBitmapText(kItems[i], listX, y, 3,
                                    0x4A, 0x28, 0x10, 255);
        }
    }
    else
    {
        // alt ekran başlığı
        const char *title = "OPTIONS";
        if (m_screen == MENUSCREEN_SOUND)
        {
            title = "OPTIONS - SOUND";
        }
        else if (m_screen == MENUSCREEN_DIFFICULTY)
        {
            title = "OPTIONS - DIFFICULTY";
        }
        else if (m_screen == MENUSCREEN_CONFIRMNEW)
        {
            title = "SAVE GAME?";
        }
        ShowText_DrawBitmapText(title, 150.0f, 110.0f, 5,
                                0x3A, 0x1E, 0x08, 255);
        const RwReal listY = 270.0f;
        const RwInt32 n = ScreenItemCount();
        for (RwInt32 i = 0; i < n; ++i)
        {
            const RwReal y = listY + (RwReal)i * rowH;
            if (i == m_selection)
            {
                RenderPlatform_DrawOverlayQuad(ballX, y - 4.0f, 26.0f, 30.0f,
                                               kBall);
            }
            if (m_screen == MENUSCREEN_OPTIONS)
            {
                if (i == 0)
                {
                    ShowText_DrawBitmapText("SOUND", listX, y, 3,
                                            0x4A, 0x28, 0x10, 255);
                }
                else if (i == 1)
                {
                    std::snprintf(line, sizeof(line), "DIFFICULTY - %s",
                                  kDiffNames[m_difficulty]);
                    ShowText_DrawBitmapText(line, listX, y, 3,
                                            0x4A, 0x28, 0x10, 255);
                }
                else
                {
                    ShowText_DrawBitmapText(
                        (m_multiplayer == 0) ? "SINGLE PLAYER" : "TWO PLAYER",
                        listX, y, 3, 0x4A, 0x28, 0x10, 255);
                }
            }
            else if (m_screen == MENUSCREEN_SOUND)
            {
                const char *label = (i == 0) ? "VOICES"
                                    : (i == 1) ? "EFFECTS" : "MUSIC";
                const RwInt32 vol = (i == 0) ? m_volVoices
                                    : (i == 1) ? m_volEffects : m_volMusic;
                ShowText_DrawBitmapText(label, listX, y, 3,
                                        0x4A, 0x28, 0x10, 255);
                // 10 segmentli ses barı (video düzeni)
                for (RwInt32 s = 0; s < 10; ++s)
                {
                    RenderPlatform_DrawOverlayQuad(
                        700.0f + (RwReal)s * 26.0f, y - 2.0f, 22.0f, 26.0f,
                        (s < vol) ? kBarFull : kBarEmpty);
                }
            }
            else if (m_screen == MENUSCREEN_DIFFICULTY)
            {
                ShowText_DrawBitmapText(kDiffNames[i], listX, y, 3,
                                        0x4A, 0x28, 0x10, 255);
            }
            else // MENUSCREEN_CONFIRMNEW
            {
                ShowText_DrawBitmapText(
                    (i == 0) ? "SAVE GAME" : "DON'T SAVE", listX, y, 3,
                    0x4A, 0x28, 0x10, 255);
            }
        }
        (void)kTitleInk;
        (void)kHugoInk;
        (void)kInk;
    }

    // klavye ipuçları (video: SELECT/BACK + tuş kutuları)
    RenderPlatform_DrawOverlayQuad(150.0f, 636.0f, 86.0f, 30.0f, kKeyBox);
    ShowText_DrawBitmapText("Enter", 158.0f, 640.0f, 2, 0x3A, 0x1E, 0x08, 255);
    ShowText_DrawBitmapText("SELECT", 244.0f, 640.0f, 2, 0x4A, 0x28, 0x10,
                            255);
    RenderPlatform_DrawOverlayQuad(560.0f, 636.0f, 58.0f, 30.0f, kKeyBox);
    ShowText_DrawBitmapText("Esc", 568.0f, 640.0f, 2, 0x3A, 0x1E, 0x08, 255);
    ShowText_DrawBitmapText("BACK", 626.0f, 640.0f, 2, 0x4A, 0x28, 0x10, 255);

    RenderPlatform_EndOverlay2D();
}

RwInt32 CMenuBehavior::ScreenItemCount() const
{
    switch (m_screen)
    {
    case MENUSCREEN_MAIN: return 5;
    case MENUSCREEN_OPTIONS: return 3;
    case MENUSCREEN_SOUND: return 3;
    case MENUSCREEN_DIFFICULTY: return 3;
    case MENUSCREEN_CONFIRMNEW: return 2;
    default: return 1;
    }
}

RwBool CMenuBehavior::AdjustLeft()
{
    if (m_screen == MENUSCREEN_SOUND)
    {
        RwInt32 *vol = (m_selection == 0) ? &m_volVoices
                       : (m_selection == 1) ? &m_volEffects : &m_volMusic;
        if (*vol > 0)
        {
            --(*vol);
            return TRUE;
        }
        return FALSE;
    }
    if (m_screen == MENUSCREEN_DIFFICULTY)
    {
        m_difficulty = (m_difficulty + 2) % 3;
        return TRUE;
    }
    return FALSE;
}

RwBool CMenuBehavior::AdjustRight()
{
    if (m_screen == MENUSCREEN_SOUND)
    {
        RwInt32 *vol = (m_selection == 0) ? &m_volVoices
                       : (m_selection == 1) ? &m_volEffects : &m_volMusic;
        if (*vol < 10)
        {
            ++(*vol);
            return TRUE;
        }
        return FALSE;
    }
    if (m_screen == MENUSCREEN_DIFFICULTY)
    {
        m_difficulty = (m_difficulty + 1) % 3;
        return TRUE;
    }
    return FALSE;
}

RwBool CMenuBehavior::Confirm()
{
    switch (m_screen)
    {
    case MENUSCREEN_MAIN:
        if (m_selection == 0)
        {
            m_screen = MENUSCREEN_CONFIRMNEW; // NEW GAME → SAVE GAME?
            m_selection = 0;
            return FALSE;
        }
        if (m_selection == 1)
        {
            m_pendingAction = MENUACTION_LOADGAME; // LOAD GAME
            return TRUE;
        }
        if (m_selection == 2)
        {
            m_screen = MENUSCREEN_OPTIONS; // OPTIONS
            m_selection = 0;
            return FALSE;
        }
        if (m_selection == 3)
        {
            m_pendingAction = MENUACTION_CREDITS; // CREDITS
            return TRUE;
        }
        m_pendingAction = MENUACTION_QUITGAME; // QUIT GAME
        return TRUE;
    case MENUSCREEN_OPTIONS:
        if (m_selection == 0)
        {
            m_screen = MENUSCREEN_SOUND; // SOUND
            m_selection = 0;
            return FALSE;
        }
        if (m_selection == 1)
        {
            m_screen = MENUSCREEN_DIFFICULTY; // DIFFICULTY
            m_selection = m_difficulty;
            return FALSE;
        }
        m_multiplayer = 1 - m_multiplayer; // SINGLE/TWO PLAYER
        return FALSE;
    case MENUSCREEN_SOUND:
        m_screen = MENUSCREEN_OPTIONS; // onay = geri
        m_selection = 0;
        return FALSE;
    case MENUSCREEN_DIFFICULTY:
        m_difficulty = m_selection; // EASY/NORMAL/HARD
        m_screen = MENUSCREEN_OPTIONS;
        m_selection = 1;
        return FALSE;
    case MENUSCREEN_CONFIRMNEW:
        m_pendingAction = MENUACTION_NEWGAME; // SAVE / DON'T SAVE
        return TRUE;
    default:
        return FALSE;
    }
}

RwBool CMenuBehavior::Back()
{
    switch (m_screen)
    {
    case MENUSCREEN_OPTIONS:
        m_screen = MENUSCREEN_MAIN;
        m_selection = 2;
        return FALSE;
    case MENUSCREEN_SOUND:
        m_screen = MENUSCREEN_OPTIONS;
        m_selection = 0;
        return FALSE;
    case MENUSCREEN_DIFFICULTY:
        m_screen = MENUSCREEN_OPTIONS;
        m_selection = 1;
        return FALSE;
    case MENUSCREEN_CONFIRMNEW:
        m_screen = MENUSCREEN_MAIN;
        m_selection = 0;
        return FALSE;
    default:
        return TRUE; // MAIN → döngüden çık (quit sayılmaz)
    }
}

CMenuBehavior::EMenuAction CMenuBehavior::TakePendingAction()
{
    const EMenuAction a = m_pendingAction;
    m_pendingAction = MENUACTION_NONE;
    return a;
}

void CMenuBehavior::AttachMaestro(Rt2dMaestro* pMaestro)
{
    if (m_pMaestro)
    {
        Rt2dMaestroDestroy(m_pMaestro);
    }
    m_pMaestro = pMaestro;
}

RwBool CMenuBehavior::LoadTextures(const char* rwsPath)
{
    // Tek yol: direkt 0xe8 (RwStudio MAPMENU/GLOBAL'u atlar).
    return MenuTextures_LoadFromRws(rwsPath);
}

// Menu_Main birleşik zemin (074 + 076 logo, CPU kompozit) — hazır
// pImage doku setinden kurar; opak sonuç C888 yolundan çizilir.
static void UploadMenuMainComposite(RwTxdImage *pImage, RwBool *pAny)
{
    if (pImage == 0 || pAny == 0)
    {
        return;
    }
            // Menu_Main: statik birleşik zemin (074 + 076 logo, CPU
            // kompozit). Gerekçe: C8888 alfalı sprite'lar librw Im2D'de
            // siyah çıkıyor (bilinen quirk); birleşik doku opak→C888.
            const RwTxdTexture* bg = 0;
            const RwTxdTexture* logo = 0;
            for (RwUInt32 i = 0; i < pImage->count; ++i)
            {
                if (std::strcmp(pImage->textures[i].name, "Menu_074") == 0)
                {
                    bg = &pImage->textures[i];
                }
                else if (std::strcmp(pImage->textures[i].name, "Menu_076")
                         == 0)
                {
                    logo = &pImage->textures[i];
                }
            }
            if (bg != 0 && logo != 0 && bg->pPixels != 0 &&
                logo->pPixels != 0)
            {
                // Menu_Main: canvas çözünürlüğünde (640×480) birleşik zemin
                // — 1280×720 GL dokusu sorunlu; canvas boyutu kanıtlı çalışır.
                const RwInt32 OW = (RwInt32)bg->width;
                const RwInt32 OH = (RwInt32)bg->height;
                RwUInt8* out = static_cast<RwUInt8*>(
                    std::malloc((size_t)OW * OH * 4));
                if (out != 0)
                {
                    // zemin: birebir kopya (aynı boyut)
                    for (RwInt32 y = 0; y < OH; ++y)
                    {
                        std::memcpy(out + (size_t)y * OW * 4,
                                    bg->pPixels + (size_t)y * bg->pitch,
                                    (size_t)OW * 4);
                    }
                    // logo: 1:1 alpha blend, canvas (70,35)
                    const RwInt32 lx = 70;
                    const RwInt32 ly = 35;
                    for (RwUInt32 y = 0; y < logo->height; ++y)
                    {
                        for (RwUInt32 x = 0; x < logo->width; ++x)
                        {
                            const RwInt32 dx = lx + (RwInt32)x;
                            const RwInt32 dy = ly + (RwInt32)y;
                            if (dx < 0 || dx >= OW || dy < 0 || dy >= OH)
                            {
                                continue;
                            }
                            const RwUInt8* s = logo->pPixels +
                                ((size_t)y * logo->pitch + x * 4);
                            const float a = s[3] / 255.0f;
                            if (a <= 0.0f)
                            {
                                continue;
                            }
                            RwUInt8* d =
                                out + ((size_t)dy * OW + dx) * 4;
                            for (int c = 0; c < 3; ++c)
                            {
                                d[c] = (RwUInt8)(s[c] * a + d[c] * (1 - a) +
                                                 0.5f);
                            }
                            d[3] = 255;
                        }
                    }
                    if (RenderPlatform_UploadTexture("Menu_Main", OW, OH,
                                                     out, OW * 4) == TRUE)
                    {
                        *pAny = TRUE;
                    }
                    if (std::getenv("CANNONCRUISE_DUMPCOMPOSITE") != 0)
                    {
                        FILE* o = std::fopen("/tmp/composite.ppm", "wb");
                        if (o != 0)
                        {
                            std::fprintf(o, "P6\n%d %d\n255\n", OW, OH);
                            for (RwInt32 yy = 0; yy < OH; ++yy)
                            {
                                const RwUInt8* row =
                                    out + (size_t)yy * OW * 4;
                                for (RwInt32 xx = 0; xx < OW; ++xx)
                                {
                                    std::fputc(row[xx * 4 + 0], o);
                                    std::fputc(row[xx * 4 + 1], o);
                                    std::fputc(row[xx * 4 + 2], o);
                                }
                            }
                            std::fclose(o);
                        }
                    }
                    std::free(out);
                }
            }
}

RwBool CMenuBehavior::HasTextures() const
{
    return RenderPlatform_HasTexture("Menu_074");
}

RwBool MenuTextures_LoadFromRws(const char* rwsPath)
{
    if (rwsPath == 0)
    {
        return FALSE;
    }
    // Direkt 0xe8 txd (sabit düzen; RwStudio MAPMENU/GLOBAL atlar).
    FILE* f = std::fopen(rwsPath, "rb");
    if (f == 0)
    {
        return FALSE;
    }
    std::fseek(f, 0, SEEK_END);
    long size = std::ftell(f);
    std::fseek(f, 0, SEEK_SET);
    RwBool any = FALSE;
    if (size > 0xe8 + 8)
    {
        RwUInt8* buf = static_cast<RwUInt8*>(std::malloc((size_t)size));
        if (buf != 0 && std::fread(buf, 1, (size_t)size, f) == (size_t)size)
        {
            RwTxdImage* pImage = static_cast<RwTxdImage*>(
                std::malloc(sizeof(RwTxdImage)));
            if (pImage != 0)
            {
                // decoder sayaç-sınırlı: dosya sonuna kadar güvenli.
                if (RwTxdDecodeImage(buf + 0xe8, (RwUInt32)size - 0xe8,
                                     pImage) == TRUE)
                {
                    for (RwUInt32 i = 0; i < pImage->count; ++i)
                    {
                        const RwTxdTexture* t = &pImage->textures[i];
                        if (t->pPixels != 0 && t->width > 0 &&
                            t->height > 0 &&
                            RenderPlatform_UploadTexture(
                                t->name, (RwInt32)t->width,
                                (RwInt32)t->height, t->pPixels,
                                (RwInt32)t->pitch) == TRUE)
                        {
                            any = TRUE;
                        }
                    }
                    // Menu_Main kompoziti (074+076 varsa; yoksa no-op).
                    UploadMenuMainComposite(pImage, &any);
                }
                std::free(pImage);
            }
            std::free(buf);
        }
    }
    std::fclose(f);
    return any;
}

RwBool CMenuBehavior::MoveNext()
{
    if (m_pMaestro == 0)
    {
        return FALSE;
    }
    const RwInt32 n = ScreenItemCount();
    if (n > 0)
    {
        m_selection = (m_selection + 1) % n;
    }
    return Rt2dMaestroSelectNext(m_pMaestro);
}

RwBool CMenuBehavior::MovePrev()
{
    if (m_pMaestro == 0)
    {
        return FALSE;
    }
    const RwInt32 n = ScreenItemCount();
    if (n > 0)
    {
        m_selection = (m_selection + n - 1) % n;
    }
    return Rt2dMaestroSelectPrev(m_pMaestro);
}

const char* CMenuBehavior::GetSelectedCommand() const
{
    if (m_pMaestro == 0)
    {
        return 0;
    }
    return Rt2dMaestroGetSelectedCommand(m_pMaestro);
}

