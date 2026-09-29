#ifndef SHIPPLAYERINPUTPROXY_H
#define SHIPPLAYERINPUTPROXY_H

// ============================================================================
// CannonCruise - Player Ship Input Proxy (ShipPlayerInputProxy.h)
// Original path: D:\Projects\CannonCruisePC\code\ShipPlayerInputProxy.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include "Ship.h"
#include <rwcore.h>

class CShipPlayerInputProxy : public RWS::CEventHandler
{
public:
    CShipPlayerInputProxy(CShip* pShip = 0);
    virtual ~CShipPlayerInputProxy();

    virtual void HandleEvents(const RWS::CMsg& msg);

    void AttachShip(CShip* pShip);
    void Update();

private:
    CShip* m_pShip;
    RwInt32 m_iPlayerIndex;
};

#endif // SHIPPLAYERINPUTPROXY_H
