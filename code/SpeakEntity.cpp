// ============================================================================
// CannonCruise - Speech Audio Entity (SpeakEntity.cpp)
// Original path: D:\Projects\CannonCruisePC\code\SpeakEntity.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_0045a6d0 — CSpeakEntity constructor & registration
//   FUN_0045ab00 — CSpeakEntity::TriggerSpeak ("CSpeakManager::GetSingleton()->IsSpeakValid(m_iCategory, m_iCategoryIndex) Failed")
// ============================================================================

#include "SpeakEntity.h"
#include "SpeakManager.h"
#include "framework/core/attributehandler/cclassfactory.h"
#include <cassert>

// Register CSpeakEntity with RenderWare Studio reflection system
RWS_REGISTER_CLASS(CSpeakEntity, []() -> void* { return new CSpeakEntity(); }, sizeof(CSpeakEntity));

// ---------------------------------------------------------------------------
// Constructor – mirrors FUN_0045a6d0.
// Initializes category to 1 and line index to 0.
// ---------------------------------------------------------------------------
CSpeakEntity::CSpeakEntity()
    : m_iCategory(1),
      m_iCategoryIndex(0)
{
}

// ---------------------------------------------------------------------------
// Destructor.
// ---------------------------------------------------------------------------
CSpeakEntity::~CSpeakEntity()
{
}

// ---------------------------------------------------------------------------
// HandleEvents – framework message listener.
// ---------------------------------------------------------------------------
void CSpeakEntity::HandleEvents(const RWS::CMsg& msg)
{
}

// ---------------------------------------------------------------------------
// TriggerSpeak – mirrors FUN_0045ab00.
// Validates speech parameters with CSpeakManager and plays the line.
// ---------------------------------------------------------------------------
void CSpeakEntity::TriggerSpeak()
{
    CSpeakManager* pSpeakMgr = CSpeakManager::GetSingletonPtr();
    if (pSpeakMgr)
    {
        assert(pSpeakMgr->IsSpeakValid(m_iCategory, m_iCategoryIndex) &&
               "CSpeakManager::GetSingleton()->IsSpeakValid(m_iCategory, m_iCategoryIndex) Failed");

        pSpeakMgr->PlaySpeak(m_iCategory, m_iCategoryIndex);
    }
}
