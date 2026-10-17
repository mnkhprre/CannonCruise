// ============================================================================
// CannonCruise - After Game Menu Behavior (AfterGameMenuBehavior.cpp)
// Original path: D:\Projects\CannonCruisePC\code\AfterGameMenuBehavior.cpp
// Reverse-engineered via Ghidra and CC.asm analysis.
//
// Binary functions:
//   FUN_0043cb30 — CAfterGameMenuBehavior constructor & registration
//   FUN_0043d9d0 — CAfterGameMenuBehavior::ShowSummary
// ============================================================================

#include "AfterGameMenuBehavior.h"
#include "Rt2dMaestro.h"
#include "Rt2dMaestroStream.h"
#include "RenderPlatform.h"
#include "ShowText.h"
#include "framework/core/attributehandler/cclassfactory.h"
#include <cassert>
#include <cstdio>
#include <cstring>

RWS_REGISTER_CLASS(CAfterGameMenuBehavior, []() -> void* { return new CAfterGameMenuBehavior(); }, sizeof(CAfterGameMenuBehavior));

/**
 * @brief Constructs a new CAfterGameMenuBehavior instance.
 *
 * Sets score, accuracy, and rating values to zero.
 *
 * @see FUN_0043cb30
 */
CAfterGameMenuBehavior::CAfterGameMenuBehavior()
    : m_pMaestro(0),
      m_levelCount(0),
      m_selection(0),
      m_pendingLevel(-1),
      m_bSummary(FALSE),
      m_uScore(0),
      m_uAccuracy(0),
      m_uRating(0)
{
    std::memset(m_levels, 0, sizeof(m_levels));
}

/**
 * @brief Destructor.
 */
CAfterGameMenuBehavior::~CAfterGameMenuBehavior()
{
    if (m_pMaestro != 0)
    {
        Rt2dMaestroDestroy(m_pMaestro);
        m_pMaestro = 0;
    }
}

/**
 * @brief Handles menu events (such as MENUTRIGGER_MapMenuEnter or MapMenuRatingInquire).
 * @param msg Event message container.
 */
void CAfterGameMenuBehavior::HandleEvents(const RWS::CMsg& msg)
{
    // MENUTRIGGER_MapMenuEnter, MapMenuRatingInquire etc.
}

/**
 * @brief Updates post-game statistics to be displayed to the player.
 * @see FUN_0043d9d0
 */
void CAfterGameMenuBehavior::ShowSummary(RwUInt32 score, RwUInt32 accuracy,
                                         RwUInt32 rating)
{
    m_uScore = score;
    m_uAccuracy = accuracy;
    m_uRating = rating;
    m_bSummary = TRUE; // summary screen (confirm/back → map)
}

void CAfterGameMenuBehavior::AttachMaestro(Rt2dMaestro* pMaestro)
{
    if (m_pMaestro != 0)
    {
        Rt2dMaestroDestroy(m_pMaestro);
    }
    m_pMaestro = pMaestro;
    m_levelCount = 0;
    m_selection = 0;
    m_pendingLevel = -1;
    if (pMaestro == 0)
    {
        return;
    }
    // Level groups: "/Viking_1/" … (type=1, '_' + digit in name).
    for (RwUInt32 i = 0;
         i < pMaestro->info.nodeCount && m_levelCount < MAX_LEVELS; ++i)
    {
        if (pMaestro->info.nodes[i].type != rwrt2dNODE_GROUP)
        {
            continue;
        }
        const char* raw = pMaestro->info.nodes[i].name;
        // "/Viking_1/" → base="Viking", num=1
        char base[32] = {0};
        int num = 0;
        if (std::sscanf(raw, "/%31[^_/]_%d/", base, &num) != 2 || num <= 0)
        {
            continue;
        }
        MapLevel* lv = &m_levels[m_levelCount];
        // "VIKING 1" (uppercase) + "VIKING01" (file core)
        size_t bi = 0;
        for (size_t k = 0; base[k] != 0 && bi + 1 < sizeof(lv->display); ++k)
        {
            char c = base[k];
            lv->display[bi++] =
                (char)((c >= 'a' && c <= 'z') ? (c - 'a' + 'A') : c);
        }
        std::snprintf(lv->display + bi, sizeof(lv->display) - bi, " %d",
                      num);
        // "VIKING01" (file core)
        lv->file[0] = 0;
        bi = 0;
        for (size_t k = 0; base[k] != 0 && bi + 1 < sizeof(lv->file); ++k)
        {
            char c = base[k];
            lv->file[bi++] =
                (char)((c >= 'a' && c <= 'z') ? (c - 'a' + 'A') : c);
        }
        std::snprintf(lv->file + bi, sizeof(lv->file) - bi, "%02d", num);
        ++m_levelCount;
    }
}

RwBool CAfterGameMenuBehavior::MoveNext()
{
    if (m_pMaestro == 0 || m_bSummary == TRUE)
    {
        return FALSE;
    }
    if (m_levelCount > 0)
    {
        m_selection = (m_selection + 1) % m_levelCount;
    }
    return Rt2dMaestroSelectNext(m_pMaestro);
}

RwBool CAfterGameMenuBehavior::MovePrev()
{
    if (m_pMaestro == 0 || m_bSummary == TRUE)
    {
        return FALSE;
    }
    if (m_levelCount > 0)
    {
        m_selection = (m_selection + m_levelCount - 1) % m_levelCount;
    }
    return Rt2dMaestroSelectPrev(m_pMaestro);
}

RwBool CAfterGameMenuBehavior::Confirm()
{
    if (m_bSummary == TRUE)
    {
        m_bSummary = FALSE; // from summary back to map
        return FALSE;
    }
    if (m_levelCount <= 0)
    {
        return FALSE;
    }
    m_pendingLevel = m_selection; // MapMenuEnter
    return TRUE;
}

RwBool CAfterGameMenuBehavior::Back()
{
    if (m_bSummary == TRUE)
    {
        m_bSummary = FALSE;
        return FALSE;
    }
    return TRUE; // exit map (return to main menu — caller manages)
}

void CAfterGameMenuBehavior::Update(RwReal deltaTime)
{
    if (m_pMaestro != 0)
    {
        Rt2dMaestroUpdate(m_pMaestro, deltaTime);
    }
}

void CAfterGameMenuBehavior::Render()
{
    // ekran pikseli — RenderPlatform_Initialize(1280, 720)
    const RwReal kScreenW = 1280.0f;
    const RwReal kScreenH = 720.0f;
    const RwUInt32 kInk = 0xFF4A2810;
    const RwUInt32 kTitleInk = 0xFF3A1E08;
    const RwUInt32 kBall = 0xFF201008;
    const RwUInt32 kKeyBox = 0xFFE8D8B0;

    RenderPlatform_BeginOverlay2D();
    // Map ground: CAG_Menu_079 (640×480 grid map) fullscreen.
    if (RenderPlatform_HasTexture("CAG_Menu_079") == TRUE)
    {
        RenderPlatform_DrawTexturedQuad(0.0f, 0.0f, kScreenW, kScreenH,
                                        "CAG_Menu_079");
    }
    else
    {
        RenderPlatform_DrawOverlayQuad(0.0f, 0.0f, kScreenW, kScreenH,
                                       0xFFC7A05E);
    }

    if (m_bSummary == TRUE)
    {
        // Summary screen (end-of-level: score/accuracy/rating).
        RenderPlatform_DrawOverlayQuad(kScreenW * 0.2f, kScreenH * 0.2f,
                                       kScreenW * 0.6f, kScreenH * 0.55f,
                                       0xFFC7A05E);
        char line[128];
        ShowText_DrawBitmapText("LEVEL COMPLETE", kScreenW * 0.25f,
                                kScreenH * 0.25f, 4, 0x3A, 0x1E, 0x08, 255);
        std::snprintf(line, sizeof(line), "SCORE %u", m_uScore);
        ShowText_DrawBitmapText(line, kScreenW * 0.25f, kScreenH * 0.42f, 3,
                                0x4A, 0x28, 0x10, 255);
        std::snprintf(line, sizeof(line), "ACCURACY %u", m_uAccuracy);
        ShowText_DrawBitmapText(line, kScreenW * 0.25f, kScreenH * 0.52f, 3,
                                0x4A, 0x28, 0x10, 255);
        std::snprintf(line, sizeof(line), "RATING %u", m_uRating);
        ShowText_DrawBitmapText(line, kScreenW * 0.25f, kScreenH * 0.62f, 3,
                                0x4A, 0x28, 0x10, 255);
    }
    else
    {
        // Title (video: TRICKY TROLL FJORD + rating line).
        ShowText_DrawBitmapText("TRICKY TROLL FJORD", kScreenW * 0.3f,
                                kScreenH * 0.05f, 3, 0x3A, 0x1E, 0x08, 255);
        ShowText_DrawBitmapText("YOUR RATING - NOT COMPLETED",
                                kScreenW * 0.3f, kScreenH * 0.11f, 2,
                                0x4A, 0x28, 0x10, 255);
        // Level list (functionally decodes scene node positions until end;
        // video map cursor scene surplus).
        const RwReal listX = kScreenW * 0.06f;
        const RwReal listY = kScreenH * 0.24f;
        const RwReal rowH = kScreenH * 0.052f;
        const RwInt32 kVisible = 10;
        RwInt32 first = 0;
        if (m_levelCount > kVisible)
        {
            first = m_selection - kVisible / 2;
            if (first < 0)
            {
                first = 0;
            }
            if (first > m_levelCount - kVisible)
            {
                first = m_levelCount - kVisible;
            }
        }
        for (RwInt32 i = 0; i < kVisible && first + i < m_levelCount; ++i)
        {
            const RwInt32 li = first + i;
            const RwReal y = listY + (RwReal)i * rowH;
            if (li == m_selection)
            {
                RenderPlatform_DrawOverlayQuad(listX - 34.0f, y - 4.0f,
                                               24.0f, 28.0f, kBall);
            }
            ShowText_DrawBitmapText(m_levels[li].display, listX, y, 2,
                                    0x4A, 0x28, 0x10, 255);
        }
    }

    // keyboard hints
    RenderPlatform_DrawOverlayQuad(150.0f, 636.0f, 86.0f, 30.0f, kKeyBox);
    ShowText_DrawBitmapText("Enter", 158.0f, 640.0f, 2, 0x3A, 0x1E, 0x08, 255);
    ShowText_DrawBitmapText("SELECT", 244.0f, 640.0f, 2, 0x4A, 0x28, 0x10,
                            255);
    ShowText_DrawBitmapText("Esc", 568.0f, 640.0f, 2, 0x3A, 0x1E, 0x08, 255);
    ShowText_DrawBitmapText("BACK", 626.0f, 640.0f, 2, 0x4A, 0x28, 0x10, 255);
    (void)kInk;
    (void)kTitleInk;

    RenderPlatform_EndOverlay2D();
}

const char* CAfterGameMenuBehavior::GetLevelDisplay(RwInt32 i) const
{
    if (i < 0 || i >= m_levelCount)
    {
        return 0;
    }
    return m_levels[i].display;
}

const char* CAfterGameMenuBehavior::GetLevelFile(RwInt32 i) const
{
    if (i < 0 || i >= m_levelCount)
    {
        return 0;
    }
    return m_levels[i].file;
}

RwInt32 CAfterGameMenuBehavior::TakeSelectedLevel()
{
    const RwInt32 lv = m_pendingLevel;
    m_pendingLevel = -1;
    return lv;
}

