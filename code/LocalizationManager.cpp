// ============================================================================
// CannonCruise - Localization Manager (LocalizationManager.cpp)
// Original path: D:\Projects\CannonCruisePC\code\LocalizationManager.cpp
// Tersine muhendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazilmistir.
//
// Binary Fonksiyonlari:
//   FUN_004918c0 - CLocalizationManager constructor & initialization
//   FUN_00491f20 - CLocalizationManager::SetLanguage
//   sub_492D40   - Localiza\langmap.def okuma
//   sub_493130   - Localiza\lang.def okuma
//   sub_491120   - MENUTRIGGER_LangSelect1..12 handler
// ============================================================================

#include "LocalizationManager.h"
#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdio>
#include <cstring>

static const char *s_langTriggerMap[12] = {
    "eng", // MENUTRIGGER_LangSelect1  (English)
    "nor", // MENUTRIGGER_LangSelect2  (Norwegian)
    "swe", // MENUTRIGGER_LangSelect3  (Swedish)
    "den", // MENUTRIGGER_LangSelect4  (Danish)
    "fin", // MENUTRIGGER_LangSelect5  (Finnish)
    "eng", // MENUTRIGGER_LangSelect6  (fallback)
    "eng", // MENUTRIGGER_LangSelect7
    "eng", // MENUTRIGGER_LangSelect8
    "eng", // MENUTRIGGER_LangSelect9
    "eng", // MENUTRIGGER_LangSelect10
    "eng", // MENUTRIGGER_LangSelect11
    "eng", // MENUTRIGGER_LangSelect12
};

static std::string ToLowerStr(const std::string &str)
{
    std::string res = str;
    for (size_t i = 0; i < res.size(); ++i)
    {
        res[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(res[i])));
    }
    return res;
}

/**
 * @brief Constructs a new CLocalizationManager instance.
 * @see FUN_004918c0
 */
CLocalizationManager::CLocalizationManager()
    : m_CurrentLanguage("eng"),
      m_DefaultLanguage("eng"),
      m_pFontTexDict(0)
{
    LoadDefinitions();
}

/**
 * @brief Destructor. Destroys font texture dictionary and clears string cache.
 */
CLocalizationManager::~CLocalizationManager()
{
    m_StringTable.clear();
    m_LanguageCodes.clear();
    m_LanguageNames.clear();
    if (m_pFontTexDict)
    {
        RwTexDictionaryDestroy(m_pFontTexDict);
        m_pFontTexDict = 0;
    }
}

/**
 * @brief Loads definitions from Localiza/lang.def and Localiza/langmap.def.
 */
RwBool CLocalizationManager::LoadDefinitions(const char *pBasePath)
{
    char szLangDef[260];
    char szLangMap[260];

    auto TryOpen = [](const char *path1, const char *path2, FILE *&f1, FILE *&f2) -> bool {
        f1 = fopen(path1, "r");
        f2 = fopen(path2, "r");
        return (f1 != nullptr);
    };

    FILE *fDef = nullptr;
    FILE *fMap = nullptr;

    if (pBasePath && pBasePath[0] != '\0')
    {
        snprintf(szLangDef, sizeof(szLangDef), "%s\\Localiza\\lang.def", pBasePath);
        snprintf(szLangMap, sizeof(szLangMap), "%s\\Localiza\\langmap.def", pBasePath);
        TryOpen(szLangDef, szLangMap, fDef, fMap);
    }

    if (!fDef)
    {
        snprintf(szLangDef, sizeof(szLangDef), "Localiza\\lang.def");
        snprintf(szLangMap, sizeof(szLangMap), "Localiza\\langmap.def");
        TryOpen(szLangDef, szLangMap, fDef, fMap);
    }

    if (!fDef)
    {
        const char *orig = "C:\\Program Files (x86)\\CannonCruise";
        snprintf(szLangDef, sizeof(szLangDef), "%s\\Localiza\\lang.def", orig);
        snprintf(szLangMap, sizeof(szLangMap), "%s\\Localiza\\langmap.def", orig);
        TryOpen(szLangDef, szLangMap, fDef, fMap);
    }

    if (!fDef)
    {
        // Fallback varsayilan diller
        m_LanguageCodes = {"den", "eng", "fin", "nor", "swe"};
        m_LanguageNames = {"Danish", "English", "Finnish", "Norwegian", "Swedish"};
        m_DefaultLanguage = "eng";
        return FALSE;
    }

    m_LanguageCodes.clear();
    char line[128];
    bool first = true;
    while (fgets(line, sizeof(line), fDef))
    {
        // Trim whitespace
        char *p = line;
        while (*p && (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')) ++p;
        char *end = p + strlen(p);
        while (end > p && (*(end - 1) == ' ' || *(end - 1) == '\t' || *(end - 1) == '\r' || *(end - 1) == '\n'))
        {
            --end;
            *end = '\0';
        }

        if (*p == '\0') continue;

        if (first)
        {
            // HUGO_CC basligi
            first = false;
            continue;
        }

        bool isDefault = false;
        if (*p == '*')
        {
            isDefault = true;
            ++p;
        }

        std::string code = ToLowerStr(p);
        m_LanguageCodes.push_back(code);
        if (isDefault)
        {
            m_DefaultLanguage = code;
        }
    }
    fclose(fDef);

    if (fMap)
    {
        m_LanguageNames.clear();
        while (fgets(line, sizeof(line), fMap))
        {
            char *p = line;
            while (*p && (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')) ++p;
            char *end = p + strlen(p);
            while (end > p && (*(end - 1) == ' ' || *(end - 1) == '\t' || *(end - 1) == '\r' || *(end - 1) == '\n'))
            {
                --end;
                *end = '\0';
            }
            if (*p != '\0')
            {
                m_LanguageNames.push_back(p);
            }
        }
        fclose(fMap);
    }

    return TRUE;
}

/**
 * @brief Handles incoming event messages.
 * @see sub_491120
 */
void CLocalizationManager::HandleEvents(const RWS::CMsg& msg)
{
    if (msg.m_Id == 0) return;

    if (msg.m_pData != nullptr)
    {
        const char *pData = static_cast<const char *>(msg.m_pData);

        // Sayi mi? -> trigger index olarak yorumla (1-12 -> s_langTriggerMap)
        if (pData[0] >= '1' && pData[0] <= '9' && pData[1] == 0)
        {
            const int idx = pData[0] - '1';
            if (idx >= 0 && idx < 12)
            {
                std::printf("LocalizationManager: LangSelect%d -> '%s'\n",
                            idx + 1, s_langTriggerMap[idx]);
                SetLanguage(s_langTriggerMap[idx]);
            }
        }
        else if (pData[0] != 0)
        {
            std::printf("LocalizationManager: SetLanguage('%s') via event\n",
                        pData);
            SetLanguage(pData);
        }
    }
}

/**
 * @brief Sets the active localization language and parses string table.
 * @see FUN_00491f20
 */
RwBool CLocalizationManager::SetLanguage(const char* pLangCode)
{
    if (!pLangCode || pLangCode[0] == '\0') return FALSE;

    std::string lowerCode = ToLowerStr(pLangCode);

    char szTxtPath[260];
    snprintf(szTxtPath, sizeof(szTxtPath), "Localiza\\lang_%s.txt", lowerCode.c_str());

    FILE* pFile = fopen(szTxtPath, "rb");
    if (!pFile)
    {
        char szFallback[260];
        snprintf(szFallback, sizeof(szFallback),
                 "C:\\Program Files (x86)\\CannonCruise\\Localiza\\lang_%s.txt",
                 lowerCode.c_str());
        pFile = fopen(szFallback, "rb");
        if (pFile)
        {
            strncpy(szTxtPath, szFallback, sizeof(szTxtPath) - 1);
        }
    }

    if (!pFile)
    {
        std::printf("LocalizationManager: dil dosyasi bulunamadi: Localiza\\lang_%s.txt\n",
                    lowerCode.c_str());
        return FALSE;
    }

    // Dosya boyutunu bul
    fseek(pFile, 0, SEEK_END);
    long fileSize = ftell(pFile);
    fseek(pFile, 0, SEEK_SET);

    if (fileSize < 2)
    {
        fclose(pFile);
        return FALSE;
    }

    // Unicode BOM kontrolu (0xFEFF / 0xFFFE)
    unsigned short bom = 0;
    fread(&bom, sizeof(unsigned short), 1, pFile);
    if (bom != 0xFEFF && bom != 0xFFFE)
    {
        std::printf("LocalizationManager: dosya Unicode BOM eksik: %s\n", szTxtPath);
        fclose(pFile);
        return FALSE;
    }

    const size_t numWchars = (fileSize - 2) / sizeof(wchar_t);
    std::vector<wchar_t> buffer(numWchars + 1, 0);
    fread(&buffer[0], sizeof(wchar_t), numWchars, pFile);
    fclose(pFile);

    // Byte swap gerekirse (0xFFFE ise)
    if (bom == 0xFFFE)
    {
        for (size_t i = 0; i < numWchars; ++i)
        {
            unsigned short val = static_cast<unsigned short>(buffer[i]);
            buffer[i] = static_cast<wchar_t>((val >> 8) | (val << 8));
        }
    }

    m_StringTable.clear();

    // Satir satir ayristir: format "<KEY> <SCALE> <TEXT>"
    size_t i = 0;
    while (i < numWchars)
    {
        // Satir sonuna kadar ilerle
        size_t lineStart = i;
        while (i < numWchars && buffer[i] != L'\r' && buffer[i] != L'\n')
        {
            ++i;
        }
        size_t lineEnd = i;
        while (i < numWchars && (buffer[i] == L'\r' || buffer[i] == L'\n'))
        {
            ++i;
        }

        // Bos satir kontrolu
        size_t cur = lineStart;
        while (cur < lineEnd && (buffer[cur] == L' ' || buffer[cur] == L'\t')) ++cur;
        if (cur >= lineEnd) continue;

        // Yorum kontrolu ('#' veya '?#')
        if (buffer[cur] == L'#' || (cur + 1 < lineEnd && buffer[cur] == 0xFEFF && buffer[cur + 1] == L'#'))
        {
            continue;
        }

        // Ilk token: KEY
        size_t keyStart = cur;
        while (cur < lineEnd && buffer[cur] != L' ' && buffer[cur] != L'\t') ++cur;
        size_t keyEnd = cur;

        std::string keyStr;
        for (size_t k = keyStart; k < keyEnd; ++k)
        {
            keyStr += static_cast<char>(buffer[k]);
        }

        // Ikinci token: SCALE (orn. 100)
        while (cur < lineEnd && (buffer[cur] == L' ' || buffer[cur] == L'\t')) ++cur;
        while (cur < lineEnd && buffer[cur] != L' ' && buffer[cur] != L'\t') ++cur;

        // Kalan kisim: TEXT
        while (cur < lineEnd && (buffer[cur] == L' ' || buffer[cur] == L'\t')) ++cur;

        std::wstring valStr;
        if (cur < lineEnd)
        {
            valStr.assign(&buffer[cur], lineEnd - cur);
        }

        if (!keyStr.empty())
        {
            m_StringTable[keyStr] = valStr;
        }
    }

    m_CurrentLanguage = lowerCode;
    std::printf("LocalizationManager: '%s' dili yuklendi (%zu dizge)\n",
                m_CurrentLanguage.c_str(), m_StringTable.size());

    // Font doku sozlugunu yukle (sub_4923A0: "Localiza\Win\win_%s.txd")
    if (m_pFontTexDict)
    {
        RwTexDictionaryDestroy(m_pFontTexDict);
        m_pFontTexDict = nullptr;
    }

    char szTxdPath[260];
    snprintf(szTxdPath, sizeof(szTxdPath), "Localiza\\Win\\win_%s.txd", lowerCode.c_str());
    RwStream* pTxdStream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, szTxdPath);
    if (!pTxdStream)
    {
        char szTxdFallback[260];
        snprintf(szTxdFallback, sizeof(szTxdFallback),
                 "C:\\Program Files (x86)\\CannonCruise\\Localiza\\Win\\win_%s.txd",
                 lowerCode.c_str());
        pTxdStream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, szTxdFallback);
    }
    if (pTxdStream)
    {
        m_pFontTexDict = RwTexDictionaryStreamRead(pTxdStream);
        RwStreamClose(pTxdStream, nullptr);
        if (m_pFontTexDict)
        {
            std::printf("LocalizationManager: font doku sozlugu yuklendi: win_%s.txd\n",
                        lowerCode.c_str());
        }
    }

    return TRUE;
}

/**
 * @brief Returns the localized wide character string for a token key.
 */
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

/**
 * @brief Returns the index (1-12) for a given language code, or -1 if not found.
 */
int CLocalizationManager::GetLangSelectIndex(const char *pLangCode) const
{
    if (!pLangCode) return -1;
    std::string lower = ToLowerStr(pLangCode);
    for (int i = 0; i < 12; ++i)
    {
        if (std::strcmp(s_langTriggerMap[i], lower.c_str()) == 0)
            return i + 1;
    }
    return -1;
}