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

/**
 * @class CPhysicsWorld
 * @brief Singleton wrapper around the Havok physics simulation (hkpWorld).
 *
 * Manages the lifecycle of rigid bodies and phantom volumes used for
 * collision detection. The Update() method advances the Havok simulation
 * by one timestep. Rigid bodies are tracked in a flat vector so that they
 * can be validated (RigidBodyExistsInWorld) before access. This class is
 * a singleton accessed via CSingleton<CPhysicsWorld>.
 */
class CPhysicsWorld : public CSingleton<CPhysicsWorld>
{
public:
    /** @brief Constructor – initialises the Havok world pointer to NULL. */
    CPhysicsWorld();

    /**
     * @brief Destructor – asserts that all rigid bodies have been removed.
     *
     * Clearing the vectors here is a safety measure; leaking bodies would
     * indicate a teardown ordering bug.
     */
    virtual ~CPhysicsWorld();

    /**
     * @brief Advances the physics simulation by deltaTime seconds.
     * @param deltaTime  Time step (the original game uses a fixed 1/60 s).
     *
     * Internally calls hkpWorld::stepDeltaTime (stub in the current build).
     */
    void Update(RwReal deltaTime);

    /**
     * @brief Registers a rigid body with the physics world.
     * @param pBody  Pointer to a Havok hkpRigidBody (cast to void*).
     */
    void AddRigidBody(void* pBody);

    /**
     * @brief Unregisters and removes a rigid body from the simulation.
     * @param pBody  Pointer to the body to remove.
     *
     * Performs a linear search; safe to call even if the body was already
     * removed (the search simply finds nothing).
     */
    void RemoveRigidBody(void* pBody);

    /**
     * @brief Checks whether a rigid body is currently registered.
     * @param pBody  Pointer to the body to look up.
     * @return TRUE if the body is found, FALSE otherwise.
     */
    RwBool RigidBodyExistsInWorld(void* pBody) const;

    /**
     * @brief Registers a phantom (trigger volume) with the physics world.
     * @param pPhantom  Pointer to a Havok hkpPhantom (cast to void*).
     */
    void AddPhantom(void* pPhantom);

    /**
     * @brief Unregisters and removes a phantom from the simulation.
     * @param pPhantom  Pointer to the phantom to remove.
     */
    void RemovePhantom(void* pPhantom);

private:
    std::vector<void*> m_avpRigidBodyVector; ///< All active rigid bodies.
    std::vector<void*> m_avpPhantomVector;   ///< All active phantom volumes.
    void* m_pHavokWorld;                     ///< Opaque pointer to hkpWorld.
};

/**
 * @class CPhysicsWorldBehaviour
 * @brief RWS behaviour that owns and drives the CPhysicsWorld singleton.
 *
 * Registered with the RenderWare Studio class factory so the engine can
 * instantiate it from level data. On each iMsgPhysicsUpdate message it
 * steps the physics world at a fixed 60 Hz timestep.
 */
class CPhysicsWorldBehaviour : public RWS::CEventHandler
{
public:
    /** @brief Constructor – creates the CPhysicsWorld singleton instance. */
    CPhysicsWorldBehaviour();

    /** @brief Destructor – destroys the owned CPhysicsWorld. */
    virtual ~CPhysicsWorldBehaviour();

    /**
     * @brief Steps the physics simulation on each iMsgPhysicsUpdate message.
     * @param msg  Engine message (only iMsgPhysicsUpdate is processed).
     */
    virtual void HandleEvents(const RWS::CMsg& msg);

private:
    CPhysicsWorld* m_pPhysicsWorld; ///< Owned physics world instance.
};

#endif // PHYSICSWORLD_H
