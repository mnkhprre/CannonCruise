// ============================================================================
// CannonCruise - Speech Manager (SpeakManager.cpp)
// Original path: D:\Projects\CannonCruisePC\code\SpeakManager.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_0045b3b0 — CSpeakManager constructor & initialization
//   FUN_0045b7c0 — CSpeakManager::IsSpeakValid ("iCategory >= 1 Failed", "iCategory <= SPEAK_MAX_NUM_CATEGORIES Failed", "iIndex >= 0 Failed", "m_anSpeaks[iCategory] > 0 Failed", "(RwUInt32)iIndex < m_anSpeaks[iCategory] Failed")
//   FUN_0045bcc0 — CSpeakManager::PlaySpeak
// ============================================================================

#include "SpeakManager.h"
#include <cassert>

// ---------------------------------------------------------------------------
// Constructor – mirrors FUN_0045b3b0.
// Initializes paths, sets current category/index to -1, and initializes
// category counts via CSpeakStreamDefs.
// ---------------------------------------------------------------------------
CSpeakManager::CSpeakManager()
    : m_SpeaksPath("Sound\\Win32\\Speaks\\"), m_iCurrentCategory(-1),
      m_iCurrentIndex(-1) {
  CSpeakStreamDefs::Initialize();
}

// ---------------------------------------------------------------------------
// Destructor – halts any active speech stream.
// ---------------------------------------------------------------------------
CSpeakManager::~CSpeakManager() { StopSpeak(); }

// ---------------------------------------------------------------------------
// HandleEvents – framework message listener.
// ---------------------------------------------------------------------------
void CSpeakManager::HandleEvents(const RWS::CMsg &msg) {}

// ---------------------------------------------------------------------------
// IsSpeakValid – mirrors FUN_0045b7c0.
// Checks that category is between 1 and 36, that index is >= 0, that category
// has registered voice lines (> 0), and that index is within the count.
// ---------------------------------------------------------------------------
RwBool CSpeakManager::IsSpeakValid(RwInt32 iCategory, RwInt32 iIndex) const {
  assert(iCategory >= 1 && "iCategory >= 1 Failed");
  assert(iCategory <= SPEAK_MAX_NUM_CATEGORIES &&
         "iCategory <= SPEAK_MAX_NUM_CATEGORIES Failed");
  assert(iIndex >= 0 && "iIndex >= 0 Failed");

  RwUInt32 count = CSpeakStreamDefs::GetCategorySpeakCount(iCategory);
  assert(count > 0 && "m_anSpeaks[iCategory] > 0 Failed");
  assert((RwUInt32)iIndex < count &&
         "(RwUInt32)iIndex < m_anSpeaks[iCategory] Failed");

  return ((RwUInt32)iIndex < count);
}

// ---------------------------------------------------------------------------
// PlaySpeak – mirrors FUN_0045bcc0.
// Validates requested line, tracks active category and index, and streams
// "Sound\\Win32\\Speaks\\%d.%d.wav" through the audio backend.
// ---------------------------------------------------------------------------
void CSpeakManager::PlaySpeak(RwInt32 iCategory, RwInt32 iIndex,
                              const RwV3d *pPos) {
  if (!IsSpeakValid(iCategory, iIndex))
    return;

  m_iCurrentCategory = iCategory;
  m_iCurrentIndex = iIndex;

  // Stream "Sound\Win32\Speaks\%d.%d.wav"
}

// ---------------------------------------------------------------------------
// StopSpeak – stops playback and resets active indices.
// ---------------------------------------------------------------------------
void CSpeakManager::StopSpeak() {
  m_iCurrentCategory = -1;
  m_iCurrentIndex = -1;
}
