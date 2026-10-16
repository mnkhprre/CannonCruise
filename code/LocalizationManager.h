#ifndef LOCALIZATIONMANAGER_H
#define LOCALIZATIONMANAGER_H

// ============================================================================
// CannonCruise - Localization Manager (LocalizationManager.h)
// Original path: D:\Projects\CannonCruisePC\code\LocalizationManager.h
// Tersine muhendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazilmistir.
// ============================================================================

#include "Singleton.h"
#include "framework/core/eventhandler/ceventhandler.h"
#include <map>
#include <string>
#include <vector>
#include <rwcore.h>

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
   * @brief Loads definitions from Localiza/lang.def and Localiza/langmap.def.
   * @param pBasePath Optional directory path containing Localiza folder.
   * @return TRUE if definitions were found and loaded, FALSE otherwise.
   */
  RwBool LoadDefinitions(const char *pBasePath = nullptr);

  /**
   * @brief Loads and activates a localized language file pack.
   * @param pLangCode Language suffix identifier (e.g. "eng", "den", "fin", "nor", "swe").
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

  /**
   * @brief Returns the 1-based index (1-12) for a given language code.
   */
  int GetLangSelectIndex(const char *pLangCode) const;

  /**
   * @brief Returns current language code string (e.g. "eng").
   */
  const char *GetCurrentLanguage() const { return m_CurrentLanguage.c_str(); }

  /**
   * @brief Returns the number of loaded language definitions.
   */
  int GetLanguageCount() const { return static_cast<int>(m_LanguageCodes.size()); }

  /**
   * @brief Returns language code by index.
   */
  const char *GetLanguageCode(int idx) const {
    if (idx >= 0 && idx < static_cast<int>(m_LanguageCodes.size()))
      return m_LanguageCodes[idx].c_str();
    return "";
  }

  /**
   * @brief Returns friendly language name by index (from langmap.def).
   */
  RwTexDictionary *GetFontTexDict() const { return m_pFontTexDict; }

  const char *GetLanguageName(int idx) const {
    if (idx >= 0 && idx < static_cast<int>(m_LanguageNames.size()))
      return m_LanguageNames[idx].c_str();
    return "";
  }

private:
  std::string                         m_CurrentLanguage;   ///< Currently active language code
  std::vector<std::string>            m_LanguageCodes;     ///< Available codes ("den", "eng", etc.)
  std::vector<std::string>            m_LanguageNames;     ///< Friendly names ("Danish", "English", etc.)
  std::string                         m_DefaultLanguage;   ///< Default language code from lang.def
  std::map<std::string, std::wstring> m_StringTable;       ///< Key to wide text string translation map
  RwTexDictionary*                    m_pFontTexDict;      ///< Font texture dictionary handle
};

#endif // LOCALIZATIONMANAGER_H