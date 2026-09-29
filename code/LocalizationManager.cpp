// ============================================================================
// CannonCruise - Localization Manager (LocalizationManager.cpp)
// Original path: D:\Projects\CannonCruisePC\code\LocalizationManager.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_004918c0 — CLocalizationManager constructor & initialization
//   FUN_00491f20 — CLocalizationManager::SetLanguage
// ============================================================================

#include "LocalizationManager.h"
#include <cassert>
#include <cstdio>

CLocalizationManager::CLocalizationManager()
    : m_CurrentLanguage("en"),
      m_pFontTexDict(0)
{
}

CLocalizationManager::~CLocalizationManager()
{
    if (m_pFontTexDict)
    {
        RwTexDictionaryDestroy(m_pFontTexDict);
        m_pFontTexDict = 0;
    }
}

void CLocalizationManager::HandleEvents(const RWS::CMsg& msg)
{
}

// ============================================================================
// CLocalizationManager::SetLanguage (FUN_00491f20)
// ASM: 00491f20 — Dil dosyasını yükle ve Unicode başlığını kontrol et
// ============================================================================
RwBool CLocalizationManager::SetLanguage(const char* pLangCode)
{
    if (!pLangCode) return FALSE;

    m_CurrentLanguage = pLangCode;

    char szTxtPath[128];
    snprintf(szTxtPath, sizeof(szTxtPath), "Localiza\\lang_%s.txt", pLangCode);

    FILE* pFile = fopen(szTxtPath, "rb");
    if (!pFile)
    {
        assert(false && "LocalizationManager: file not found");
        return FALSE;
    }

    // Unicode BOM kontrolü (0xFEFF / 0xFFFE)
    unsigned short bom = 0;
    fread(&bom, sizeof(unsigned short), 1, pFile);
    if (bom != 0xFEFF && bom != 0xFFFE)
    {
        assert(false && "LocalizationManager: file misses the Unicode header!");
        fclose(pFile);
        return FALSE;
    }

    fclose(pFile);
    return TRUE;
}

const wchar_t* CLocalizationManager::GetString(const char* pKey) const
{
    if (!pKey) return L"";

    std::map<std::string, std::wstring>::const_iterator it = m_StringTable.find(pKey);
    if (it != m_StringTable.end())
    {
        return it->second.c_str();
    }
    return L"";
}
