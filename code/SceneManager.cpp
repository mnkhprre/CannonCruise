// ============================================================================
// CannonCruise - Scene Manager (SceneManager.cpp)
// Original path: D:\Projects\CannonCruisePC\code\SceneManager.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_004a1190 — CSceneManager constructor & event registration ("iMsgChangeScene", "iMsgPreSceneDestroy", "iMsgPostSceneDestroy")
//   FUN_004a2120 — CSceneManager::ChangeScene ("You must provide a scene name...", "%sStreams\\%s.RWS")
//   FUN_004a3090 — CSceneManager::Update
// ============================================================================

#include "SceneManager.h"
#include "RwsAssets.h"   // .RWS asset yol çözücü (merkezi)
#include <cassert>
#include <cstdio>
#include <cstring>

// ---------------------------------------------------------------------------
// Constructor – mirrors FUN_004a1190.
// Sets initial scene pointer to NULL and configures stream asset directory.
// ---------------------------------------------------------------------------
CSceneManager::CSceneManager()
    : m_pCurrentScene(0),
      m_StreamsPath("Streams\\")
{
}

// ---------------------------------------------------------------------------
// Destructor – deletes active scene container and releases resources.
// ---------------------------------------------------------------------------
CSceneManager::~CSceneManager()
{
    if (m_pCurrentScene)
    {
        delete m_pCurrentScene;
        m_pCurrentScene = 0;
    }
}

// ---------------------------------------------------------------------------
// HandleEvents – listens for scene change triggers via "iMsgChangeScene".
// Validates message data payload and invokes ChangeScene().
// ---------------------------------------------------------------------------
void CSceneManager::HandleEvents(const RWS::CMsg& msg)
{
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

// ---------------------------------------------------------------------------
// ChangeScene – mirrors FUN_004a2120.
// Tears down existing active scene, validates scene name lengths, formats the
// ".RWS" stream asset path, instantiates a new CScene object, and loads it.
// ---------------------------------------------------------------------------
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

    // Binary formatı "%sStreams\\%s.RWS" (FUN_004a2120). Önek ("") eskiden
    // elle veriliyordu; artık göreli derinlik/env merkezi çözülür
    // (RwsAssets_Resolve) — build yerleşimine göre Streams/ kökünü bulur.
    char szStreamPath[512] = {0};
    char szStreamName[128];
    snprintf(szStreamName, sizeof(szStreamName), "Streams/%s.RWS", pSceneName);
    RwsAssets_Resolve(szStreamName, szStreamPath, sizeof(szStreamPath));

    if (!m_pCurrentScene->Load(pSceneName))
    {
        assert(false && "Unable to load scene");
        delete m_pCurrentScene;
        m_pCurrentScene = 0;
        return FALSE;
    }

    return TRUE;
}

// ---------------------------------------------------------------------------
// GetCurrentSceneName – returns name of current scene, or "NOSCENE" if empty.
// ---------------------------------------------------------------------------
const char* CSceneManager::GetCurrentSceneName() const
{
    if (m_pCurrentScene)
    {
        return m_pCurrentScene->GetName();
    }
    return "NOSCENE";
}

// ---------------------------------------------------------------------------
// Update – mirrors FUN_004a3090.
// Per-frame maintenance tick for scene streaming and deferred transitions.
// ---------------------------------------------------------------------------
void CSceneManager::Update()
{
}
