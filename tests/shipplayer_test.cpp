// ============================================================================
// CannonCruise - ShipPlayer Test (tests/shipplayer_test.cpp)
// ============================================================================

#include "ShipPlayer.h"
#include "framework/core/attributehandler/cclassfactory.h"
#include <cassert>
#include <cstdio>

int main(int argc, char* argv[])
{
    std::printf("=== CannonCruise CShipPlayer Test ===\n");

    // 1. CClassFactory kayit testi
    std::printf("[1] Reflection class factory registration...\n");
    RwBool isReg = RWS::CClassFactory::ClassIsRegistered("CShipPlayer");
    assert(isReg == TRUE && "CShipPlayer must be registered with CClassFactory");
    std::printf("  OK: 'CShipPlayer' is registered in CClassFactory\n");

    // 2. Factory create testi
    std::printf("[2] Factory Create()...\n");
    void* pInstance = RWS::CClassFactory::Create("CShipPlayer");
    assert(pInstance != nullptr && "CClassFactory::Create('CShipPlayer') must succeed");
    CShipPlayer* pPlayer = static_cast<CShipPlayer*>(pInstance);
    assert(CShipPlayer::GetPlayerShip() == pPlayer && "GetPlayerShip() must point to active instance");
    std::printf("  OK: Instance created and registered as active player ship\n");

    // 3. Skor ve can ozellikleri
    std::printf("[3] Score and health properties...\n");
    assert(pPlayer->GetTotalScore() == 0);
    pPlayer->IncreaseStaticScore(150);
    pPlayer->IncreaseDynamicScore(50);
    assert(pPlayer->GetStaticScore() == 150);
    assert(pPlayer->GetDynamicScore() == 50);
    assert(pPlayer->GetTotalScore() == 200);
    std::printf("  OK: Scores updated (Total = %d)\n", pPlayer->GetTotalScore());

    // 4. Hava durumu ve olay yonetimi
    std::printf("[4] Event message handling...\n");
    assert(pPlayer->IsStormActive() == FALSE);

    RWS::CMsg msgStorm;
    msgStorm.m_Id = 0x5432;
    msgStorm.m_pData = const_cast<char*>("iMsgStormStart");
    pPlayer->HandleEvents(msgStorm);
    assert(pPlayer->IsStormActive() == TRUE && "Storm should be active");

    msgStorm.m_pData = const_cast<char*>("iMsgStormStop");
    pPlayer->HandleEvents(msgStorm);
    assert(pPlayer->IsStormActive() == FALSE && "Storm should be stopped");
    std::printf("  OK: Storm events processed correctly\n");

    // 5. Respawn testi
    std::printf("[5] Respawn test...\n");
    RwV3d spawnPos = {10.0f, 0.0f, 25.0f};
    pPlayer->Respawn(spawnPos);
    assert(pPlayer->GetSpeed() == 0.0f);
    std::printf("  OK: Respawn completed successfully\n");

    // 6. Cleanup
    delete pPlayer;
    assert(CShipPlayer::GetPlayerShip() == nullptr && "Active player ship must be null after delete");
    std::printf("  OK: Destructor cleaned up singleton pointer\n");

    std::printf("ALL SHIPPLAYER TESTS PASSED!\n");
    return 0;
}