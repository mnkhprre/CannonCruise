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

/**
 * @brief Constructs a new CShowTextManager singleton instance.
 *
 * Initializes the wide character scratchpad buffer.
 *
 * @see FUN_004b5e90
 */
CShowTextManager::CShowTextManager()
{
    memset(m_sTempString, 0, sizeof(m_sTempString));
}

/**
 * @brief Destructor. Clears pending text messages.
 */
CShowTextManager::~CShowTextManager()
{
    m_TextQueue.clear();
}

/**
 * @brief Handles incoming event messages.
 * @param msg Event message container.
 */
void CShowTextManager::HandleEvents(const RWS::CMsg& msg)
{
    // MSG_GET_TAIL_OF_SHOW_TEXT_QUEUE
}

/**
 * @brief Pushes a new text message onto the playback FIFO queue.
 *
 * @param pText Null-terminated string to queue.
 * @param duration Visibility lifespan in seconds.
 */
void CShowTextManager::QueueText(const char* pText, RwReal duration)
{
    if (pText)
    {
        m_TextQueue.push_back(pText);
    }
}

/**
 * @brief Transcodes an ASCII string into 16-bit wide character units with bounds checks.
 *
 * Validates length against internal scratchpad buffer and output target buffer.
 *
 * @param pInput Source null-terminated string.
 * @param pOutputBuffer Destination wide character buffer.
 * @param maxLen Maximum characters to write (including null terminator).
 * @see FUN_004b64a0
 */
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

/**
 * @brief Ticks queue timer and displays next pending message if available.
 * @param deltaTime Frame time step in seconds.
 */
void CShowTextManager::Update(RwReal deltaTime)
{
}

