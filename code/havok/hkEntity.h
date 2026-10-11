#ifndef HK_ENTITY_H
#define HK_ENTITY_H

// ============================================================================
// CannonCruise - Havok Physics 2.2.1 Stub (hkEntity.h)
// Binary tipler: hkWorldObject, hkEntity, hkRigidBody, hkWorld
// Tersine mühendislik: CC.asm / CC_ida.asm
// ============================================================================

#include "hkBaseTypes.h"
#include "hkClass.h"

class hkWorld;
class hkShape;
class hkCollisionFilter;

/// Dünyaya eklenebilen temel nesne
class hkWorldObject : public hkSerializable
{
public:
    virtual ~hkWorldObject();

    /// İsim (Havok 2.x: getName)
    const char* getName() const { return m_name; }
    void setName(const char* name) { m_name = name; }

    /// Nesnenin ait olduğu dünya
    hkWorld* getWorld() const { return m_world; }

    /// İç çarpışma maskesi
    hkUint32 getCollisionFilterInfo() const { return m_collisionFilterInfo; }
    void setCollisionFilterInfo(hkUint32 info) { m_collisionFilterInfo = info; }

protected:
    hkWorldObject();

    const char* m_name;
    hkWorld* m_world;
    hkUint32 m_collisionFilterInfo;
};

/// Simülasyona katılan fiziksel varlık (taban sınıf)
class hkEntity : public hkWorldObject
{
public:
    virtual ~hkEntity();

    /// Dünya konumu
    void getPosition(hkVector3& position) const;
    void setPosition(const hkVector3& position);

    /// Doğrusal hız (birim: dünya birimi / saniye)
    void getLinearVelocity(hkVector3& velocity) const;
    void setLinearVelocity(const hkVector3& velocity);

    /// Açısal hız
    void getAngularVelocity(hkVector3& velocity) const;
    void setAngularVelocity(const hkVector3& velocity);

    /// Aktivasyon (uyku) durumu
    void setActivated(hkBool activated);
    hkBool isActivated() const;

    /// Şekil erişimi
    void setShape(hkShape* pShape);
    hkShape* getShape() const;

protected:
    hkEntity();

    hkVector3 m_position;
    hkVector3 m_linearVelocity;
    hkVector3 m_angularVelocity;
    hkBool m_activated;
    hkShape* m_pShape;
};

/// Sert cisim — oyun CPhysicsEntity::m_pRigidBody bu tipi tutar
class hkRigidBody : public hkEntity
{
public:
    hkRigidBody(const hkVector3& mass, hkShape* pShape);
    virtual ~hkRigidBody();

    virtual const hkClass* getClass() const;

    /// Kütle
    hkReal getMass() const { return m_mass; }
    void setMass(hkReal mass) { m_mass = mass; }

    /// Dönme ataleti köşegeni
    void getInertiaDiag(hkVector3& inertia) const;

    /// Zorlama uygula (merkez-of-gravity-relative)
    void applyForce(const hkVector3& force);
    void applyImpulse(const hkVector3& impulse);

private:
    hkReal m_mass;
    hkVector3 m_inertiaDiag;
};

#endif // HK_ENTITY_H
