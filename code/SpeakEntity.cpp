// ============================================================================
// CannonCruise - Speech Audio Entity (SpeakEntity.cpp)
// Original path: D:\Projects\CannonCruisePC\code\SpeakEntity.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_0045a6d0 — CSpeakEntity constructor & registration
//   FUN_0045ab00 — CSpeakEntity::TriggerSpeak
// ============================================================================

#include "SpeakEntity.h"
#include "SpeakManager.h"
#include "framework/core/attributehandler/cclassfactory.h"
#include <cassert>

RWS_REGISTER_CLASS(CSpeakEntity, []() -> void* { return new CSpeakEntity(); }, sizeof(CSpeakEntity));

CSpeakEntity::CSpeakEntity()
    : m_iCategory(1),
      m_iCategoryIndex(0)
{
}

CSpeakEntity::~CSpeakEntity()
{
}

void CSpeakEntity::HandleEvents(const RWS::CMsg& msg)
{
}

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
