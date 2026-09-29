#ifndef SAVEGAMEMANAGER_H
#define SAVEGAMEMANAGER_H

// ============================================================================
// CannonCruise - Save Game Manager (SaveGameManager.h)
// Original path: D:\Projects\CannonCruisePC\code\SaveGameManager.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include "Singleton.h"
#include <rwcore.h>

struct SSaveGameData
{
    RwUInt32 m_uLevelUnlocked;
    RwUInt32 m_uHighScore;
    RwUInt32 m_uCoins;
    RwUInt32 m_uChecksum;
};

class CSaveGameManager : public CSingleton<CSaveGameManager>
{
public:
    CSaveGameManager();
    virtual ~CSaveGameManager();

    RwBool Save();
    RwBool Load();

    SSaveGameData& GetData() { return m_SaveData; }

private:
    SSaveGameData m_SaveData;
    char m_szSavePath[64];
};

#endif // SAVEGAMEMANAGER_H
