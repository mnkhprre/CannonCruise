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

/**
 * @class CLocalizationManager
 * @brief Multi-language localization string table manager.
 *
 * Loads Unicode (UTF-16/UCS-2 LE) language packs from the filesystem (`Localiza\lang_<code_>.txt`),
 * parses key-value pairs, and provides localized wide strings for game text and subtitles.
 */
class CLocalizationManager : public CSingleton<CLocalizationManager>,
                             public RWS::CEventHandler {
public:
  /**
   * @brief Constructs a new CLocalizationManager singleton instance.
   * @see FUN_004918c0
   */
  CLocalizationManager();

  /**
   * @brief Destructor. Destroys font texture dictionary and clears string cache.
   */
  virtual ~CLocalizationManager();

  /**
   * @brief Handles incoming event messages.
   * @param msg Event message container.
   */
  virtual void HandleEvents(const RWS::CMsg &msg);

  /**
   * @brief Loads and activates a localized language file pack.
   * @param pLangCode Language suffix identifier (e.g. "en", "de", "fr").
   * @return TRUE if successfully loaded and validated against Unicode BOM, FALSE otherwise.
   * @see FUN_00491f20
   */
  RwBool SetLanguage(const char *pLangCode);

  /**
   * @brief Looks up a localized wide string corresponding to a given key.
   * @param pKey Localization token key.
   * @return Pointer to wide character string or empty wide string if key is absent.
   */
  const wchar_t *GetString(const char *pKey) const;

private:
  std::string                         m_CurrentLanguage; ///< Currently active language code
  std::map<std::string, std::wstring> m_StringTable;     ///< Key to wide text string translation map
  RwTexDictionary*                    m_pFontTexDict;    ///< Font texture dictionary handle
};

#endif // LOCALIZATIONMANAGER_H
