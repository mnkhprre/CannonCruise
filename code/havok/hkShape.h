#ifndef HK_SHAPE_H
#define HK_SHAPE_H

// ============================================================================
// CannonCruise - Havok Physics 2.2.1 Shim (hkShape.h)
// Binary tipleri (CC.asm / CC_ida.asm):
//   hkShape, hkConvexShape, hkSphereRepShape, hkSphereShape, hkCapsuleShape,
//   hkTriangleShape, hkBoxShape, hkConvexVerticesShape, hkMeshShape,
//   hkShapeCollection, hkBvTreeShape, hkMoppBvTreeShape
// Metotlar: hkShapeCollection::castRay, ::getAabb, ::getMaximumProjection
//
// SHIM (Phase 4.6h): These classes are REAL implementations; each holds a Bullet
// collision shape (btCollisionShape) and forwards to it. Naming and inheritance
// hierarchy stays faithful to Havok 2.2.1 (binary fidelity); underlying engine
// is Bullet. getBulletShape() is the bridge accessor.
// ============================================================================

#include "hkBaseTypes.h"
#include "hkClass.h"

class hkEntity;
class hkPhantom;

// Bullet bridge — full definition in havokimpl.cpp (header kept light).
// NOTE: btCollisionShape is declared as CLASS in Bullet; MSVC mangling
// preserves struct/class distinction (PAU vs PAV) → forward declaration must
// also be 'class', otherwise LNK2019 (2026-10-15).
class btCollisionShape;

/// Collision shape base class
class hkShape : public hkSerializable
{
public:
    enum { TYPE_SPHERE, TYPE_CAPSULE, TYPE_TRIANGLE, TYPE_BOX,
           TYPE_CONVEX_VERTICES, TYPE_MESH, TYPE_BV_TREE, TYPE_MOPP_BV_TREE,
           TYPE_SHAPE_COLLECTION, TYPE_MAX };

    virtual ~hkShape();

    virtual int getType() const = 0;
    virtual void getAabb(hkAabb& aabb) const = 0;

    /// Bullet bridge — subclasses return their btCollisionShape
    virtual btCollisionShape* getBulletShape() const = 0;

    /// Shape casting (Havok 2.x: static castRay helpers)
    static const hkShape* castRay(const hkShape* pShape, const hkVector3& from, const hkVector3& to);
};

/// Convex shape base
class hkConvexShape : public hkShape
{
public:
    virtual ~hkConvexShape();
    virtual int getType() const;
};

/// Sphere rep form (sphere / capsule / triangle shell)
class hkSphereRepShape : public hkConvexShape
{
public:
    virtual ~hkSphereRepShape();
};

/// Sphere shape — sphere-mesh collision agent (hkBvAgent) for
class hkSphereShape : public hkSphereRepShape
{
public:
    hkSphereShape(hkReal radius);
    virtual ~hkSphereShape();
    virtual const hkClass* getClass() const;
    virtual int getType() const;
    virtual void getAabb(hkAabb& aabb) const;
    virtual btCollisionShape* getBulletShape() const;

    hkReal getRadius() const { return m_radius; }

private:
    hkReal m_radius;
    btCollisionShape* m_pBulletShape; ///< btSphereShape
};

/// Capsule shape
class hkCapsuleShape : public hkSphereRepShape
{
public:
    hkCapsuleShape(const hkVector3& start, const hkVector3& end, hkReal radius);
    virtual ~hkCapsuleShape();
    virtual const hkClass* getClass() const;
    virtual int getType() const;
    virtual void getAabb(hkAabb& aabb) const;
    virtual btCollisionShape* getBulletShape() const;

private:
    btCollisionShape* m_pBulletShape; ///< btCapsuleShape
};

/// Triangle shape (static collision geometry)
class hkTriangleShape : public hkSphereRepShape
{
public:
    hkTriangleShape(const hkVector3& v0, const hkVector3& v1, const hkVector3& v2);
    virtual ~hkTriangleShape();
    virtual const hkClass* getClass() const;
    virtual int getType() const;
    virtual void getAabb(hkAabb& aabb) const;
    virtual btCollisionShape* getBulletShape() const;

private:
    btCollisionShape* m_pBulletShape; ///< btTriangleShape
};

/// Box shape — counterpart of game PhysicsShape_CreateBox flow
class hkBoxShape : public hkConvexShape
{
public:
    /// halfExtents: half-axes (see PhysicsBodyBoxDesc::halfExtents)
    hkBoxShape(const hkVector3& halfExtents);
    virtual ~hkBoxShape();
    virtual const hkClass* getClass() const;
    virtual int getType() const;
    virtual void getAabb(hkAabb& aabb) const;
    virtual btCollisionShape* getBulletShape() const;

private:
    btCollisionShape* m_pBulletShape; ///< btBoxShape
};

/// Convex vertex list (generated from DFF collision geometry)
class hkConvexVerticesShape : public hkConvexShape
{
public:
    hkConvexVerticesShape(const hkVector3* vertices, int numVertices,
                          const hkUint32* indices, int numIndices);
    virtual ~hkConvexVerticesShape();
    virtual const hkClass* getClass() const;
    virtual int getType() const;
    virtual void getAabb(hkAabb& aabb) const;
    virtual btCollisionShape* getBulletShape() const;

private:
    btCollisionShape* m_pBulletShape; ///< btConvexHullShape
};

/// Triangle mesh shape (static world geometry)
class hkMeshShape : public hkShape
{
public:
    hkMeshShape(const hkVector3* vertices, int numVertices,
                const hkUint32* indices, int numIndices);
    virtual ~hkMeshShape();
    virtual const hkClass* getClass() const;
    virtual int getType() const;
    virtual void getAabb(hkAabb& aabb) const;
    virtual btCollisionShape* getBulletShape() const;

private:
    void* m_pTriangleMesh;           ///< btTriangleMesh (owned by shape)
    btCollisionShape* m_pBulletShape; ///< btBvhTriangleMeshShape
};

/// Shape collection base — castRay / getMaximumProjection methods in ASM
class hkShapeCollection : public hkShape
{
public:
    virtual ~hkShapeCollection();
    virtual const hkClass* getClass() const;
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
    virtual const hkClass* getClass() const;
    virtual int getType() const;
    virtual void getAabb(hkAabb& aabb) const;
    /// Koleksiyonun şekli devredilir (BV ağaç Bullet'te BVH içinde)
    virtual btCollisionShape* getBulletShape() const;

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
    hkUint8* m_pMoppCode; ///< MOPP kodu (Binary blob; Bullet BVH'a sıkıştırma yok)
    int m_moppSize;
};

#endif // HK_SHAPE_H
