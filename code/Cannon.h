#ifndef CANNON_H
#define CANNON_H

// ============================================================================
// CannonCruise - Cannon Weapon Entity (Cannon.h)
// Original path: D:\Projects\CannonCruisePC\code\Cannon.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include "Cannonball.h"
#include "framework/core/eventhandler/ceventhandler.h"
#include <rwcore.h>
#include <rpworld.h>

class CCannon : public RWS::CEventHandler
{
public:
    CCannon();
    virtual ~CCannon();

    virtual void HandleEvents(const RWS::CMsg& msg);

    void Initialize(RpClump* pClump, RwUInt32 ownerID);
    void Fire(ECannonballType type, const RwV3d* pDirection, RwReal speed = 50.0f);
    void Aim(RwReal elevation, RwReal azimuth);

private:
    RpClump* m_pClump;
    RwFrame* m_pGunBarrelFrame;
    RwFrame* m_pGunBaseFrame;
    RwUInt32 m_uOwnerID;
    RwReal m_fReloadTimer;
};

#endif // CANNON_H
