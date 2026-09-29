// ============================================================================
// CannonCruise - Save Game Manager (SaveGameManager.cpp)
// Original path: D:\Projects\CannonCruisePC\code\SaveGameManager.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_0049dee0 — CSaveGameManager constructor & initialization
//   FUN_0049e5e0 — CSaveGameManager::Save
//   FUN_0049ed60 — CSaveGameManager::Load
// ============================================================================

#include "SaveGameManager.h"
#include <cassert>
#include <cstdio>
#include <cstring>

CSaveGameManager::CSaveGameManager()
    : m_szSavePath(".\\CannonCruise.sav")
{
    memset(&m_SaveData, 0, sizeof(m_SaveData));
    Load();
}

CSaveGameManager::~CSaveGameManager()
{
}

RwBool CSaveGameManager::Save()
{
    FILE* pFile = fopen(m_szSavePath, "wb");
    if (!pFile) return FALSE;

    m_SaveData.m_uChecksum = m_SaveData.m_uLevelUnlocked ^ m_SaveData.m_uHighScore ^ m_SaveData.m_uCoins ^ 0x53415645;
    fwrite(&m_SaveData, sizeof(SSaveGameData), 1, pFile);
    fclose(pFile);

    return TRUE;
}

RwBool CSaveGameManager::Load()
{
    FILE* pFile = fopen(m_szSavePath, "rb");
    if (!pFile)
    {
        // Varsayılan değerler
        m_SaveData.m_uLevelUnlocked = 1;
        m_SaveData.m_uHighScore = 0;
        m_SaveData.m_uCoins = 0;
        return FALSE;
    }

    fread(&m_SaveData, sizeof(SSaveGameData), 1, pFile);
    fclose(pFile);

    RwUInt32 expectedChecksum = m_SaveData.m_uLevelUnlocked ^ m_SaveData.m_uHighScore ^ m_SaveData.m_uCoins ^ 0x53415645;
    if (m_SaveData.m_uChecksum != expectedChecksum)
    {
        assert(false && "INVALID!!!!");
        m_SaveData.m_uLevelUnlocked = 1;
        return FALSE;
    }

    return TRUE;
}
