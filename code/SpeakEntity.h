#ifndef SPEAKENTITY_H
#define SPEAKENTITY_H

// ============================================================================
// CannonCruise - Speech Audio Entity (SpeakEntity.h)
// Original path: D:\Projects\CannonCruisePC\code\SpeakEntity.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include "framework/core/eventhandler/ceventhandler.h"
#include <rwcore.h>

/**
 * @class CSpeakEntity
 * @brief Level entity configured to trigger contextual voice lines upon game events.
 *
 * Placed in levels or attached to narrative triggers. Stores a configured
 * category and index, and invokes CSpeakManager::PlaySpeak() upon activation.
 */
class CSpeakEntity : public RWS::CEventHandler
{
public:
    /**
     * @brief Constructs speech entity defaulting to category 1, index 0.
     */
    CSpeakEntity();

    /**
     * @brief Destructor.
     */
    virtual ~CSpeakEntity();

    /**
     * @brief Framework message dispatcher.
     * @param msg  Message descriptor.
     */
    virtual void HandleEvents(const RWS::CMsg& msg);

    /**
     * @brief Validates category and index and plays speech through CSpeakManager.
     */
    void TriggerSpeak();

private:
    RwInt32 m_iCategory;        ///< Voice category identifier (default 1).
    RwInt32 m_iCategoryIndex;   ///< Voice line index within category (default 0).
};

#endif // SPEAKENTITY_H
