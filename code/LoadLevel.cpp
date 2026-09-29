// ============================================================================
// CannonCruise - Level Loader (LoadLevel.cpp)
// Original path: D:\Projects\CannonCruisePC\code\LoadLevel.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_004900c0 — CLoadLevel constructor
//   FUN_00490670 — CLoadLevel::RequestLevel
// ============================================================================

#include "LoadLevel.h"
#include <cassert>
#include <cstring>

CLoadLevel::CLoadLevel()
{
    memset(m_szLevelName, 0, sizeof(m_szLevelName));
}

CLoadLevel::~CLoadLevel()
{
}

void CLoadLevel::HandleEvents(const RWS::CMsg& msg)
{
}

// ============================================================================
// CLoadLevel::RequestLevel (FUN_00490670)
// ASM: 00490670 — Yeni seviyeyi talep et ve doğrula
// ============================================================================
void CLoadLevel::RequestLevel(const char* pLevelName)
{
    if (!pLevelName) return;

    assert(strlen(pLevelName) < sizeof(m_szLevelName) && "Level name exceeds maximum characters!");

    strncpy(m_szLevelName, pLevelName, sizeof(m_szLevelName) - 1);
    m_szLevelName[sizeof(m_szLevelName) - 1] = '\0';
}
