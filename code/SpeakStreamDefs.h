#ifndef SPEAKSTREAMDEFS_H
#define SPEAKSTREAMDEFS_H

// ============================================================================
// CannonCruise - Speech Stream Definitions (SpeakStreamDefs.h)
// Original path: D:\Projects\CannonCruisePC\code\SpeakStreamDefs.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include <rwcore.h>

#define SPEAK_MAX_NUM_CATEGORIES 36

struct SSpeakCategoryInfo
{
    RwUInt32 m_uCategory;
    RwUInt32 m_uCount;
};

class CSpeakStreamDefs
{
public:
    static void Initialize();
    static RwBool IsValidSpeakSetLoaded();
    static RwUInt32 GetCategorySpeakCount(RwUInt32 category);

private:
    static RwUInt32 sm_anSpeaks[SPEAK_MAX_NUM_CATEGORIES + 1];
    static RwBool sm_bValidSpeakSetLoaded;
};

#endif // SPEAKSTREAMDEFS_H
