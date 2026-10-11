#ifndef HK_COLLISION_FILTER_H
#define HK_COLLISION_FILTER_H

// ============================================================================
// CannonCruise - Havok Physics 2.2.1 Stub (hkCollisionFilter.h)
// Binary tipler: hkCollisionFilter, hkNullCollisionFilter
// ============================================================================

#include "hkBaseTypes.h"
#include "hkClass.h"

class hkEntity;
class hkPhantom;
class hkShape;

/// Çarpışma çifti filtresi — hangi entity çiftlerinin çarpışacağını belirler
class hkCollisionFilter : public hkSerializable
{
public:
    virtual ~hkCollisionFilter();

    virtual const hkClass* getClass() const;

    /// Bu çarpışma çifti filtrelenmeli mi?
    virtual hkBool isCollisionEnabled(const hkEntity* a, const hkEntity* b) const;
    virtual hkBool isCollisionEnabled(const hkEntity* e, const hkPhantom* p) const;

    /// Şekil düzeyinde filtre
    virtual hkBool isCollisionEnabled(const hkShape* a, const hkShape* b) const;
};

/// Hiçbir şeyi filtrelemeyen varsayılan filtre
class hkNullCollisionFilter : public hkCollisionFilter
{
public:
    virtual ~hkNullCollisionFilter();
    virtual hkBool isCollisionEnabled(const hkEntity* a, const hkEntity* b) const;
};

#endif // HK_COLLISION_FILTER_H
