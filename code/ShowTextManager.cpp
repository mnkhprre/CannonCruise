// ============================================================================
// CannonCruise - Show Text Manager (ShowTextManager.cpp)
// Original path: D:\Projects\CannonCruisePC\code\ShowTextManager.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_004b5e90 — CShowTextManager constructor & registration
//   FUN_004b64a0 — CShowTextManager::FormatString
// ============================================================================

#include "ShowTextManager.h"
#include <cassert>
#include <cstring>

CShowTextManager::CShowTextManager()
{
    memset(m_sTempString, 0, sizeof(m_sTempString));
}

CShowTextManager::~CShowTextManager()
{
    m_TextQueue.clear();
}

void CShowTextManager::HandleEvents(const RWS::CMsg& msg)
{
    // MSG_GET_TAIL_OF_SHOW_TEXT_QUEUE
}

void CShowTextManager::QueueText(const char* pText, RwReal duration)
{
    if (pText)
    {
        m_TextQueue.push_back(pText);
    }
}

void CShowTextManager::FormatString(const char* pInput, RwUInt16* pOutputBuffer, RwUInt32 maxLen)
{
    if (!pInput || !pOutputBuffer || maxLen == 0) return;

    size_t len = strlen(pInput);
    assert(len < ((sizeof(m_sTempString) / sizeof(RwUInt16)) - 2) && "iStringLength < ((sizeof(m_sTempString) / sizeof(RwUInt16)) - 2) Failed");
    assert(len < maxLen && "Line too long.");

    for (size_t i = 0; i < len; ++i)
    {
        pOutputBuffer[i] = (RwUInt16)(unsigned char)pInput[i];
    }
    pOutputBuffer[len] = 0;
}

void CShowTextManager::Update(RwReal deltaTime)
{
}
