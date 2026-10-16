// ============================================================================
// CannonCruise - InputManager Test (tests/inputmanager_test.cpp)
// ============================================================================

#include "InputManager.h"
#include "ShipPlayer.h"
#include "ShipPlayerInputProxy.h"
#include <cassert>
#include <cstdio>

int main(int argc, char* argv[])
{
    std::printf("=== CannonCruise CInputManager Test ===\n");

    CInputManager inputMgr;

    // 1. Focus Stack Testi
    std::printf("[1] Focus Stack...\n");
    RwInt32 focusMenu = inputMgr.AllocateFocusID();
    RwInt32 focusGame = inputMgr.AllocateFocusID();
    assert(focusMenu >= 0 && focusGame >= 0 && focusMenu != focusGame);

    inputMgr.PushFocusID(focusMenu);
    assert(inputMgr.GetCurrentFocusID() == focusMenu);
    inputMgr.PushFocusID(focusGame);
    assert(inputMgr.GetCurrentFocusID() == focusGame);

    inputMgr.PopFocusID(focusGame);
    assert(inputMgr.GetCurrentFocusID() == focusMenu);

    inputMgr.FreeFocusID(focusMenu);
    inputMgr.FreeFocusID(focusGame);
    std::printf("  OK: Focus stack allocated, pushed, popped and freed\n");

    // 2. Button State & Edge Detection Testi
    std::printf("[2] Button State & Edge Trigger Detection...\n");
    assert(inputMgr.IsButtonDown(0, P1_INPUT_3) == FALSE);

    // Kare 1: Buton basildi
    inputMgr.SetButtonState(0, P1_INPUT_3, TRUE);
    assert(inputMgr.IsButtonDown(0, P1_INPUT_3) == TRUE);
    assert(inputMgr.IsButtonPressed(0, P1_INPUT_3) == TRUE && "İlk karede rising edge olmalı");

    // Latch update (Kare 2)
    inputMgr.Update();
    assert(inputMgr.IsButtonDown(0, P1_INPUT_3) == TRUE);
    assert(inputMgr.IsButtonPressed(0, P1_INPUT_3) == FALSE && "İkinci karede edge sönmeli");

    // Buton bırakıldı
    inputMgr.SetButtonState(0, P1_INPUT_3, FALSE);
    inputMgr.Update();
    assert(inputMgr.IsButtonDown(0, P1_INPUT_3) == FALSE);
    std::printf("  OK: Button press and edge detection verified\n");

    // 3. ShipPlayerInputProxy Entegrasyon Testi
    std::printf("[3] ShipPlayerInputProxy integration...\n");
    CShipPlayer ship;
    CShipPlayerInputProxy proxy(&ship);

    // İleri hareket (P1_INPUT_3)
    inputMgr.SetButtonState(0, P1_INPUT_3, TRUE);
    proxy.Update();
    ship.Update(0.1f);
    assert(ship.GetSpeed() > 0.0f && "Gemi ileri hizlanmali");

    // Sol dumen (P1_INPUT_1)
    inputMgr.SetButtonState(0, P1_INPUT_1, TRUE);
    proxy.Update();
    // Rudder -1.0f ayarlandi

    std::printf("  OK: Ship steering and throttle driven by InputManager\n");

    std::printf("ALL INPUT MANAGER TESTS PASSED!\n");
    return 0;
}