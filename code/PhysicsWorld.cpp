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
#include "PhysicsBackend.h"
#include "framework/core/attributehandler/cclassfactory.h"
#include <cassert>

RWS_REGISTER_CLASS(CPhysicsWorldBehaviour, []() -> void* { return new CPhysicsWorldBehaviour(); }, sizeof(CPhysicsWorldBehaviour));

// ---------------------------------------------------------------------------
// Create – startup zinciri oluşturucusu (binary: sub_41A680, HavokInit
// sonrası alloc 0x440 + ctor sub_49C870 → dword_635AF4'ye singleton yazımı).
// Singleton zaten varsa mevcut örnek döner (ikinci new CSingleton assert
//'i tetiklemez).
// ---------------------------------------------------------------------------
CPhysicsWorld* CPhysicsWorld::Create()
{
    CPhysicsWorld* pExisting = CPhysicsWorld::GetSingletonPtr();
    if (pExisting)
    {
        return pExisting;
    }
    return new CPhysicsWorld();
}

// ---------------------------------------------------------------------------
// CPhysicsWorld constructor – creates the Bullet backend world.
// Gravity -9.8 / solver iter 20 (sub_55E6D0); AABB ±500 documented as
// broadphase clamp gap in PhysicsBackend.cpp.
// ---------------------------------------------------------------------------
CPhysicsWorld::CPhysicsWorld()
    : m_pHavokWorld(0)
{
    m_pHavokWorld = PhysicsWorld_ToOpaque(PhysicsWorld_Create());
}

// ---------------------------------------------------------------------------
// CPhysicsWorld destructor – asserts that every rigid body was properly
// removed before shutdown. If the assertion fires, it signals a teardown
// ordering bug (some entity forgot to unregister its body).
// ---------------------------------------------------------------------------
CPhysicsWorld::~CPhysicsWorld()
{
    assert(m_avpRigidBodyVector.empty() && "Still rigid bodies in m_avpRigidBodyVector !");
    if (m_pHavokWorld)
    {
        PhysicsWorld_Destroy(PhysicsWorld_FromOpaque(m_pHavokWorld));
        m_pHavokWorld = 0;
    }
    m_avpPhantomVector.clear();
}

// ---------------------------------------------------------------------------
// Update – steps the physics simulation forward by deltaTime.
// In the original binary (FUN_0043a040) this calls hkpWorld::stepDeltaTime;
// backend: btDiscreteDynamicsWorld::stepSimulation(dt, 1, dt) — sabit adım.
// ---------------------------------------------------------------------------
void CPhysicsWorld::Update(RwReal deltaTime)
{
    if (m_pHavokWorld)
    {
        PhysicsWorld_Step(PhysicsWorld_FromOpaque(m_pHavokWorld), deltaTime);
    }
}

// ---------------------------------------------------------------------------
// AddRigidBody – registers a physics body so it participates in simulation.
// Mirrors FUN_0043aa20 in the original binary.
// ---------------------------------------------------------------------------
void CPhysicsWorld::AddRigidBody(void* pBody)
{
    assert(pBody != 0 && "pRigidBody != NULL Failed");
    if (m_pHavokWorld)
    {
        PhysicsWorld_AddRigidBody(PhysicsWorld_FromOpaque(m_pHavokWorld), pBody);
    }
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

    if (m_pHavokWorld)
    {
        PhysicsWorld_RemoveRigidBody(PhysicsWorld_FromOpaque(m_pHavokWorld), pBody);
    }

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
// Backend: btGhostObject (CF_NO_CONTACT_RESPONSE).
// ---------------------------------------------------------------------------
void CPhysicsWorld::AddPhantom(void* pPhantom)
{
    assert(pPhantom != 0 && "pPhantom != NULL Failed");
    if (m_pHavokWorld)
    {
        PhysicsWorld_AddTrigger(PhysicsWorld_FromOpaque(m_pHavokWorld), pPhantom);
    }
    m_avpPhantomVector.push_back(pPhantom);
}

// ---------------------------------------------------------------------------
// RemovePhantom – unregisters a phantom from the simulation.
// Same linear-search pattern as RemoveRigidBody.
// ---------------------------------------------------------------------------
void CPhysicsWorld::RemovePhantom(void* pPhantom)
{
    assert(pPhantom != 0 && "pPhantom != NULL Failed");

    if (m_pHavokWorld)
    {
        PhysicsWorld_RemoveTrigger(PhysicsWorld_FromOpaque(m_pHavokWorld), pPhantom);
    }

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
// Constructor – uses the startup-created singleton (binary sub_41A680);
// if absent (seviye/test yolu), creates one and takes ownership.
// Binary'de behaviour ctor'ı (sub_43B860) yalnız container tahsis eder —
// dünya singleton'ı startup'tan gelir.
// ---------------------------------------------------------------------------
CPhysicsWorldBehaviour::CPhysicsWorldBehaviour()
    : m_pPhysicsWorld(0), m_bOwnsWorld(FALSE)
{
    m_pPhysicsWorld = CPhysicsWorld::GetSingletonPtr();
    if (!m_pPhysicsWorld)
    {
        m_pPhysicsWorld = new CPhysicsWorld();
        m_bOwnsWorld = TRUE;
    }
}

// ---------------------------------------------------------------------------
// Destructor – tears down the physics world only if this behaviour created
// it (startup-created singleton shutdown zincirinde lifespan sahibidir —
// TextureCacheShutdown/binary sub_41AB90 dword_635AF4 temizliği).
// ---------------------------------------------------------------------------
CPhysicsWorldBehaviour::~CPhysicsWorldBehaviour()
{
    if (m_pPhysicsWorld && m_bOwnsWorld)
    {
        delete m_pPhysicsWorld;
    }
    m_pPhysicsWorld = 0;
    m_bOwnsWorld = FALSE;
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
