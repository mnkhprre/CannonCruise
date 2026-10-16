#ifndef HK_PHANTOM_H
#define HK_PHANTOM_H

// ============================================================================
// CannonCruise - Havok Physics 2.2.1 Shim (hkPhantom.h)
// Binary tipler: hkPhantom, hkShapePhantom, hkCachingShapePhantom
// Trigger / sensor volumes — CPhysicsWorld::AddPhantom registers these.
// ============================================================================

#include "hkEntity.h"

class hkShape;

// Bullet bridge — full definition in havokimpl.cpp
class btGhostObject;

/// Detection volume without collision response
class hkPhantom : public hkWorldObject
{
public:
    virtual ~hkPhantom();

    virtual const hkClass* getClass() const;

    /// AABB sorgusu
    virtual void getAabb(hkAabb& aabb) const = 0;

    /// Entities in AABB
    void getPenetratingEntityList(hkEntity** entities, int& numEntities) const;

    /// Position access — trigger volume world position (ghost transform)
    void getPosition(hkVector3& position) const;
    void setPosition(const hkVector3& position);

    /// Bullet bridge — btGhostObject (overlap list source)
    btGhostObject* getBulletGhost() const { return m_pBulletGhost; }

protected:
    hkPhantom();
    explicit hkPhantom(btGhostObject* pGhost);

    btGhostObject* m_pBulletGhost; ///< Bullet bridge
};

/// Shape-based phantom (box / sphere / capsule trigger)
class hkShapePhantom : public hkPhantom
{
public:
    hkShapePhantom(hkShape* pShape);
    virtual ~hkShapePhantom();

    virtual void getAabb(hkAabb& aabb) const;

    hkShape* getShape() const { return m_pShape; }

private:
    hkShape* m_pShape;
};

/// Wide-phase cached phantom (CC.asm: hkCachingShapePhantom)
class hkCachingShapePhantom : public hkShapePhantom
{
public:
    hkCachingShapePhantom(hkShape* pShape);
    virtual ~hkCachingShapePhantom();

    virtual void getAabb(hkAabb& aabb) const;

    void updateCaching();
};

#endif // HK_PHANTOM_H
