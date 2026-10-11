#ifndef MENUBEHAVIOR_H
#define MENUBEHAVIOR_H

// ============================================================================
// CannonCruise - 2D Maestro Menu Behavior (MenuBehavior.h)
// Original path: D:\Projects\CannonCruisePC\code\MenuBehavior.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include "framework/core/eventhandler/ceventhandler.h"
#include <rwcore.h>
#include <rt2d.h>
#include <rt2danim.h>

/**
 * @class CMenuBehavior
 * @brief RenderWare 2D (Rt2dMaestro) UI menu controller.
 *
 * Hosts and drives interactive RenderWare 2D vector / bitmap animated menus,
 * buttons, and screens using the Rt2dMaestro engine.
 */
class CMenuBehavior : public RWS::CEventHandler
{
public:
    /**
     * @brief Constructs a new CMenuBehavior instance and claims an input focus ID.
     * @see FUN_00443050
     */
    CMenuBehavior();

    /**
     * @brief Destructor. Destroys Rt2dMaestro instance and frees input focus.
     */
    virtual ~CMenuBehavior();

    /**
     * @brief Handles incoming event messages (e.g. MSG_MENU_RENDER_FRONT_RT2D).
     * @param msg Event message container.
     */
    virtual void HandleEvents(const RWS::CMsg& msg);

    /**
     * @brief Ticks 2D menu animation and input dispatch.
     * @param deltaTime Elapsed frame time in seconds.
     * @see FUN_00443800
     */
    void Update(RwReal deltaTime);

    /**
     * @brief Renders 2D menu layout using RenderWare 2D Maestro.
     * @see FUN_00443c20
     */
    void Render();

    /**
     * @brief Retrieves the active Rt2dMaestro object.
     * @return Pointer to Rt2dMaestro or NULL.
     */
    Rt2dMaestro* GetMaestro() const { return m_pMaestro; }

    /**
     * @brief Sahiplenme: parse edilmiş maestro'yu davranışa bağlar
     *        (önceki varsa destroy edilir). Video-kurgu: MAINMENU/
     *        OPTIONS/SOUND/PAUSE durumları aynı Menu.anm'i paylaşır.
     */
    void AttachMaestro(Rt2dMaestro* pMaestro);

    /**
     * @brief Gömülü menü dokularını yükler (RWS TexDictionary → GL).
     *        Menu_074 (640×480 zemin) + Menu_076 (logo) + bar/imleç.
     * @return TRUE (en az bir doku yüklendi).
     */
    RwBool LoadTextures(const char* rwsPath);
    RwBool HasTextures() const;
    /// Video-kurgu menü ekranları (lang_eng.txt ID'leriyle birebir).
    enum EMenuScreen {
        MENUSCREEN_MAIN,      ///< NEW GAME/LOAD GAME/OPTIONS/CREDITS/QUIT GAME
        MENUSCREEN_OPTIONS,   ///< SOUND/DIFFICULTY•x/SINGLE PLAYER
        MENUSCREEN_SOUND,     ///< VOICES/EFFECTS/MUSIC barları
        MENUSCREEN_DIFFICULTY,///< EASY/NORMAL/HARD
        MENUSCREEN_CONFIRMNEW ///< SAVE GAME? (NewSave/NewNoSave)
    };
    /// Terminal eylemler (main.cpp döngüden çıkar).
    enum EMenuAction {
        MENUACTION_NONE, MENUACTION_NEWGAME, MENUACTION_LOADGAME,
        MENUACTION_QUITGAME, MENUACTION_CREDITS
    };

    /// Seçim gezinmesi (W/S/↑/↓) — aktif ekranda; maestro da ilerler.
    RwBool MoveNext();
    RwBool MovePrev();
    /// Ses barı ayarı (A/D/←/→) — SOUND ekranında.
    RwBool AdjustLeft();
    RwBool AdjustRight();
    /// Onay (Enter/Space): ekran geçişi veya terminal eylem. TRUE=döngüden çık.
    RwBool Confirm();
    /// Geri (Esc): üst ekrana; MAIN'de TRUE=döngüden çık (quit sayılmaz).
    RwBool Back();
    /// Onaylanan komutun adı (NULL ise seçim yok).
    const char* GetSelectedCommand() const;
    EMenuAction TakePendingAction();
    EMenuScreen GetScreen() const { return m_screen; }

  private:
    Rt2dMaestro* m_pMaestro;      ///< RenderWare 2D Maestro animation scene manager
    RwInt32      m_iInputFocusID; ///< Input focus registration ID
    EMenuScreen  m_screen;        ///< Aktif video-kurgu ekranı
    RwInt32      m_selection;     ///< Aktif ekranda seçili satır
    RwInt32      m_volVoices;     ///< 0..10 (SpeakVolSet)
    RwInt32      m_volEffects;    ///< 0..10 (EffectVolSet)
    RwInt32      m_volMusic;      ///< 0..10 (MusicVolSet)
    RwInt32      m_difficulty;    ///< 0=EASY 1=NORMAL 2=HARD (SetDifficulty)
    RwInt32      m_multiplayer;   ///< 0=SINGLE 1=TWO (SetMultiPlayer)
    EMenuAction  m_pendingAction; ///< Terminal eylem (Take ile tüketilir)
    RwInt32 ScreenItemCount() const;
};

/**
 * @brief RWS gömülü TexDictionary'yi direkt okur (0xe8 sabit düzen —
 *        MAINMENU/MAPMENU/GLOBAL/LANGSEL/seviyeler) ve dokuları GL'e yükler.
 *        RwStudio ayrıştırıcısına girmez (MAPMENU'yu atlar).
 * @return TRUE (en az bir doku yüklendi).
 */
RwBool MenuTextures_LoadFromRws(const char* rwsPath);

#endif // MENUBEHAVIOR_H
