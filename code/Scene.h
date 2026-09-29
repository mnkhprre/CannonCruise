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

class CScene : public RWS::CEventHandler
{
public:
    CScene();
    virtual ~CScene();

    virtual void HandleEvents(const RWS::CMsg& msg);

    // Sahne Yaşam Döngüsü
    RwBool Load(const char* pSceneName);
    void Unload();

    RpWorld* GetWorld() const { return m_pWorld; }
    const char* GetName() const { return m_SceneName.c_str(); }

private:
    std::string m_SceneName;
    RpWorld* m_pWorld;
    RwInt32 m_iGameInputFocusID;
};

#endif // SCENE_H
