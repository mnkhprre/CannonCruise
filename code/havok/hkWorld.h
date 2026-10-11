#ifndef HK_WORLD_H
#define HK_WORLD_H

// ============================================================================
// CannonCruise - Havok Physics 2.2.1 Stub (hkWorld.h)
// Binary: "hkWorld" (CC.asm), FUN_0043a040 (CPhysicsWorld::Update → stepDeltaTime)
// Havok 2.x adlandırması: hkWorld (hkpWorld değil — hkp öneki Havok 4+).
// ============================================================================

#include "hkBaseTypes.h"
#include "hkClass.h"

class hkEntity;
class hkPhantom;
class hkConstraint;
class hkAction;
class hkCollisionFilter;

/// Fizik simülasyon dünyası
class hkWorld : public hkSerializable
{
public:
    /// Yapılandırıcı. Havok 2.x'te dünya genişliği Broad-phase ayarlarıyla verilir.
    hkWorld(hkReal worldMinX = -250.0f, hkReal worldMaxX = 250.0f,
            hkReal worldMinY = -250.0f, hkReal worldMaxY = 250.0f,
            hkReal worldMinZ = -250.0f, hkReal worldMaxZ = 250.0f);
    virtual ~hkWorld();

    virtual const hkClass* getClass() const;

    /// Simülasyonu deltaTime saniye ilerlet (FUN_0043a040 bu metodu çağırır)
    void stepDeltaTime(hkReal deltaTime);

    /// Entity ekle / çıkar (rigid body'ler hkEntity alt sınıfıdır)
    hkResult addEntity(hkEntity* pEntity);
    hkResult removeEntity(hkEntity* pEntity);

    /// Phantom (tetikleyici hacim) ekle / çıkar
    hkResult addPhantom(hkPhantom* pPhantom);
    hkResult removePhantom(hkPhantom* pPhantom);

    /// Bağlayıcı ve aksiyon yönetimi
    hkResult addConstraint(hkConstraint* pConstraint);
    hkResult removeConstraint(hkConstraint* pConstraint);
    hkResult addAction(hkAction* pAction);
    hkResult removeAction(hkAction* pAction);

    /// Çarpışma filtresini ayarla
    void setCollisionFilter(hkCollisionFilter* pFilter);

    /// İş parçacığı kilidi yer tutucuları (Havok 2.x checkForWrite ailesi)
    void markForWrite();
    void unmarkForWrite();
    void markForRead();
    void unmarkForRead();

private:
    hkAabb m_broadPhaseWorld;
    hkCollisionFilter* m_pCollisionFilter;
    void* m_pEntities;      ///< Internal linked list (opaque)
    void* m_pPhantoms;      ///< Internal linked list (opaque)
    void* m_pConstraints;
};

#endif // HK_WORLD_H
