#ifndef HK_WORLD_H
#define HK_WORLD_H

// ============================================================================
// CannonCruise - Havok Physics 2.2.1 Shim (hkWorld.h)
// Binary: "hkWorld" (CC.asm), FUN_0043a040 (CPhysicsWorld::Update → stepDeltaTime)
// Havok 2.x naming: hkWorld (not hkpWorld — hkp prefix is Havok 4+).
// ============================================================================

#include "hkBaseTypes.h"
#include "hkClass.h"

class hkEntity;
class hkPhantom;
class hkConstraint;
class hkAction;
class hkCollisionFilter;

// Bullet bridge — PhysicsBackend's opaque world handle (full definition in cpp)
struct PhysicsWorldHandle;

/// Physics simulation world
class hkWorld : public hkSerializable
{
public:
    /// Constructor. Havok 2.x world extents given via broad-phase settings.
    hkWorld(hkReal worldMinX = -250.0f, hkReal worldMaxX = 250.0f,
            hkReal worldMinY = -250.0f, hkReal worldMaxY = 250.0f,
            hkReal worldMinZ = -250.0f, hkReal worldMaxZ = 250.0f);
    virtual ~hkWorld();

    virtual const hkClass* getClass() const;

    /// Advance simulation by deltaTime seconds (FUN_0043a040 calls this method)
    void stepDeltaTime(hkReal deltaTime);

    /// Add / remove entity (rigid bodies are hkEntity subclasses)
    hkResult addEntity(hkEntity* pEntity);
    hkResult removeEntity(hkEntity* pEntity);

    /// Phantom (trigger volume) add / remove
    hkResult addPhantom(hkPhantom* pPhantom);
    hkResult removePhantom(hkPhantom* pPhantom);

    /// Constraint and action management
    hkResult addConstraint(hkConstraint* pConstraint);
    hkResult removeConstraint(hkConstraint* pConstraint);
    hkResult addAction(hkAction* pAction);
    hkResult removeAction(hkAction* pAction);

    /// Set collision filter
    void setCollisionFilter(hkCollisionFilter* pFilter);
    hkCollisionFilter* getCollisionFilter() const { return m_pCollisionFilter; }

    /// Threading lock stubs (Havok 2.x checkForWrite family)
    /// Bullet single-threaded → no-op (binary contract preserved).
    void markForWrite();
    void unmarkForWrite();
    void markForRead();
    void unmarkForRead();

    /// Bullet bridge — PhysicsBackend world handle (step/add/remove)
    PhysicsWorldHandle* getPhysicsWorld() const { return m_pPhysicsWorld; }

private:
    hkAabb m_broadPhaseWorld;
    hkCollisionFilter* m_pCollisionFilter;
    PhysicsWorldHandle* m_pPhysicsWorld; ///< Bullet world (PhysicsBackend)
    void* m_pEntities;      ///< internal tracking list (hkEntity)
    void* m_pPhantoms;      ///< internal tracking list (hkPhantom)
    void* m_pConstraints;   ///< internal tracking list (hkConstraint)
    void* m_pActions;       ///< internal tracking list (hkAction)
};

#endif // HK_WORLD_H
