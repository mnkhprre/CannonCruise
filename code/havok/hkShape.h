#ifndef HK_SHAPE_H
#define HK_SHAPE_H

// ============================================================================
// CannonCruise - Havok Physics 2.2.1 Stub (hkShape.h)
// Binary tipleri (CC.asm / CC_ida.asm):
//   hkShape, hkConvexShape, hkSphereRepShape, hkSphereShape, hkCapsuleShape,
//   hkTriangleShape, hkConvexVerticesShape, hkMeshShape, hkShapeCollection,
//   hkBvTreeShape, hkMoppBvTreeShape
// Metotlar: hkShapeCollection::castRay, ::getAabb, ::getMaximumProjection
// ============================================================================

#include "hkBaseTypes.h"
#include "hkClass.h"

class hkEntity;
class hkPhantom;

/// Çarpışma şekli taban sınıfı
class hkShape : public hkSerializable
{
public:
    enum { TYPE_SPHERE, TYPE_CAPSULE, TYPE_TRIANGLE, TYPE_BOX,
           TYPE_CONVEX_VERTICES, TYPE_MESH, TYPE_BV_TREE, TYPE_MOPP_BV_TREE,
           TYPE_SHAPE_COLLECTION, TYPE_MAX };

    virtual ~hkShape();

    virtual int getType() const = 0;
    virtual void getAabb(hkAabb& aabb) const = 0;

    /// Şekil casting (Havok 2.x: static castRay yardımcıları)
    static const hkShape* castRay(const hkShape* pShape, const hkVector3& from, const hkVector3& to);
};

/// Konveks şekil tabanı
class hkConvexShape : public hkShape
{
public:
    virtual ~hkConvexShape();
    virtual int getType() const;
};

/// Küre rep forması (küre / kapsül / üçgen kabuğu)
class hkSphereRepShape : public hkConvexShape
{
public:
    virtual ~hkSphereRepShape();
};

/// Küre şekil — sphere-mesh çarpışma ajanı (hkBvAgent) için
class hkSphereShape : public hkSphereRepShape
{
public:
    hkSphereShape(hkReal radius);
    virtual ~hkSphereShape();
    virtual void getAabb(hkAabb& aabb) const;

    hkReal getRadius() const { return m_radius; }

private:
    hkReal m_radius;
};

/// Kapsül şekil
class hkCapsuleShape : public hkSphereRepShape
{
public:
    hkCapsuleShape(const hkVector3& start, const hkVector3& end, hkReal radius);
    virtual ~hkCapsuleShape();
    virtual void getAabb(hkAabb& aabb) const;
};

/// Üçgen şekil (statik çarpışma geometrisi)
class hkTriangleShape : public hkSphereRepShape
{
public:
    hkTriangleShape(const hkVector3& v0, const hkVector3& v1, const hkVector3& v2);
    virtual ~hkTriangleShape();
    virtual void getAabb(hkAabb& aabb) const;
};

/// Konveks köşe listesi (DFF çarpışma geometrisinden üretilir)
class hkConvexVerticesShape : public hkConvexShape
{
public:
    hkConvexVerticesShape(const hkVector3* vertices, int numVertices,
                          const hkUint32* indices, int numIndices);
    virtual ~hkConvexVerticesShape();
    virtual int getType() const;
    virtual void getAabb(hkAabb& aabb) const;
};

/// Üçgen mesh şekil (statik dünya geometrisi)
class hkMeshShape : public hkShape
{
public:
    hkMeshShape(const hkVector3* vertices, int numVertices,
                const hkUint32* indices, int numIndices);
    virtual ~hkMeshShape();
    virtual int getType() const;
    virtual void getAabb(hkAabb& aabb) const;
};

/// Şekil koleksiyonu tabanı — castRay / getMaximumProjection metotları ASM'de
class hkShapeCollection : public hkShape
{
public:
    virtual ~hkShapeCollection();
    virtual int getType() const;

    virtual hkBool castRay(const hkVector3& from, const hkVector3& to, hkEntity*& hit) const = 0;
    virtual hkReal getMaximumProjection(const hkVector3& direction) const = 0;
};

/// BV ağaç şekli (broad-phase hızlandırması)
class hkBvTreeShape : public hkShape
{
public:
    explicit hkBvTreeShape(hkShapeCollection* pCollection);
    virtual ~hkBvTreeShape();
    virtual int getType() const;
    virtual void getAabb(hkAabb& aabb) const;

protected:
    hkShapeCollection* m_pCollection;
};

/// MOPP (Memory Optimized Partial Polytope) BV ağaç — sıkıştırılmış broad-phase
class hkMoppBvTreeShape : public hkBvTreeShape
{
public:
    hkMoppBvTreeShape(hkShapeCollection* pCollection, const hkUint8* moppCode, int moppSize);
    virtual ~hkMoppBvTreeShape();
    virtual int getType() const;

private:
    hkUint8* m_pMoppCode;
    int m_moppSize;
};

#endif // HK_SHAPE_H
