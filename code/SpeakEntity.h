#ifndef SPEAKENTITY_H
#define SPEAKENTITY_H

// ============================================================================
// CannonCruise - Speech Audio Entity (SpeakEntity.h)
// Original path: D:\Projects\CannonCruisePC\code\SpeakEntity.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include "framework/core/eventhandler/ceventhandler.h"
#include <rwcore.h>

class CSpeakEntity : public RWS::CEventHandler
{
public:
    CSpeakEntity();
    virtual ~CSpeakEntity();

    virtual void HandleEvents(const RWS::CMsg& msg);

    void TriggerSpeak();

private:
    RwInt32 m_iCategory;
    RwInt32 m_iCategoryIndex;
};

#endif // SPEAKENTITY_H
