#ifndef HK_ENTITY_H
#define HK_ENTITY_H

// ============================================================================
// CannonCruise - Havok Physics 2.2.1 Shim (hkEntity.h)
// Binary tipler: hkWorldObject, hkEntity, hkRigidBody, hkWorld
// Reverse-engineered: CC.asm / CC_ida.asm
//
// SHIM (Phase 4.6h): hkEntity/hkRigidBody are REAL implementations; each holds
// a Bullet rigid body (btRigidBody) and forwards to it.
// getBulletBody() is the bridge accessor. Naming faithful to Havok 2.2.1.
// ============================================================================

#include "hkBaseTypes.h"
#include "hkClass.h"

class hkWorld;
class hkShape;
class hkCollisionFilter;

// Bullet bridge — full definition in havokimpl.cpp
class btRigidBody;

/// Base object addable to world
class hkWorldObject : public hkSerializable
{
public:
    virtual ~hkWorldObject();

    /// Name (Havok 2.x: getName)
    const char* getName() const { return m_name; }
    void setName(const char* name) { m_name = name; }

    /// World the object belongs to
    hkWorld* getWorld() const { return m_world; }

    /// Internal collision mask
    hkUint32 getCollisionFilterInfo() const { return m_collisionFilterInfo; }
    void setCollisionFilterInfo(hkUint32 info) { m_collisionFilterInfo = info; }

    /// Internal world assignment — hkWorld::addEntity/removeEntity use this path
    void setWorld(hkWorld* pWorld) { m_world = pWorld; }

protected:
    hkWorldObject();

    const char* m_name;
    hkWorld* m_world;
    hkUint32 m_collisionFilterInfo;
};

/// Physical object participating in simulation (base class)
class hkEntity : public hkWorldObject
{
public:
    virtual ~hkEntity();

    /// World position
    void getPosition(hkVector3& position) const;
    void setPosition(const hkVector3& position);

    /// Linear velocity (unit: world units / second)
    void getLinearVelocity(hkVector3& velocity) const;
    void setLinearVelocity(const hkVector3& velocity);

    /// Angular velocity
    void getAngularVelocity(hkVector3& velocity) const;
    void setAngularVelocity(const hkVector3& velocity);

    /// Aktivasyon (uyku) durumu
    void setActivated(hkBool activated);
    hkBool isActivated() const;

    /// Shape access
    void setShape(hkShape* pShape);
    hkShape* getShape() const;

    /// Bullet bridge — rigid body representing this entity
    btRigidBody* getBulletBody() const { return m_pBulletBody; }

protected:
    hkEntity();
    /// Construct with Bullet body (hkRigidBody ctor uses this path)
    explicit hkEntity(btRigidBody* pBulletBody);

    hkVector3 m_position;
    hkVector3 m_linearVelocity;
    hkVector3 m_angularVelocity;
    hkBool m_activated;
    hkShape* m_pShape;
    btRigidBody* m_pBulletBody; ///< Bullet bridge
};

/// Sert cisim — oyun CPhysicsEntity::m_pRigidBody bu tipi tutar
class hkRigidBody : public hkEntity
{
public:
    hkRigidBody(const hkVector3& mass, hkShape* pShape);
    virtual ~hkRigidBody();

    virtual const hkClass* getClass() const;

    /// Mass
    hkReal getMass() const { return m_mass; }
    void setMass(hkReal mass);

    /// Inertia diagonal
    void getInertiaDiag(hkVector3& inertia) const;

    /// Zorlama uygula (merkez-of-gravity-relative)
    void applyForce(const hkVector3& force);
    void applyImpulse(const hkVector3& impulse);

private:
    hkReal m_mass;
    hkVector3 m_inertiaDiag;
};

#endif // HK_ENTITY_H
