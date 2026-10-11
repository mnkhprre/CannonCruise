#ifndef SCENE_H
#define SCENE_H

// ============================================================================
// CannonCruise - Scene Container (Scene.h)
// Original path: D:\Projects\CannonCruisePC\code\Scene.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include "framework/core/eventhandler/ceventhandler.h"
#include <rwcore.h>
#include <rpworld.h>
#include <string>

/**
 * @class CScene
 * @brief Manages the active level environment, world bounding volume, and input context.
 *
 * Encapsulates the RenderWare RpWorld instance for the loaded level, sets up
 * expansive world bounds ([-10000..+10000]), claims game input focus from
 * CInputManager via `m_iGameInputFocusID`, and provides static access to the
 * active simulation world for entities to add/remove clumps and atomics.
 */
class CScene : public RWS::CEventHandler
{
public:
    /**
     * @brief Constructs an empty scene with invalid focus ID (-1) and NULL world.
     */
    CScene();

    /**
     * @brief Destructor – ensures scene is unloaded and world resources freed.
     */
    virtual ~CScene();

    /**
     * @brief Framework message dispatcher.
     * @param msg  Message descriptor.
     */
    virtual void HandleEvents(const RWS::CMsg& msg);

    // ---- Scene Lifecycle ----------------------------------------------------

    /**
     * @brief Allocates an RpWorld, acquires input focus, and prepares the level.
     * @param pSceneName  Identifier name of the level/scene to load.
     * @return TRUE if scene was successfully initialized, FALSE otherwise.
     */
    RwBool Load(const char* pSceneName);

    /**
     * @brief Releases input focus, destroys the RpWorld, and clears scene state.
     */
    void Unload();

    /**
     * @brief Retrieves the global active world pointer.
     * @return Pointer to current RpWorld.
     */
    static RpWorld* GetWorld();

    /**
     * @brief Retrieves this scene instance's RpWorld pointer.
     * @return Pointer to owned RpWorld.
     */
    RpWorld* GetSceneWorld() const { return m_pWorld; }

    /**
     * @brief Returns the name string of the currently loaded scene.
     * @return C-string representation of scene name.
     */
    const char* GetName() const { return m_SceneName.c_str(); }

private:
    static RpWorld* sm_pActiveWorld; ///< Global active world reference for game actors.
    std::string m_SceneName;         ///< Name identifier of the currently loaded scene.
    RpWorld* m_pWorld;               ///< RenderWare BSP/spatial world container.
    RwInt32 m_iGameInputFocusID;     ///< Input focus identifier claimed by gameplay mode.
};

#endif // SCENE_H
