#ifndef HK_CONSTRAINT_H
#define HK_CONSTRAINT_H

// ============================================================================
// CannonCruise - Havok Physics 2.2.1 Stub (hkConstraint.h)
// Binary tipler: hkConstraint, hkAction
// ============================================================================

#include "hkBaseTypes.h"
#include "hkClass.h"

class hkEntity;
class hkWorld;

/// Eklem / bağlayıcı — iki entity arasında ilişki kurar
class hkConstraint : public hkSerializable
{
public:
    hkConstraint(hkEntity* pEntityA, hkEntity* pEntityB);
    virtual ~hkConstraint();

    virtual const hkClass* getClass() const;

    hkEntity* getEntityA() const { return m_pEntityA; }
    hkEntity* getEntityB() const { return m_pEntityB; }

protected:
    hkEntity* m_pEntityA;
    hkEntity* m_pEntityB;
};

/// Fizik aksiyonu — entity'ye bağlı davranış (tetikleyici, sürtünme vb.)
class hkAction : public hkSerializable
{
public:
    explicit hkAction(hkEntity* pEntity);
    virtual ~hkAction();

    virtual const hkClass* getClass() const;

    /// Simülasyon adımında çağrılır
    virtual void apply(hkReal deltaTime) = 0;

    hkEntity* getEntity() const { return m_pEntity; }

protected:
    hkEntity* m_pEntity;
};

#endif // HK_CONSTRAINT_H
