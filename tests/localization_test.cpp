// ============================================================================
// CannonCruise - LocalizationManager Test (tests/localization_test.cpp)
// ============================================================================

#include "LocalizationManager.h"
#include <cassert>
#include <cstdio>
#include <cwchar>
#include <rwcore.h>

int main(int argc, char* argv[])
{
    std::printf("=== CannonCruise LocalizationManager Test ===\n");

    // RenderWare motoru baslatilmasi (RwTexDictionaryStreamRead icin gereklidir)
    if (RwEngineInit(0, 0, 0) != TRUE || RwEngineOpen() != TRUE || RwEngineStart() != TRUE)
    {
        std::fprintf(stderr, "localization_test: engine init failed\n");
        return 2;
    }

    {
        CLocalizationManager locMgr;

        // 1. Definition testi
        std::printf("[1] Definitions...\n");
        int count = locMgr.GetLanguageCount();
        std::printf("  Found %d languages\n", count);
        assert(count >= 5 && "En az 5 dil bulunmali (den, eng, fin, nor, swe)");

        assert(locMgr.GetLangSelectIndex("eng") == 1);
        assert(locMgr.GetLangSelectIndex("nor") == 2);
        assert(locMgr.GetLangSelectIndex("swe") == 3);
        assert(locMgr.GetLangSelectIndex("den") == 4);
        assert(locMgr.GetLangSelectIndex("fin") == 5);
        std::printf("  OK: Language indices verified\n");

        // 2. SetLanguage testi
        std::printf("[2] SetLanguage('eng')...\n");
        RwBool ok = locMgr.SetLanguage("eng");
        assert(ok && "SetLanguage('eng') basarili olmali");
        assert(locMgr.GetFontTexDict() != nullptr && "win_eng.txd font texture dictionary yuklenmeli");
        std::printf("  OK: Font texture dictionary loaded\n");

        // 3. String lookup testi
        std::printf("[3] GetString lookup...\n");
        const wchar_t* pQuit = locMgr.GetString("0001");
        std::printf("  '0001' -> %ls\n", pQuit);
        assert(pQuit != nullptr && std::wcslen(pQuit) > 0);
        assert(std::wcsstr(pQuit, L"QUIT") != nullptr);

        const wchar_t* pNew = locMgr.GetString("0200");
        std::printf("  '0200' -> %ls\n", pNew);
        assert(pNew != nullptr && std::wcslen(pNew) > 0);
        assert(std::wcsstr(pNew, L"NEW GAME") != nullptr);

        // 4. Event testi
        std::printf("[4] Event trigger test...\n");
        RWS::CMsg msg;
        msg.m_Id = 0x1234;
        msg.m_pData = const_cast<char*>("2"); // Nor
        locMgr.HandleEvents(msg);
        assert(std::string(locMgr.GetCurrentLanguage()) == "nor");

        const wchar_t* pQuitNor = locMgr.GetString("0001");
        std::printf("  Norwegian '0001' -> %ls\n", pQuitNor);
        assert(pQuitNor != nullptr && std::wcslen(pQuitNor) > 0);

        // Geri eng yap
        msg.m_pData = const_cast<char*>("eng");
        locMgr.HandleEvents(msg);
        assert(std::string(locMgr.GetCurrentLanguage()) == "eng");
    }

    RwEngineStop();
    RwEngineClose();
    RwEngineTerm();

    std::printf("ALL LOCALIZATION TESTS PASSED!\n");
    return 0;
}