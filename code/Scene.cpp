// ============================================================================
// CannonCruise - Scene Container (Scene.cpp)
// Original path: D:\Projects\CannonCruisePC\code\Scene.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_0049f100 — CScene::Load ("m_iGameInputFocusID == -1 Failed", "Failed to create WaveBreaker world")
//   FUN_0049f6a0 — CScene::Unload
// ============================================================================

#include "Scene.h"
#include "InputManager.h"
#include <cassert>

// Global pointer to the currently active RenderWare simulation world
RpWorld* CScene::sm_pActiveWorld = 0;

// ---------------------------------------------------------------------------
// GetWorld – static accessor providing active RpWorld to actors, emitters, etc.
// ---------------------------------------------------------------------------
RpWorld* CScene::GetWorld()
{
    return sm_pActiveWorld;
}

// ---------------------------------------------------------------------------
// Constructor – initializes state variables to NULL / unallocated.
// ---------------------------------------------------------------------------
CScene::CScene()
    : m_pWorld(0),
      m_iGameInputFocusID(-1)
{
}

// ---------------------------------------------------------------------------
// Destructor – unloads scene and releases world if still active.
// ---------------------------------------------------------------------------
CScene::~CScene()
{
    Unload();
}

// ---------------------------------------------------------------------------
// HandleEvents – framework message listener for scene lifecycle events.
// ---------------------------------------------------------------------------
void CScene::HandleEvents(const RWS::CMsg& msg)
{
}

// ---------------------------------------------------------------------------
// Load – mirrors FUN_0049f100.
// Acquires gameplay modal input focus from CInputManager, configures an expansive
// world bounding box ([-10000, 10000]), and allocates the RpWorld container.
// ---------------------------------------------------------------------------
RwBool CScene::Load(const char* pSceneName)
{
    if (!pSceneName) return FALSE;

    m_SceneName = pSceneName;

    // Check and reserve game input focus
    assert(m_iGameInputFocusID == -1 && "m_iGameInputFocusID == -1 Failed");

    CInputManager* pInputMgr = CInputManager::GetSingletonPtr();
    if (pInputMgr)
    {
        m_iGameInputFocusID = pInputMgr->AllocateFocusID();
        if (m_iGameInputFocusID != -1)
        {
            pInputMgr->PushFocusID(m_iGameInputFocusID);
        }
    }

    assert(m_iGameInputFocusID != -1 && "m_iGameInputFocusID != -1 Failed");

    // Construct RpWorld bounding envelope
    RwBBox bbox;
    bbox.inf.x = -10000.0f;
    bbox.inf.y = -1000.0f;
    bbox.inf.z = -10000.0f;
    bbox.sup.x = 10000.0f;
    bbox.sup.y = 1000.0f;
    bbox.sup.z = 10000.0f;

    m_pWorld = RpWorldCreate(&bbox);
    assert(m_pWorld != 0 && "Failed to create WaveBreaker world");

    sm_pActiveWorld = m_pWorld;

    return TRUE;
}

// ---------------------------------------------------------------------------
// Unload – mirrors FUN_0049f6a0.
// Pops and deallocates game input focus, tears down the RpWorld instance,
// and resets static active world pointer.
// ---------------------------------------------------------------------------
void CScene::Unload()
{
    if (m_iGameInputFocusID != -1)
    {
        CInputManager* pInputMgr = CInputManager::GetSingletonPtr();
        if (pInputMgr)
        {
            pInputMgr->PopFocusID(m_iGameInputFocusID);
            pInputMgr->FreeFocusID(m_iGameInputFocusID);
        }
        m_iGameInputFocusID = -1;
    }

    if (m_pWorld)
    {
        if (sm_pActiveWorld == m_pWorld)
        {
            sm_pActiveWorld = 0;
        }
        RpWorldDestroy(m_pWorld);
        m_pWorld = 0;
    }

    m_SceneName.clear();
}
