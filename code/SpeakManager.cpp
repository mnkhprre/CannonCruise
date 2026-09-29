// ============================================================================
// CannonCruise - Speech Manager (SpeakManager.cpp)
// Original path: D:\Projects\CannonCruisePC\code\SpeakManager.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_0045b3b0 — CSpeakManager constructor & initialization
//   FUN_0045b7c0 — CSpeakManager::IsSpeakValid
//   FUN_0045bcc0 — CSpeakManager::PlaySpeak
// ============================================================================

#include "SpeakManager.h"
#include <cassert>

CSpeakManager::CSpeakManager()
    : m_SpeaksPath("Sound\\Win32\\Speaks\\"), m_iCurrentCategory(-1),
      m_iCurrentIndex(-1) {
  CSpeakStreamDefs::Initialize();
}

CSpeakManager::~CSpeakManager() { StopSpeak(); }

void CSpeakManager::HandleEvents(const RWS::CMsg &msg) {}

// ============================================================================
// CSpeakManager::IsSpeakValid (FUN_0045b7c0)
// ASM: 0045b7c0 — Replik kategorisi ve indeksi geçerli mi kontrol et
// ============================================================================
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

void CSpeakManager::PlaySpeak(RwInt32 iCategory, RwInt32 iIndex,
                              const RwV3d *pPos) {
  if (!IsSpeakValid(iCategory, iIndex))
    return;

  m_iCurrentCategory = iCategory;
  m_iCurrentIndex = iIndex;

  // "Sound\Win32\Speaks\%d.%d.wav" dosyasını oynat
}

void CSpeakManager::StopSpeak() {
  m_iCurrentCategory = -1;
  m_iCurrentIndex = -1;
}
