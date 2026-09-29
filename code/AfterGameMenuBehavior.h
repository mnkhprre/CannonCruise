#ifndef AFTERGAMEMENUBEHAVIOR_H
#define AFTERGAMEMENUBEHAVIOR_H

// ============================================================================
// CannonCruise - After Game Menu Behavior (AfterGameMenuBehavior.h)
// Original path: D:\Projects\CannonCruisePC\code\AfterGameMenuBehavior.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include "framework/core/eventhandler/ceventhandler.h"
#include <rwcore.h>

class CAfterGameMenuBehavior : public RWS::CEventHandler
{
public:
    CAfterGameMenuBehavior();
    virtual ~CAfterGameMenuBehavior();

    virtual void HandleEvents(const RWS::CMsg& msg);

    void ShowSummary(RwUInt32 score, RwUInt32 accuracy, RwUInt32 rating);

private:
    RwUInt32 m_uScore;
    RwUInt32 m_uAccuracy;
    RwUInt32 m_uRating;
};

#endif // AFTERGAMEMENUBEHAVIOR_H
