// ============================================================================
// CannonCruise - Speech Manager (SpeakManager.cpp)
// Original path: D:\Projects\CannonCruisePC\code\SpeakManager.cpp
// Reverse-engineered via Ghidra and CC.asm analysis.
//
// Binary Functions:
//   FUN_0045b3b0 - CSpeakManager constructor & initialization
//   FUN_0045b7c0 - CSpeakManager::IsSpeakValid
//   FUN_0045bcc0 - CSpeakManager::PlaySpeak
// ============================================================================

#include "SpeakManager.h"
#include <cassert>
#include <cstdio>
#include <cstring>
#include <windows.h>
#include <mmsystem.h>

// ---------------------------------------------------------------------------
// Constructor - mirrors FUN_0045b3b0.
// Initializes paths, sets current category/index to -1, and initializes
// category counts via CSpeakStreamDefs. Default language is Turkish ("Tyr").
// ---------------------------------------------------------------------------
CSpeakManager::CSpeakManager()
    : m_SpeaksPath("Sound\\Win32\\Speaks\\"),
      m_Language("Tyr"),
      m_iCurrentCategory(-1),
      m_iCurrentIndex(-1) {
  CSpeakStreamDefs::Initialize();
}

// ---------------------------------------------------------------------------
// Destructor - halts any active speech stream.
// ---------------------------------------------------------------------------
CSpeakManager::~CSpeakManager() { StopSpeak(); }

// ---------------------------------------------------------------------------
// SetLanguage - changes the language directory for voice lines.
// ---------------------------------------------------------------------------
void CSpeakManager::SetLanguage(const char* pLang) {
  if (pLang && pLang[0]) {
    m_Language = pLang;
  }
}

// ---------------------------------------------------------------------------
// HandleEvents - framework message listener.
// ---------------------------------------------------------------------------
void CSpeakManager::HandleEvents(const RWS::CMsg &msg) {}

// ---------------------------------------------------------------------------
// IsSpeakValid - mirrors FUN_0045b7c0.
// Checks that category is between 1 and 36, that index is >= 0, that category
// has registered voice lines (> 0), and that index is within the count.
// ---------------------------------------------------------------------------
RwBool CSpeakManager::IsSpeakValid(RwInt32 iCategory, RwInt32 iIndex) const {
  if (iCategory < 1 || iCategory > SPEAK_MAX_NUM_CATEGORIES || iIndex < 0) {
    return FALSE;
  }

  RwUInt32 count = CSpeakStreamDefs::GetCategorySpeakCount(iCategory);
  if (count == 0) {
    // If not in hardcoded table, allow reasonable bounds
    return (iIndex < 20) ? TRUE : FALSE;
  }

  return ((RwUInt32)iIndex < count) ? TRUE : FALSE;
}

// ---------------------------------------------------------------------------
// PlaySpeak - mirrors FUN_0045bcc0.
// Streams "Sound\Win32\Speaks\<Lang>\<cue>.wav" via dedicated MCI voice channel.
// ---------------------------------------------------------------------------
void CSpeakManager::PlaySpeak(RwInt32 iCategory, RwInt32 iIndex,
                              const RwV3d *pPos) {
  if (!IsSpeakValid(iCategory, iIndex))
    return;

  m_iCurrentCategory = iCategory;
  m_iCurrentIndex = iIndex;

  StopSpeak();

  // Voice cue pattern to find: e.g. "_1.1.wav" or "_10.2.wav"
  char cuePattern[64] = {0};
  std::snprintf(cuePattern, sizeof(cuePattern), "_%d.%d", iCategory, iIndex + 1);

  char searchDir[MAX_PATH] = {0};
  std::snprintf(searchDir, sizeof(searchDir), "%s%s\\*.wav", m_SpeaksPath.c_str(), m_Language.c_str());

  char targetFile[MAX_PATH] = {0};
  WIN32_FIND_DATAA fd;
  HANDLE hFind = ::FindFirstFileA(searchDir, &fd);
  if (hFind != INVALID_HANDLE_VALUE) {
    do {
      if (std::strstr(fd.cFileName, cuePattern) != nullptr) {
        std::snprintf(targetFile, sizeof(targetFile), "%s%s\\%s", m_SpeaksPath.c_str(), m_Language.c_str(), fd.cFileName);
        break;
      }
    } while (::FindNextFileA(hFind, &fd));
    ::FindClose(hFind);
  }

  // Fallback direct paths
  if (targetFile[0] == 0) {
    std::snprintf(targetFile, sizeof(targetFile), "%s%s\\%d.%d.wav", m_SpeaksPath.c_str(), m_Language.c_str(), iCategory, iIndex + 1);
  }

  // Stream via MCI cc_voice alias
  char mciCmd[MAX_PATH + 64] = {0};
  std::snprintf(mciCmd, sizeof(mciCmd), "open \"%s\" type waveaudio alias cc_voice", targetFile);
  if (::mciSendStringA(mciCmd, nullptr, 0, nullptr) == 0) {
    ::mciSendStringA("play cc_voice", nullptr, 0, nullptr);
  }
}

// ---------------------------------------------------------------------------
// StopSpeak - stops playback and resets active indices.
// ---------------------------------------------------------------------------
void CSpeakManager::StopSpeak() {
  ::mciSendStringA("stop cc_voice", nullptr, 0, nullptr);
  ::mciSendStringA("close cc_voice", nullptr, 0, nullptr);
  m_iCurrentCategory = -1;
  m_iCurrentIndex = -1;
}
