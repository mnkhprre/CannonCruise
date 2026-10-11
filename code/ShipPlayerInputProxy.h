#ifndef SHIPPLAYERINPUTPROXY_H
#define SHIPPLAYERINPUTPROXY_H

// ============================================================================
// CannonCruise - Player Ship Input Proxy (ShipPlayerInputProxy.h)
// Original path: D:\Projects\CannonCruisePC\code\ShipPlayerInputProxy.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include "Ship.h"
#include <rwcore.h>

/**
 * @class CShipPlayerInputProxy
 * @brief Input controller translating hardware button inputs into naval maneuvers.
 *
 * Connects the human player (via CInputManager) to their controlled CShip entity.
 * Translates directional buttons into analog rudder turning commands,
 * and button triggers into port/starboard cannon battery fire commands.
 */
class CShipPlayerInputProxy : public RWS::CEventHandler
{
public:
    /**
     * @brief Constructs proxy bound to optional player ship.
     * @param pShip  Target ship instance, or NULL.
     */
    CShipPlayerInputProxy(CShip* pShip = 0);

    /**
     * @brief Destructor.
     */
    virtual ~CShipPlayerInputProxy();

    /**
     * @brief Framework message dispatcher.
     * @param msg  Message descriptor (e.g. iMsgTeleportShip).
     */
    virtual void HandleEvents(const RWS::CMsg& msg);

    /**
     * @brief Binds a controlled CShip instance to this proxy.
     * @param pShip  Pointer to ship instance.
     */
    void AttachShip(CShip* pShip);

    /**
     * @brief Per-frame input sampling and vessel maneuver dispatch.
     *
     * Reads digital buttons:
     *   - Left / Right (P1_INPUT_1, P1_INPUT_2) -> CShip::SetRudder()
     *   - Port Broadside (P1_INPUT_4) -> CShip::FirePortCannons()
     *   - Starboard Broadside (P1_INPUT_5) -> CShip::FireStarboardCannons()
     */
    void Update();

private:
    CShip* m_pShip;             ///< Currently attached player ship instance.
    RwInt32 m_iPlayerIndex;     ///< Controller port index (0 for Player 1).
};

#endif // SHIPPLAYERINPUTPROXY_H
