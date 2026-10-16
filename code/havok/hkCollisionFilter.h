#ifndef HK_COLLISION_FILTER_H
#define HK_COLLISION_FILTER_H

// ============================================================================
// CannonCruise - Havok Physics 2.2.1 Shim (hkCollisionFilter.h)
// Binary tipler: hkCollisionFilter, hkNullCollisionFilter
// ============================================================================

#include "hkBaseTypes.h"
#include "hkClass.h"

class hkEntity;
class hkPhantom;
class hkShape;

/// Collision pair filter — determines which entity pairs collide
class hkCollisionFilter : public hkSerializable
{
public:
    virtual ~hkCollisionFilter();

    virtual const hkClass* getClass() const;

    /// Should this collision pair be filtered?
    virtual hkBool isCollisionEnabled(const hkEntity* a, const hkEntity* b) const;
    virtual hkBool isCollisionEnabled(const hkEntity* e, const hkPhantom* p) const;

    /// Shape-level filter
    virtual hkBool isCollisionEnabled(const hkShape* a, const hkShape* b) const;
};

/// Default filter — filters nothing
class hkNullCollisionFilter : public hkCollisionFilter
{
public:
    virtual ~hkNullCollisionFilter();
    virtual hkBool isCollisionEnabled(const hkEntity* a, const hkEntity* b) const;
};

#endif // HK_COLLISION_FILTER_H
