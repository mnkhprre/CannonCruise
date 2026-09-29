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

class CSceneManager : public CSingleton<CSceneManager>, public RWS::CEventHandler
{
public:
    CSceneManager();
    virtual ~CSceneManager();

    virtual void HandleEvents(const RWS::CMsg& msg);

    // Sahne Değiştirme
    RwBool ChangeScene(const char* pSceneName);
    void Update();

    CScene* GetCurrentScene() const { return m_pCurrentScene; }
    const char* GetCurrentSceneName() const;

private:
    CScene* m_pCurrentScene;
    std::string m_NextSceneName;
    std::string m_StreamsPath;
};

#endif // SCENEMANAGER_H
