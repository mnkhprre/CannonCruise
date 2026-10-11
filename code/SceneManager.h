#ifndef SCENEMANAGER_H
#define SCENEMANAGER_H

// ============================================================================
// CannonCruise - Scene Manager (SceneManager.h)
// Original path: D:\Projects\CannonCruisePC\code\SceneManager.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include "Singleton.h"
#include "framework/core/eventhandler/ceventhandler.h"
#include "Scene.h"
#include <string>

/**
 * @class CSceneManager
 * @brief Singleton coordinating scene transitions and stream resource management.
 *
 * Listens for system transition messages (`iMsgChangeScene`, `iMsgPreSceneDestroy`,
 * `iMsgPostSceneDestroy`). Manages the lifetime of the active CScene, loads
 * packaged `.RWS` stream assets from the "Streams\" directory, and supports
 * transitional placeholder modes like "NOSCENE".
 */
class CSceneManager : public CSingleton<CSceneManager>, public RWS::CEventHandler
{
public:
    /**
     * @brief Constructs scene manager singleton, defaulting streams directory to "Streams\".
     */
    CSceneManager();

    /**
     * @brief Destructor – cleans up active scene and tears down registered event listeners.
     */
    virtual ~CSceneManager();

    /**
     * @brief Event message dispatcher (handles iMsgChangeScene).
     * @param msg  Message descriptor carrying target scene name in data payload.
     */
    virtual void HandleEvents(const RWS::CMsg& msg);

    // ---- Scene Transitions --------------------------------------------------

    /**
     * @brief Unloads current level and mounts new level scene from stream archive.
     * @param pSceneName  Identifier name of the level to load (e.g. "Level01", "NOSCENE").
     * @return TRUE on success, FALSE if stream fails to open or initialize.
     */
    RwBool ChangeScene(const char* pSceneName);

    /**
     * @brief Per-frame update tick for scheduled transitions and resource streaming.
     */
    void Update();

    /**
     * @brief Returns pointer to the currently active scene container.
     * @return Pointer to current CScene, or NULL if none loaded.
     */
    CScene* GetCurrentScene() const { return m_pCurrentScene; }

    /**
     * @brief Returns the name string of the current level or "NOSCENE".
     * @return C-string representation of current scene.
     */
    const char* GetCurrentSceneName() const;

private:
    CScene* m_pCurrentScene;        ///< Pointer to dynamically allocated active scene instance.
    std::string m_NextSceneName;    ///< Target scene name queued for deferred transition.
    std::string m_StreamsPath;      ///< Base relative directory path for .RWS stream assets.
};

#endif // SCENEMANAGER_H
