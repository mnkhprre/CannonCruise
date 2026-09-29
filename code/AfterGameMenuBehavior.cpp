// ============================================================================
// CannonCruise - After Game Menu Behavior (AfterGameMenuBehavior.cpp)
// Original path: D:\Projects\CannonCruisePC\code\AfterGameMenuBehavior.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_0043cb30 — CAfterGameMenuBehavior constructor & registration
//   FUN_0043d9d0 — CAfterGameMenuBehavior::ShowSummary
// ============================================================================

#include "AfterGameMenuBehavior.h"
#include "framework/core/attributehandler/cclassfactory.h"
#include <cassert>

RWS_REGISTER_CLASS(CAfterGameMenuBehavior, []() -> void* { return new CAfterGameMenuBehavior(); }, sizeof(CAfterGameMenuBehavior));

CAfterGameMenuBehavior::CAfterGameMenuBehavior()
    : m_uScore(0),
      m_uAccuracy(0),
      m_uRating(0)
{
}

CAfterGameMenuBehavior::~CAfterGameMenuBehavior()
{
}

void CAfterGameMenuBehavior::HandleEvents(const RWS::CMsg& msg)
{
    // MENUTRIGGER_MapMenuEnter, MapMenuRatingInquire vb.
}

void CAfterGameMenuBehavior::ShowSummary(RwUInt32 score, RwUInt32 accuracy, RwUInt32 rating)
{
    m_uScore = score;
    m_uAccuracy = accuracy;
    m_uRating = rating;
}
