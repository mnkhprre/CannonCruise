#ifndef HK_PHANTOM_H
#define HK_PHANTOM_H

// ============================================================================
// CannonCruise - Havok Physics 2.2.1 Stub (hkPhantom.h)
// Binary tipler: hkPhantom, hkShapePhantom, hkCachingShapePhantom
// Tetikleyici / sensör hacimler — CPhysicsWorld::AddPhantom bunları kaydeder.
// ============================================================================

#include "hkEntity.h"

class hkShape;

/// Çarpışma yanıtı üretmeyen algılama hacmi
class hkPhantom : public hkWorldObject
{
public:
    virtual ~hkPhantom();

    /// AABB sorgusu
    virtual void getAabb(hkAabb& aabb) const = 0;

    /// AABB içindeki entity'ler
    void getPenetratingEntityList(hkEntity** entities, int& numEntities) const;

protected:
    hkPhantom();
};

/// Şekil tabanlı phantom (kutu / küre / kapsül tetikleyici)
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

/// Geniş faz önbellekli phantom (CC.asm: hkCachingShapePhantom)
class hkCachingShapePhantom : public hkShapePhantom
{
public:
    hkCachingShapePhantom(hkShape* pShape);
    virtual ~hkCachingShapePhantom();

    virtual void getAabb(hkAabb& aabb) const;

    void updateCaching();
};

#endif // HK_PHANTOM_H
