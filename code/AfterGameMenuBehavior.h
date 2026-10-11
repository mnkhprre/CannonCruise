#ifndef AFTERGAMEMENUBEHAVIOR_H
#define AFTERGAMEMENUBEHAVIOR_H

// ============================================================================
// CannonCruise - After Game Menu Behavior (AfterGameMenuBehavior.h)
// Original path: D:\Projects\CannonCruisePC\code\AfterGameMenuBehavior.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include "framework/core/eventhandler/ceventhandler.h"
#include <rwcore.h>
#include <rt2danim.h>

/**
 * @class CAfterGameMenuBehavior
 * @brief Post-mission debriefing and victory/defeat scoreboard UI handler.
 *
 * Displays end-of-level performance ratings, total score, shot accuracy percentages,
 * and handles transition back to map/level selection screens.
 *
 * Video-kurgu (SCR.mov): MAPMENU harita ekranı (CAG_Menu.anm) — seviye
 * seçimi (MapMenuEnter) + özet ekranı (skor/isabet/derece basamakları).
 */
class CAfterGameMenuBehavior : public RWS::CEventHandler
{
public:
    /**
     * @brief Constructs a new CAfterGameMenuBehavior instance.
     * @see FUN_0043cb30
     */
    CAfterGameMenuBehavior();

    /**
     * @brief Destructor.
     */
    virtual ~CAfterGameMenuBehavior();

    /**
     * @brief Handles post-game menu messages (e.g. MENUTRIGGER_MapMenuEnter, MapMenuRatingInquire).
     * @param msg Event message container.
     */
    virtual void HandleEvents(const RWS::CMsg& msg);

    /**
     * @brief Updates the mission summary metrics to present on the scoreboard.
     * @param score Final accumulated point score.
     * @param accuracy Cannon shot hit-to-fire accuracy percentage.
     * @param rating Star/tier achievement ranking metric.
     * @see FUN_0043d9d0
     */
    void ShowSummary(RwUInt32 score, RwUInt32 accuracy, RwUInt32 rating);

    /// Maestro sahiplenme (CAG_Menu.anm) — seviye grupları taranır.
    void AttachMaestro(Rt2dMaestro* pMaestro);
    Rt2dMaestro* GetMaestro() const { return m_pMaestro; }

    /// Harita gezgin (W/S/↑/↓) + maestro ilerletme.
    RwBool MoveNext();
    RwBool MovePrev();
    /// Onay (Enter): MapMenuEnter → bekleyen seviye. TRUE=terminal.
    RwBool Confirm();
    /// Geri (Esc): TRUE=döngüden çık.
    RwBool Back();
    void Update(RwReal deltaTime);
    void Render();

    RwInt32 GetLevelCount() const { return m_levelCount; }
    const char* GetLevelDisplay(RwInt32 i) const;
    /// Seviye dosya çekirdeği ("VIKING01"); NULL geçersiz.
    const char* GetLevelFile(RwInt32 i) const;
    /// Onaylanan seviye (Take ile tüketilir, -1 yok).
    RwInt32 TakeSelectedLevel();
    RwBool HasTextures() const;

private:
    struct MapLevel
    {
        char display[32]; ///< "VIKING 1"
        char file[16];    ///< "VIKING01"
    };
    enum { MAX_LEVELS = 16 };

    Rt2dMaestro* m_pMaestro;
    MapLevel     m_levels[MAX_LEVELS];
    RwInt32      m_levelCount;
    RwInt32      m_selection;
    RwInt32      m_pendingLevel; ///< onaylanan (Take ile tüketilir)
    RwBool       m_bSummary;     ///< özet ekranı modu
    RwUInt32 m_uScore;    ///< Final mission score
    RwUInt32 m_uAccuracy; ///< Accuracy percentage [0, 100]
    RwUInt32 m_uRating;   ///< Performance grade / star rating
};

#endif // AFTERGAMEMENUBEHAVIOR_H
