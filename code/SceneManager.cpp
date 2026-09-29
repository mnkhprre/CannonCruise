// ============================================================================
// CannonCruise - Scene Manager (SceneManager.cpp)
// Original path: D:\Projects\CannonCruisePC\code\SceneManager.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_004a2120 — CSceneManager::ChangeScene
//   FUN_004a3090 — CSceneManager::Update
// ============================================================================

#include "SceneManager.h"
#include <cassert>
#include <cstdio>
#include <cstring>

CSceneManager::CSceneManager()
    : m_pCurrentScene(0),
      m_StreamsPath("Streams\\")
{
}

CSceneManager::~CSceneManager()
{
    if (m_pCurrentScene)
    {
        delete m_pCurrentScene;
        m_pCurrentScene = 0;
    }
}

void CSceneManager::HandleEvents(const RWS::CMsg& msg)
{
    // iMsgChangeScene olayı alındığında
    if (msg.m_pData)
    {
        const char* pSceneName = (const char*)msg.m_pData;
        ChangeScene(pSceneName);
    }
    else
    {
        assert(false && "You must provide a scene name with the iMsgChangeScene event");
    }
}

// ============================================================================
// CSceneManager::ChangeScene (FUN_004a2120)
// ASM: 004a2120 — Sahneyi kapat, yeni .RWS akışını yükle ve başlat
// ============================================================================
RwBool CSceneManager::ChangeScene(const char* pSceneName)
{
    assert(pSceneName != 0 && "You must provide a scene name with the iMsgChangeScene event");
    assert(strlen(pSceneName) < 64 && "Scene name too long");

    if (m_pCurrentScene)
    {
        delete m_pCurrentScene;
        m_pCurrentScene = 0;
    }

    if (strcmp(pSceneName, "NOSCENE") == 0 || strcmp(pSceneName, "NoScene") == 0)
    {
        return TRUE;
    }

    m_pCurrentScene = new CScene();
    assert(m_pCurrentScene != 0 && "Scene not created");

    char szStreamPath[256];
    snprintf(szStreamPath, sizeof(szStreamPath), "%%sStreams\\%%s.RWS", "", pSceneName);

    if (!m_pCurrentScene->Load(pSceneName))
    {
        assert(false && "Unable to load scene");
        delete m_pCurrentScene;
        m_pCurrentScene = 0;
        return FALSE;
    }

    return TRUE;
}

const char* CSceneManager::GetCurrentSceneName() const
{
    if (m_pCurrentScene)
    {
        return m_pCurrentScene->GetName();
    }
    return "NOSCENE";
}

void CSceneManager::Update()
{
}
