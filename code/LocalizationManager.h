#ifndef LOCALIZATIONMANAGER_H
#define LOCALIZATIONMANAGER_H

// ============================================================================
// CannonCruise - Localization Manager (LocalizationManager.h)
// Original path: D:\Projects\CannonCruisePC\code\LocalizationManager.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include "Singleton.h"
#include "framework/core/eventhandler/ceventhandler.h"
#include <map>
#include <rwcore.h>
#include <string>

class CLocalizationManager : public CSingleton<CLocalizationManager>,
                             public RWS::CEventHandler {
public:
  CLocalizationManager();
  virtual ~CLocalizationManager();

  virtual void HandleEvents(const RWS::CMsg &msg);

  RwBool SetLanguage(const char *pLangCode);
  const wchar_t *GetString(const char *pKey) const;

private:
  std::string m_CurrentLanguage;
  std::map<std::string, std::wstring> m_StringTable;
  RwTexDictionary *m_pFontTexDict;
};

#endif // LOCALIZATIONMANAGER_H
