// ============================================================================
// CannonCruise - Scene Container (Scene.cpp)
// Original path: D:\Projects\CannonCruisePC\code\Scene.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_0049f100 — CScene::Load
//   FUN_0049f6a0 — CScene::Unload
// ============================================================================

#include "Scene.h"
#include "InputManager.h"
#include <cassert>

CScene::CScene()
    : m_pWorld(0),
      m_iGameInputFocusID(-1)
{
}

CScene::~CScene()
{
    Unload();
}

void CScene::HandleEvents(const RWS::CMsg& msg)
{
}

// ============================================================================
// CScene::Load (FUN_0049f100)
// ASM: 0049f100 — Sahneyi yükle, dünyayı oluştur ve giriş odağını al
// ============================================================================
RwBool CScene::Load(const char* pSceneName)
{
    if (!pSceneName) return FALSE;

    m_SceneName = pSceneName;

    // Giriş odak kontrolü
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

    // RpWorld oluşturma
    RwBBox bbox;
    bbox.inf.x = -10000.0f;
    bbox.inf.y = -1000.0f;
    bbox.inf.z = -10000.0f;
    bbox.sup.x = 10000.0f;
    bbox.sup.y = 1000.0f;
    bbox.sup.z = 10000.0f;

    m_pWorld = RpWorldCreate(&bbox);
    assert(m_pWorld != 0 && "Failed to create WaveBreaker world");

    return TRUE;
}

// ============================================================================
// CScene::Unload (FUN_0049f6a0)
// ASM: 0049f6a0 — Dünyayı yok et ve giriş odağını bırak
// ============================================================================
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
        RpWorldDestroy(m_pWorld);
        m_pWorld = 0;
    }

    m_SceneName.clear();
}
