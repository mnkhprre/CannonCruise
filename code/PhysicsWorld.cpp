// ============================================================================
// CannonCruise - Havok Physics World (PhysicsWorld.cpp)
// Original path: D:\Projects\CannonCruisePC\code\PhysicsWorld.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_0043b9a0 — CPhysicsWorldBehaviour constructor & registration
//   FUN_0043a040 — CPhysicsWorld::Update
//   FUN_0043aa20 — CPhysicsWorld::AddRigidBody
//   FUN_0043ae30 — CPhysicsWorld::RemoveRigidBody
// ============================================================================

#include "PhysicsWorld.h"
#include "framework/core/attributehandler/cclassfactory.h"
#include <cassert>

RWS_REGISTER_CLASS(CPhysicsWorldBehaviour, []() -> void* { return new CPhysicsWorldBehaviour(); }, sizeof(CPhysicsWorldBehaviour));

CPhysicsWorld::CPhysicsWorld()
    : m_pHavokWorld(0)
{
}

CPhysicsWorld::~CPhysicsWorld()
{
    assert(m_avpRigidBodyVector.empty() && "Still rigid bodies in m_avpRigidBodyVector !");
    m_avpRigidBodyVector.clear();
    m_avpPhantomVector.clear();
}

void CPhysicsWorld::Update(RwReal deltaTime)
{
    // Havok fizik adımını ilerlet (hkpWorld::stepDeltaTime)
}

void CPhysicsWorld::AddRigidBody(void* pBody)
{
    assert(pBody != 0 && "pRigidBody != NULL Failed");
    m_avpRigidBodyVector.push_back(pBody);
}

void CPhysicsWorld::RemoveRigidBody(void* pBody)
{
    assert(pBody != 0 && "pRigidBody != NULL Failed");

    for (std::vector<void*>::iterator it = m_avpRigidBodyVector.begin();
         it != m_avpRigidBodyVector.end(); ++it)
    {
        if (*it == pBody)
        {
            m_avpRigidBodyVector.erase(it);
            break;
        }
    }
}

RwBool CPhysicsWorld::RigidBodyExistsInWorld(void* pBody) const
{
    if (!pBody) return FALSE;

    for (size_t i = 0; i < m_avpRigidBodyVector.size(); ++i)
    {
        if (m_avpRigidBodyVector[i] == pBody)
        {
            return TRUE;
        }
    }
    return FALSE;
}

void CPhysicsWorld::AddPhantom(void* pPhantom)
{
    assert(pPhantom != 0 && "pPhantom != NULL Failed");
    m_avpPhantomVector.push_back(pPhantom);
}

void CPhysicsWorld::RemovePhantom(void* pPhantom)
{
    assert(pPhantom != 0 && "pPhantom != NULL Failed");

    for (std::vector<void*>::iterator it = m_avpPhantomVector.begin();
         it != m_avpPhantomVector.end(); ++it)
    {
        if (*it == pPhantom)
        {
            m_avpPhantomVector.erase(it);
            break;
        }
    }
}

// ============================================================================
// CPhysicsWorldBehaviour Implementation
// ============================================================================
CPhysicsWorldBehaviour::CPhysicsWorldBehaviour()
    : m_pPhysicsWorld(0)
{
    m_pPhysicsWorld = new CPhysicsWorld();
}

CPhysicsWorldBehaviour::~CPhysicsWorldBehaviour()
{
    if (m_pPhysicsWorld)
    {
        delete m_pPhysicsWorld;
        m_pPhysicsWorld = 0;
    }
}

void CPhysicsWorldBehaviour::HandleEvents(const RWS::CMsg& msg)
{
    // iMsgPhysicsUpdate
    if (m_pPhysicsWorld)
    {
        m_pPhysicsWorld->Update(1.0f / 60.0f);
    }
}
