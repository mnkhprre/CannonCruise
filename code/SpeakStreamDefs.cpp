// ============================================================================
// CannonCruise - Speech Stream Definitions (SpeakStreamDefs.cpp)
// Original path: D:\Projects\CannonCruisePC\code\SpeakStreamDefs.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_0045d7b0 — CSpeakStreamDefs::Initialize
// ============================================================================

#include "SpeakStreamDefs.h"
#include <cassert>
#include <cstring>

RwUInt32 CSpeakStreamDefs::sm_anSpeaks[SPEAK_MAX_NUM_CATEGORIES + 1];
RwBool CSpeakStreamDefs::sm_bValidSpeakSetLoaded = FALSE;

void CSpeakStreamDefs::Initialize()
{
    memset(sm_anSpeaks, 0, sizeof(sm_anSpeaks));

    // Kategorilerin replik sayıları (1.1 .. 35.4)
    sm_anSpeaks[1] = 4;
    sm_anSpeaks[2] = 3;
    sm_anSpeaks[3] = 3;
    sm_anSpeaks[4] = 3;
    sm_anSpeaks[5] = 4;
    sm_anSpeaks[6] = 12;
    sm_anSpeaks[7] = 3;
    sm_anSpeaks[8] = 10;
    sm_anSpeaks[9] = 10;
    sm_anSpeaks[10] = 5;

    sm_bValidSpeakSetLoaded = TRUE;
}

RwBool CSpeakStreamDefs::IsValidSpeakSetLoaded()
{
    return sm_bValidSpeakSetLoaded;
}

RwUInt32 CSpeakStreamDefs::GetCategorySpeakCount(RwUInt32 category)
{
    if (category >= 1 && category <= SPEAK_MAX_NUM_CATEGORIES)
    {
        return sm_anSpeaks[category];
    }
    return 0;
}
