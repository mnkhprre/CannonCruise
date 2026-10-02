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

// ---------------------------------------------------------------------------
// CPhysicsWorld constructor – the Havok world object is created externally
// and assigned later; we just NULL-init here.
// ---------------------------------------------------------------------------
CPhysicsWorld::CPhysicsWorld()
    : m_pHavokWorld(0)
{
}

// ---------------------------------------------------------------------------
// CPhysicsWorld destructor – asserts that every rigid body was properly
// removed before shutdown. If the assertion fires, it signals a teardown
// ordering bug (some entity forgot to unregister its body).
// ---------------------------------------------------------------------------
CPhysicsWorld::~CPhysicsWorld()
{
    assert(m_avpRigidBodyVector.empty() && "Still rigid bodies in m_avpRigidBodyVector !");
    m_avpRigidBodyVector.clear();
    m_avpPhantomVector.clear();
}

// ---------------------------------------------------------------------------
// Update – steps the Havok physics simulation forward by deltaTime.
// In the original binary (FUN_0043a040) this calls hkpWorld::stepDeltaTime.
// Currently a stub until the Havok integration layer is fully restored.
// ---------------------------------------------------------------------------
void CPhysicsWorld::Update(RwReal deltaTime)
{
    // Havok fizik adımını ilerlet (hkpWorld::stepDeltaTime)
}

// ---------------------------------------------------------------------------
// AddRigidBody – registers a physics body so it participates in simulation.
// Mirrors FUN_0043aa20 in the original binary.
// ---------------------------------------------------------------------------
void CPhysicsWorld::AddRigidBody(void* pBody)
{
    assert(pBody != 0 && "pRigidBody != NULL Failed");
    m_avpRigidBodyVector.push_back(pBody);
}

// ---------------------------------------------------------------------------
// RemoveRigidBody – unregisters a body from the simulation.
// Linear search is acceptable here because bodies are added/removed
// infrequently (ship spawn/destroy).  Mirrors FUN_0043ae30.
// ---------------------------------------------------------------------------
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

// ---------------------------------------------------------------------------
// RigidBodyExistsInWorld – returns TRUE if the given body is registered.
// Used as a safety check before dereferencing body pointers that may have
// been destroyed by another system.
// ---------------------------------------------------------------------------
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

// ---------------------------------------------------------------------------
// AddPhantom – registers a phantom (trigger / sensor volume) with the world.
// Phantoms detect overlapping bodies without generating contact responses.
// ---------------------------------------------------------------------------
void CPhysicsWorld::AddPhantom(void* pPhantom)
{
    assert(pPhantom != 0 && "pPhantom != NULL Failed");
    m_avpPhantomVector.push_back(pPhantom);
}

// ---------------------------------------------------------------------------
// RemovePhantom – unregisters a phantom from the simulation.
// Same linear-search pattern as RemoveRigidBody.
// ---------------------------------------------------------------------------
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

// ---------------------------------------------------------------------------
// Constructor – instantiates the CPhysicsWorld singleton. This behaviour is
// placed in the level by the designers; there should be exactly one instance.
// ---------------------------------------------------------------------------
CPhysicsWorldBehaviour::CPhysicsWorldBehaviour()
    : m_pPhysicsWorld(0)
{
    m_pPhysicsWorld = new CPhysicsWorld();
}

// ---------------------------------------------------------------------------
// Destructor – tears down the physics world. Must happen after all physics
// entities have been destroyed (they unregister their bodies in their dtors).
// ---------------------------------------------------------------------------
CPhysicsWorldBehaviour::~CPhysicsWorldBehaviour()
{
    if (m_pPhysicsWorld)
    {
        delete m_pPhysicsWorld;
        m_pPhysicsWorld = 0;
    }
}

// ---------------------------------------------------------------------------
// HandleEvents – the only message we care about is iMsgPhysicsUpdate.
// The fixed timestep 1/60 s matches the original game's 60 Hz physics rate.
// ---------------------------------------------------------------------------
void CPhysicsWorldBehaviour::HandleEvents(const RWS::CMsg& msg)
{
    // iMsgPhysicsUpdate
    if (m_pPhysicsWorld)
    {
        m_pPhysicsWorld->Update(1.0f / 60.0f);
    }
}
