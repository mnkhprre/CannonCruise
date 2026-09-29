#ifndef PHYSICSWORLD_H
#define PHYSICSWORLD_H

// ============================================================================
// CannonCruise - Havok Physics World (PhysicsWorld.h)
// Original path: D:\Projects\CannonCruisePC\code\PhysicsWorld.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include "Singleton.h"
#include "framework/core/eventhandler/ceventhandler.h"
#include <rwcore.h>
#include <vector>

class CPhysicsWorld : public CSingleton<CPhysicsWorld>
{
public:
    CPhysicsWorld();
    virtual ~CPhysicsWorld();

    void Update(RwReal deltaTime);

    void AddRigidBody(void* pBody);
    void RemoveRigidBody(void* pBody);
    RwBool RigidBodyExistsInWorld(void* pBody) const;

    void AddPhantom(void* pPhantom);
    void RemovePhantom(void* pPhantom);

private:
    std::vector<void*> m_avpRigidBodyVector;
    std::vector<void*> m_avpPhantomVector;
    void* m_pHavokWorld;
};

class CPhysicsWorldBehaviour : public RWS::CEventHandler
{
public:
    CPhysicsWorldBehaviour();
    virtual ~CPhysicsWorldBehaviour();

    virtual void HandleEvents(const RWS::CMsg& msg);

private:
    CPhysicsWorld* m_pPhysicsWorld;
};

#endif // PHYSICSWORLD_H
