#ifndef PHYSICSENTITY_H
#define PHYSICSENTITY_H

// ============================================================================
// CannonCruise - Physics Entity Base (PhysicsEntity.h)
// Original path: D:\Projects\CannonCruisePC\code\PhysicsEntity.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include "framework/core/eventhandler/ceventhandler.h"
#include <rwcore.h>

class CPhysicsEntity : public RWS::CEventHandler
{
public:
    CPhysicsEntity();
    virtual ~CPhysicsEntity();

    virtual void HandleEvents(const RWS::CMsg& msg);

    void* GetRigidBody() const { return m_pRigidBody; }
    void SetRigidBody(void* pBody);

    void AddToWorld();
    void RemoveFromWorld();

    void SetPosition(const RwV3d* pPos);
    void GetPosition(RwV3d* pPos) const;
    void SetLinearVelocity(const RwV3d* pVel);
    void GetLinearVelocity(RwV3d* pVel) const;

protected:
    void* m_pRigidBody;
    RwBool m_bInWorld;
};

#endif // PHYSICSENTITY_H
