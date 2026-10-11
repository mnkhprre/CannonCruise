#ifndef SHOWTEXTMANAGER_H
#define SHOWTEXTMANAGER_H

// ============================================================================
// CannonCruise - Show Text Manager (ShowTextManager.h)
// Original path: D:\Projects\CannonCruisePC\code\ShowTextManager.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include "Singleton.h"
#include "framework/core/eventhandler/ceventhandler.h"
#include <rwcore.h>
#include <vector>
#include <string>

/**
 * @class CShowTextManager
 * @brief Singleton text prompt queue and string formatting manager.
 *
 * Manages FIFO queuing of on-screen notifications/subtitles and converts
 * ASCII/ANSI character strings into Unicode (RwUInt16) font buffers.
 */
class CShowTextManager : public CSingleton<CShowTextManager>, public RWS::CEventHandler
{
public:
    /**
     * @brief Constructs a new CShowTextManager instance.
     * @see FUN_004b5e90
     */
    CShowTextManager();

    /**
     * @brief Destructor. Clears the text queue.
     */
    virtual ~CShowTextManager();

    /**
     * @brief Handles incoming event messages (e.g. MSG_GET_TAIL_OF_SHOW_TEXT_QUEUE).
     * @param msg Event message container.
     */
    virtual void HandleEvents(const RWS::CMsg& msg);

    /**
     * @brief Appends a text line to the subtitle playback FIFO queue.
     * @param pText Character string to enqueue.
     * @param duration Display time in seconds.
     */
    void QueueText(const char* pText, RwReal duration);

    /**
     * @brief Converts an 8-bit character string into a 16-bit wide character glyph buffer.
     * @param pInput Source null-terminated string.
     * @param pOutputBuffer Destination wide character array (RwUInt16).
     * @param maxLen Maximum character capacity of output buffer.
     * @see FUN_004b64a0
     */
    void FormatString(const char* pInput, RwUInt16* pOutputBuffer, RwUInt32 maxLen);

    /**
     * @brief Updates timer for active text item and advances queue on expiration.
     * @param deltaTime Elapsed frame time in seconds.
     */
    void Update(RwReal deltaTime);

private:
    RwUInt16                 m_sTempString[512]; ///< Intermediate 16-bit formatted string buffer
    std::vector<std::string> m_TextQueue;        ///< Queue of pending subtitle messages
};

#endif // SHOWTEXTMANAGER_H
