#ifndef SPEAKMANAGER_H
#define SPEAKMANAGER_H

// ============================================================================
// CannonCruise - Speech Manager (SpeakManager.h)
// Original path: D:\Projects\CannonCruisePC\code\SpeakManager.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include "Singleton.h"
#include "framework/core/eventhandler/ceventhandler.h"
#include "SpeakStreamDefs.h"
#include <rwcore.h>
#include <string>

/**
 * @class CSpeakManager
 * @brief Singleton manager coordinating voice lines, battle shouts, and dialogue lines.
 *
 * Organizes spoken character lines into numbered categories (1..SPEAK_MAX_NUM_CATEGORIES)
 * and line indices within each category. Validates category/index bounds against
 * CSpeakStreamDefs definitions, loads formatted wave streams ("Sound\\Win32\\Speaks\\%d.%d.wav"),
 * and stops or updates speech states.
 */
class CSpeakManager : public CSingleton<CSpeakManager>, public RWS::CEventHandler
{
public:
    /**
     * @brief Constructs speech manager singleton and initializes speak category tables.
     */
    CSpeakManager();

    /**
     * @brief Destructor – halts any active speech stream.
     */
    virtual ~CSpeakManager();

    /**
     * @brief Framework message dispatcher.
     * @param msg  Message descriptor.
     */
    virtual void HandleEvents(const RWS::CMsg& msg);

    /**
     * @brief Validates category and line index against definition table limits.
     * @param iCategory  Voice category identifier [1..SPEAK_MAX_NUM_CATEGORIES].
     * @param iIndex     Line index within category [0..count-1].
     * @return TRUE if valid dialogue entry exists, FALSE otherwise.
     */
    RwBool IsSpeakValid(RwInt32 iCategory, RwInt32 iIndex) const;

    /**
     * @brief Plays a dialogue voice line with optional 3D spatial positioning.
     * @param iCategory  Voice line category ID.
     * @param iIndex     Voice line variation index.
     * @param pPos       World space position coordinates, or NULL for 2D voiceover.
     */
    void PlaySpeak(RwInt32 iCategory, RwInt32 iIndex, const RwV3d* pPos = 0);
    void PlaySpeech(RwInt32 speakId) { PlaySpeak(speakId, 0); }

    /**
     * @brief Interrupts and stops currently playing voice line.
     */
    void StopSpeak();

private:
    std::string m_SpeaksPath;       ///< Root directory for speech audio assets.
    RwInt32 m_iCurrentCategory;     ///< Category of currently active voice line (-1 if idle).
    RwInt32 m_iCurrentIndex;        ///< Index of currently active voice line (-1 if idle).
};

#endif // SPEAKMANAGER_H
