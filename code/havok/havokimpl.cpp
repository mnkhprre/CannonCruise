// ============================================================================
// CannonCruise - Havok Physics 2.2.1 Shim Implementation (havokimpl.cpp)
//
// This file is the REAL implementation of the Havok 2.2.1 API surface
// reverse-engineered from binary (CC.asm / CC_ida.asm) in code/havok/*.h.
//
// PHILOSOPHY:
//   - Naming and inheritance hierarchy stays faithful to Havok 2.2.1
//     (hkWorld/hkEntity/hkRigidBody/hkPhantom — NO "hkp" prefix).
//     This lets reverse-engineered game code talk with the same names the
//     original binary calls (binary fidelity).
//   - Underlying ENGINE is Bullet 3.25 (MIT, portable). Each Havok class
//     holds a Bullet object and forwards to it.
//   - hkWorld uses tested PhysicsBackend world (PhysicsBackend.cpp) → single
//     code path, existing 22 tests unaffected.
//
// BINARY PARAMETER SOURCES (sub_55E6D0 game physics defaults):
//   [+0x04] gravity = -9.8   [+0x44] friction = 0.6   [+0x58] restitution = 0.3
//   [+0x80] solver iter = 20  (AABB ±500 broadphase clamp not yet applied)
//
// DOCUMENTED DEVIATIONS:
//   - Bullet sequential-impulse solver ≠ Havok 2004 solver — identical feel
//     not guaranteed.
//   - hkCapsuleShape: Havok segment-based, Bullet center+Y-axis → approximated
//     as height |end-start|.
//   - hkConvexVerticesShape: indices ignored (hull generated from points).
//   - hkConstraint/hkAction: tracking-focused (game doesn't use constraints yet).
//   - No MOPP compression (Bullet BVH used); moppCode stored.
// ============================================================================

#include "havok.h"

#include "PhysicsBackend.h"

#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <BulletCollision/CollisionShapes/btBoxShape.h>
#include <BulletCollision/CollisionShapes/btSphereShape.h>
#include <BulletCollision/CollisionShapes/btCapsuleShape.h>
#include <BulletCollision/CollisionShapes/btTriangleShape.h>
#include <BulletCollision/CollisionShapes/btConvexHullShape.h>
#include <BulletCollision/CollisionShapes/btTriangleMesh.h>
#include <BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include <LinearMath/btDefaultMotionState.h>
#include <LinearMath/btTransform.h>

#include <cmath>
#include <cstdlib>
#include <cstring>

namespace
{
    // sub_55E6D0 defaults (same contract as PhysicsBackend)
    const btScalar kGravityY = -9.8;    // [+0x04]
    const btScalar kFriction = 0.6;     // [+0x44]
    const btScalar kRestitution = 0.3;  // [+0x58]

    btVector3 ToBt(const hkVector3& v)
    {
        return btVector3(v.x, v.y, v.z);
    }

    hkVector3 ToHk(const btVector3& v)
    {
        return hkVector3((hkReal)v.x(), (hkReal)v.y(), (hkReal)v.z());
    }

    // Pass btCollisionShape AABB to hkAabb (identity transform)
    void BulletShapeAabb(const btCollisionShape* pShape, hkAabb& aabb)
    {
        btVector3 mn, mx;
        pShape->getAabb(btTransform::getIdentity(), mn, mx);
        aabb.m_min = ToHk(mn);
        aabb.m_max = ToHk(mx);
    }

    // Scalar mass from mass vector (Havok 2.x: x-axis is main mass;
    // infinite/huge → static). Bullet mass=0 is static.
    btScalar ScalarMass(const hkVector3& mass)
    {
        // Havok static body usually mass 0 or large positive
        // (inertia not calculated). 0 or negative → static.
        return (mass.x > 0.0f && mass.x < 1e30f) ? mass.x : 0.0f;
    }

    // Build btRigidBody from mass + shape (identity position; friction/restitution
    // from sub_55E6D0). Caller adjusts position via setPosition if init info provided.
    btRigidBody* BuildRigidBody(btScalar mass, btCollisionShape* pShape)
    {
        btVector3 localInertia(0, 0, 0);
        if (mass != 0.0f && pShape) {
            pShape->calculateLocalInertia(mass, localInertia);
        }
        btDefaultMotionState* pMotion =
            new btDefaultMotionState(btTransform::getIdentity());
        btRigidBody::btRigidBodyConstructionInfo info(mass, pMotion, pShape,
                                                      localInertia);
        info.m_friction = kFriction;
        info.m_restitution = kRestitution;
        btRigidBody* pBody = new btRigidBody(info);
        pBody->setActivationState(DISABLE_DEACTIVATION);
        return pBody;
    }

    // btCollisionObject* → hkEntity* registration (for phantom overlap mapping)
    struct EntityRegEntry {
        btCollisionObject* pObj;
        hkEntity* pEntity;
    };
    EntityRegEntry* g_pEntityReg = 0;
    int g_entityRegCount = 0;
    int g_entityRegCap = 0;

    void EntityRegAdd(btCollisionObject* pObj, hkEntity* pEntity)
    {
        if (g_entityRegCount == g_entityRegCap) {
            const int newCap = g_entityRegCap ? g_entityRegCap * 2 : 64;
            EntityRegEntry* pNew = new EntityRegEntry[newCap];
            for (int i = 0; i < g_entityRegCount; ++i) pNew[i] = g_pEntityReg[i];
            delete[] g_pEntityReg;
            g_pEntityReg = pNew;
            g_entityRegCap = newCap;
        }
        g_pEntityReg[g_entityRegCount].pObj = pObj;
        g_pEntityReg[g_entityRegCount].pEntity = pEntity;
        ++g_entityRegCount;
    }

    void EntityRegRemove(hkEntity* pEntity)
    {
        for (int i = 0; i < g_entityRegCount; ++i) {
            if (g_pEntityReg[i].pEntity == pEntity) {
                g_pEntityReg[i] = g_pEntityReg[g_entityRegCount - 1];
                --g_entityRegCount;
                return;
            }
        }
    }

    hkEntity* EntityRegFind(btCollisionObject* pObj)
    {
        for (int i = 0; i < g_entityRegCount; ++i) {
            if (g_pEntityReg[i].pObj == pObj) return g_pEntityReg[i].pEntity;
        }
        return 0;
    }
} // namespace

// ---------------------------------------------------------------------------
// hkMemory — memory singleton (HavokInit uses this interface)
// ---------------------------------------------------------------------------
namespace
{
    // Default allocator — works with CHavokMemoryManager
    class DefaultHkMemory : public hkMemory
    {
    public:
        virtual void* HK_CALL allocate(size_t size) { return std::malloc(size); }
        virtual void HK_CALL deallocate(void* p) { std::free(p); }
        virtual void* HK_CALL reallocate(void* p, size_t size)
        {
            return std::realloc(p, size);
        }
    };

    DefaultHkMemory* g_pMemory = 0;
}

void hkMemory::init()
{
    if (!g_pMemory) {
        g_pMemory = new DefaultHkMemory();
    }
}

void hkMemory::quit()
{
    delete g_pMemory;
    g_pMemory = 0;
}

hkMemory* hkMemory::getInstance()
{
    if (!g_pMemory) {
        init();
    }
    return g_pMemory;
}

// ---------------------------------------------------------------------------
// hkShape hierarchy — Bullet collision shapes
// ---------------------------------------------------------------------------
hkShape::~hkShape() {}

const hkShape* hkShape::castRay(const hkShape* pShape,
                                const hkVector3& from, const hkVector3& to)
{
    // Ray cast not yet wired (Phase 4.6). Bullet raycast on separate path.
    (void)pShape; (void)from; (void)to;
    return 0;
}

hkConvexShape::~hkConvexShape() {}
int hkConvexShape::getType() const { return TYPE_CONVEX_VERTICES; }

hkSphereRepShape::~hkSphereRepShape() {}

// --- hkSphereShape ---
hkSphereShape::hkSphereShape(hkReal radius)
    : m_radius(radius),
      m_pBulletShape(new btSphereShape(radius)) {}

hkSphereShape::~hkSphereShape() { delete m_pBulletShape; }
int hkSphereShape::getType() const { return TYPE_SPHERE; }
const hkClass* hkSphereShape::getClass() const
{
    static const hkClass kCls("hkSphereShape", 0, 0, 0,
                              (int)sizeof(hkSphereShape));
    return &kCls;
}
btCollisionShape* hkSphereShape::getBulletShape() const { return m_pBulletShape; }
void hkSphereShape::getAabb(hkAabb& aabb) const
{
    BulletShapeAabb(m_pBulletShape, aabb);
}

// --- hkCapsuleShape ---
hkCapsuleShape::hkCapsuleShape(const hkVector3& start, const hkVector3& end,
                               hkReal radius)
{
    // DEVIATION: Havok segment (start/end) based; Bullet center + Y-axis.
    // Height = |end-start|; center = midpoint.
    const btVector3 s = ToBt(start);
    const btVector3 e = ToBt(end);
    const btScalar len = (e - s).length();
    // btCapsuleShape wants half-length (height = 2*half-length NOT;
    // ctor parameter is half-length). |end-start| passed directly.
    m_pBulletShape = new btCapsuleShape(radius, len > 0.0f ? len : 0.0f);
}

hkCapsuleShape::~hkCapsuleShape() { delete m_pBulletShape; }
int hkCapsuleShape::getType() const { return TYPE_CAPSULE; }
const hkClass* hkCapsuleShape::getClass() const
{
    static const hkClass kCls("hkCapsuleShape", 0, 0, 0,
                              (int)sizeof(hkCapsuleShape));
    return &kCls;
}
btCollisionShape* hkCapsuleShape::getBulletShape() const { return m_pBulletShape; }
void hkCapsuleShape::getAabb(hkAabb& aabb) const
{
    BulletShapeAabb(m_pBulletShape, aabb);
}

// --- hkTriangleShape ---
hkTriangleShape::hkTriangleShape(const hkVector3& v0, const hkVector3& v1,
                                 const hkVector3& v2)
    : m_pBulletShape(new btTriangleShape(ToBt(v0), ToBt(v1), ToBt(v2))) {}

hkTriangleShape::~hkTriangleShape() { delete m_pBulletShape; }
int hkTriangleShape::getType() const { return TYPE_TRIANGLE; }
const hkClass* hkTriangleShape::getClass() const
{
    static const hkClass kCls("hkTriangleShape", 0, 0, 0,
                              (int)sizeof(hkTriangleShape));
    return &kCls;
}
btCollisionShape* hkTriangleShape::getBulletShape() const { return m_pBulletShape; }
void hkTriangleShape::getAabb(hkAabb& aabb) const
{
    BulletShapeAabb(m_pBulletShape, aabb);
}

// --- hkBoxShape ---
hkBoxShape::hkBoxShape(const hkVector3& halfExtents)
    : m_pBulletShape(new btBoxShape(ToBt(halfExtents))) {}

hkBoxShape::~hkBoxShape() { delete m_pBulletShape; }
int hkBoxShape::getType() const { return TYPE_BOX; }
const hkClass* hkBoxShape::getClass() const
{
    static const hkClass kCls("hkBoxShape", 0, 0, 0, (int)sizeof(hkBoxShape));
    return &kCls;
}
btCollisionShape* hkBoxShape::getBulletShape() const { return m_pBulletShape; }
void hkBoxShape::getAabb(hkAabb& aabb) const
{
    BulletShapeAabb(m_pBulletShape, aabb);
}

// --- hkConvexVerticesShape ---
hkConvexVerticesShape::hkConvexVerticesShape(const hkVector3* vertices,
                                             int numVertices,
                                             const hkUint32* indices,
                                             int numIndices)
{
    // DEVIATION: indices ignored; hull generated directly from points.
    (void)indices; (void)numIndices;
    btConvexHullShape* pHull = new btConvexHullShape();
    for (int i = 0; i < numVertices; ++i) {
        pHull->addPoint(ToBt(vertices[i]), /*recalcLocalAabb=*/false);
    }
    pHull->recalcLocalAabb();
    m_pBulletShape = pHull;
}

hkConvexVerticesShape::~hkConvexVerticesShape() { delete m_pBulletShape; }
int hkConvexVerticesShape::getType() const { return TYPE_CONVEX_VERTICES; }
const hkClass* hkConvexVerticesShape::getClass() const
{
    static const hkClass kCls("hkConvexVerticesShape", 0, 0, 0,
                              (int)sizeof(hkConvexVerticesShape));
    return &kCls;
}
btCollisionShape* hkConvexVerticesShape::getBulletShape() const
{
    return m_pBulletShape;
}
void hkConvexVerticesShape::getAabb(hkAabb& aabb) const
{
    BulletShapeAabb(m_pBulletShape, aabb);
}

// --- hkMeshShape ---
hkMeshShape::hkMeshShape(const hkVector3* vertices, int numVertices,
                         const hkUint32* indices, int numIndices)
    : m_pTriangleMesh(0), m_pBulletShape(0)
{
    btTriangleMesh* pMesh = new btTriangleMesh();
    for (int i = 0; i + 2 < numIndices; i += 3) {
        pMesh->addTriangle(ToBt(vertices[indices[i]]),
                           ToBt(vertices[indices[i + 1]]),
                           ToBt(vertices[indices[i + 2]]));
    }
    m_pTriangleMesh = pMesh;
    m_pBulletShape = new btBvhTriangleMeshShape(pMesh, /*useQuantized=*/true);
}

hkMeshShape::~hkMeshShape()
{
    delete m_pBulletShape;  // shape mesh not copied → shape first
    delete static_cast<btTriangleMesh*>(m_pTriangleMesh);
}

int hkMeshShape::getType() const { return TYPE_MESH; }
const hkClass* hkMeshShape::getClass() const
{
    static const hkClass kCls("hkMeshShape", 0, 0, 0,
                              (int)sizeof(hkMeshShape));
    return &kCls;
}
btCollisionShape* hkMeshShape::getBulletShape() const { return m_pBulletShape; }
void hkMeshShape::getAabb(hkAabb& aabb) const
{
    BulletShapeAabb(m_pBulletShape, aabb);
}

// --- hkShapeCollection / hkBvTreeShape / hkMoppBvTreeShape ---
hkShapeCollection::~hkShapeCollection() {}
int hkShapeCollection::getType() const { return TYPE_SHAPE_COLLECTION; }
const hkClass* hkShapeCollection::getClass() const
{
    static const hkClass kCls("hkShapeCollection", 0, 0, 0,
                              (int)sizeof(hkShapeCollection));
    return &kCls;
}

hkBvTreeShape::hkBvTreeShape(hkShapeCollection* pCollection)
    : m_pCollection(pCollection) {}

hkBvTreeShape::~hkBvTreeShape() {}
int hkBvTreeShape::getType() const { return TYPE_BV_TREE; }
const hkClass* hkBvTreeShape::getClass() const
{
    static const hkClass kCls("hkBvTreeShape", 0, 0, 0,
                              (int)sizeof(hkBvTreeShape));
    return &kCls;
}
btCollisionShape* hkBvTreeShape::getBulletShape() const
{
    // Bullet's BV tree shape is in BVH; collection's shape forwarded.
    return m_pCollection ? m_pCollection->getBulletShape() : 0;
}
void hkBvTreeShape::getAabb(hkAabb& aabb) const
{
    if (m_pCollection) {
        m_pCollection->getAabb(aabb);
    } else {
        aabb.m_min = hkVector3(0, 0, 0);
        aabb.m_max = hkVector3(0, 0, 0);
    }
}

hkMoppBvTreeShape::hkMoppBvTreeShape(hkShapeCollection* pCollection,
                                     const hkUint8* moppCode, int moppSize)
    : hkBvTreeShape(pCollection),
      m_pMoppCode(0), m_moppSize(moppSize)
{
    if (moppCode && moppSize > 0) {
        m_pMoppCode = static_cast<hkUint8*>(std::malloc(moppSize));
        if (m_pMoppCode) std::memcpy(m_pMoppCode, moppCode, moppSize);
    }
}

hkMoppBvTreeShape::~hkMoppBvTreeShape() { std::free(m_pMoppCode); }
int hkMoppBvTreeShape::getType() const { return TYPE_MOPP_BV_TREE; }

// ---------------------------------------------------------------------------
// hkWorldObject / hkEntity / hkRigidBody — Bullet rigid bodies
// ---------------------------------------------------------------------------
hkWorldObject::hkWorldObject()
    : m_name(0), m_world(0), m_collisionFilterInfo(0) {}

hkWorldObject::~hkWorldObject() {}

hkEntity::hkEntity()
    : m_position(), m_linearVelocity(), m_angularVelocity(),
      m_activated(1), m_pShape(0), m_pBulletBody(0) {}

hkEntity::hkEntity(btRigidBody* pBulletBody)
    : m_position(), m_linearVelocity(), m_angularVelocity(),
      m_activated(1), m_pShape(0), m_pBulletBody(pBulletBody)
{
    if (m_pBulletBody) {
        EntityRegAdd(m_pBulletBody, this);
    }
}

hkEntity::~hkEntity()
{
    EntityRegRemove(this);
    delete m_pBulletBody;
    m_pBulletBody = 0;
}

void hkEntity::getPosition(hkVector3& position) const
{
    if (m_pBulletBody) {
        position = ToHk(m_pBulletBody->getWorldTransform().getOrigin());
    }
}

void hkEntity::setPosition(const hkVector3& position)
{
    if (!m_pBulletBody) return;
    btTransform t = m_pBulletBody->getWorldTransform();
    t.setOrigin(ToBt(position));
    m_pBulletBody->setWorldTransform(t);
    if (m_pBulletBody->getMotionState()) {
        m_pBulletBody->getMotionState()->setWorldTransform(t);
    }
}

void hkEntity::getLinearVelocity(hkVector3& velocity) const
{
    if (m_pBulletBody) {
        velocity = ToHk(m_pBulletBody->getLinearVelocity());
    }
}

void hkEntity::setLinearVelocity(const hkVector3& velocity)
{
    if (m_pBulletBody) m_pBulletBody->setLinearVelocity(ToBt(velocity));
}

void hkEntity::getAngularVelocity(hkVector3& velocity) const
{
    if (m_pBulletBody) {
        velocity = ToHk(m_pBulletBody->getAngularVelocity());
    }
}

void hkEntity::setAngularVelocity(const hkVector3& velocity)
{
    if (m_pBulletBody) m_pBulletBody->setAngularVelocity(ToBt(velocity));
}

void hkEntity::setActivated(hkBool activated)
{
    if (!m_pBulletBody) return;
    if (activated) {
        m_pBulletBody->activate(true);
        m_pBulletBody->setActivationState(DISABLE_DEACTIVATION);
    } else {
        m_pBulletBody->setActivationState(ISLAND_SLEEPING);
    }
    m_activated = activated;
}

hkBool hkEntity::isActivated() const
{
    return m_pBulletBody ? (m_pBulletBody->isActive() ? 1 : 0) : m_activated;
}

void hkEntity::setShape(hkShape* pShape)
{
    m_pShape = pShape;
    if (m_pBulletBody && pShape && pShape->getBulletShape()) {
        m_pBulletBody->setCollisionShape(pShape->getBulletShape());
        // inertia recalculated (mass preserved)
        const btScalar m = m_pBulletBody->getMass();
        if (m != 0.0f) {
            btVector3 inertia(0, 0, 0);
            pShape->getBulletShape()->calculateLocalInertia(m, inertia);
            m_pBulletBody->setMassProps(m, inertia);
        }
    }
}

hkShape* hkEntity::getShape() const { return m_pShape; }

// --- hkRigidBody ---
hkRigidBody::hkRigidBody(const hkVector3& mass, hkShape* pShape)
    : hkEntity(BuildRigidBody(ScalarMass(mass),
                              pShape ? pShape->getBulletShape() : 0)),
      m_mass(mass.x), m_inertiaDiag()
{
    m_pShape = pShape;
    if (m_pBulletBody && m_pBulletBody->getMass() != 0.0f) {
        const btVector3 inv = m_pBulletBody->getInvInertiaDiagLocal();
        // inverse inertia → inertia diagonal
        m_inertiaDiag.x = (inv.x() != 0.0f) ? (hkReal)(1.0f / inv.x()) : 0.0f;
        m_inertiaDiag.y = (inv.y() != 0.0f) ? (hkReal)(1.0f / inv.y()) : 0.0f;
        m_inertiaDiag.z = (inv.z() != 0.0f) ? (hkReal)(1.0f / inv.z()) : 0.0f;
    }
}

hkRigidBody::~hkRigidBody() {}

const hkClass* hkRigidBody::getClass() const
{
    static const hkClass kCls("hkRigidBody", 0, 0, 0,
                              (int)sizeof(hkRigidBody));
    return &kCls;
}

void hkRigidBody::setMass(hkReal mass)
{
    m_mass = mass;
    if (!m_pBulletBody) return;
    const btScalar bm = (mass > 0.0f && mass < 1e30f) ? mass : 0.0f;
    btVector3 inertia(0, 0, 0);
    if (bm != 0.0f && m_pBulletBody->getCollisionShape()) {
        m_pBulletBody->getCollisionShape()->calculateLocalInertia(bm, inertia);
    }
    m_pBulletBody->setMassProps(bm, inertia);
}

void hkRigidBody::getInertiaDiag(hkVector3& inertia) const
{
    inertia = m_inertiaDiag;
}

void hkRigidBody::applyForce(const hkVector3& force)
{
    if (m_pBulletBody) m_pBulletBody->applyCentralForce(ToBt(force));
}

void hkRigidBody::applyImpulse(const hkVector3& impulse)
{
    if (m_pBulletBody) m_pBulletBody->applyCentralImpulse(ToBt(impulse));
}

// ---------------------------------------------------------------------------
// hkWorld — built on PhysicsBackend world
// ---------------------------------------------------------------------------
hkWorld::hkWorld(hkReal worldMinX, hkReal worldMaxX,
                 hkReal worldMinY, hkReal worldMaxY,
                 hkReal worldMinZ, hkReal worldMaxZ)
    : m_pCollisionFilter(0), m_pPhysicsWorld(0),
      m_pEntities(0), m_pPhantoms(0), m_pConstraints(0), m_pActions(0)
{
    m_broadPhaseWorld.m_min = hkVector3(worldMinX, worldMinY, worldMinZ);
    m_broadPhaseWorld.m_max = hkVector3(worldMaxX, worldMaxY, worldMaxZ);
    // Tested PhysicsBackend world — gravity/solver/ghost callback
    // set up in PhysicsWorld_Create (sub_55E6D0 defaults).
    m_pPhysicsWorld = PhysicsWorld_Create();
    PhysicsWorld_SetGravity(m_pPhysicsWorld, 0.0f, kGravityY, 0.0f);
}

hkWorld::~hkWorld()
{
    PhysicsWorld_Destroy(m_pPhysicsWorld);
    m_pPhysicsWorld = 0;
}

const hkClass* hkWorld::getClass() const
{
    static const hkClass kCls("hkWorld", 0, 0, 0, (int)sizeof(hkWorld));
    return &kCls;
}

void hkWorld::stepDeltaTime(hkReal deltaTime)
{
    // FUN_0043a040 (CPhysicsWorld::Update) → hkpWorld::stepDeltaTime
    PhysicsWorld_Step(m_pPhysicsWorld, deltaTime);
}

hkResult hkWorld::addEntity(hkEntity* pEntity)
{
    if (!pEntity || !pEntity->getBulletBody()) {
        return hkResult(hkResult::FAIL);
    }
    pEntity->setWorld(this);
    PhysicsWorld_AddRigidBody(m_pPhysicsWorld, pEntity->getBulletBody());
    return hkResult(hkResult::SUCCESS);
}

hkResult hkWorld::removeEntity(hkEntity* pEntity)
{
    if (!pEntity || !pEntity->getBulletBody()) {
        return hkResult(hkResult::FAIL);
    }
    if (pEntity->getWorld() == this) {
        pEntity->setWorld(0);
    }
    PhysicsWorld_RemoveRigidBody(m_pPhysicsWorld, pEntity->getBulletBody());
    return hkResult(hkResult::SUCCESS);
}

hkResult hkWorld::addPhantom(hkPhantom* pPhantom)
{
    if (!pPhantom || !pPhantom->getBulletGhost()) {
        return hkResult(hkResult::FAIL);
    }
    pPhantom->setWorld(this);
    PhysicsWorld_AddTrigger(m_pPhysicsWorld, pPhantom->getBulletGhost());
    return hkResult(hkResult::SUCCESS);
}

hkResult hkWorld::removePhantom(hkPhantom* pPhantom)
{
    if (!pPhantom || !pPhantom->getBulletGhost()) {
        return hkResult(hkResult::FAIL);
    }
    if (pPhantom->getWorld() == this) {
        pPhantom->setWorld(0);
    }
    PhysicsWorld_RemoveTrigger(m_pPhysicsWorld, pPhantom->getBulletGhost());
    return hkResult(hkResult::SUCCESS);
}

// Constraint/action tracking-focused: game doesn't use constraints yet.
// Kept as linked list for symmetric add/remove.
namespace
{
    struct HavokListNode {
        void* pData;
        HavokListNode* pNext;
    };

    bool ListContains(HavokListNode* pHead, void* pData)
    {
        for (HavokListNode* p = pHead; p; p = p->pNext) {
            if (p->pData == pData) return true;
        }
        return false;
    }

    HavokListNode* ListAdd(HavokListNode* pHead, void* pData)
    {
        HavokListNode* pNode = new HavokListNode();
        pNode->pData = pData;
        pNode->pNext = pHead;
        return pNode;
    }

    HavokListNode* ListRemove(HavokListNode* pHead, void* pData)
    {
        HavokListNode** pp = &pHead;
        while (*pp) {
            if ((*pp)->pData == pData) {
                HavokListNode* pDead = *pp;
                *pp = pDead->pNext;
                delete pDead;
                return pHead;
            }
            pp = &(*pp)->pNext;
        }
        return pHead;
    }

    void ListFree(HavokListNode* pHead)
    {
        while (pHead) {
            HavokListNode* pNext = pHead->pNext;
            delete pHead;
            pHead = pNext;
        }
    }
}

hkResult hkWorld::addConstraint(hkConstraint* pConstraint)
{
    if (!pConstraint) return hkResult(hkResult::FAIL);
    HavokListNode*& head = *reinterpret_cast<HavokListNode**>(&m_pConstraints);
    if (!ListContains(head, pConstraint)) {
        head = ListAdd(head, pConstraint);
    }
    return hkResult(hkResult::SUCCESS);
}

hkResult hkWorld::removeConstraint(hkConstraint* pConstraint)
{
    HavokListNode*& head = *reinterpret_cast<HavokListNode**>(&m_pConstraints);
    head = ListRemove(head, pConstraint);
    return hkResult(hkResult::SUCCESS);
}

hkResult hkWorld::addAction(hkAction* pAction)
{
    if (!pAction) return hkResult(hkResult::FAIL);
    HavokListNode*& head = *reinterpret_cast<HavokListNode**>(&m_pActions);
    if (!ListContains(head, pAction)) {
        head = ListAdd(head, pAction);
    }
    return hkResult(hkResult::SUCCESS);
}

hkResult hkWorld::removeAction(hkAction* pAction)
{
    HavokListNode*& head = *reinterpret_cast<HavokListNode**>(&m_pActions);
    head = ListRemove(head, pAction);
    return hkResult(hkResult::SUCCESS);
}

void hkWorld::setCollisionFilter(hkCollisionFilter* pFilter)
{
    m_pCollisionFilter = pFilter;
}

// Bullet single-threaded → stubs no-op (binary contract preserved)
void hkWorld::markForWrite() {}
void hkWorld::unmarkForWrite() {}
void hkWorld::markForRead() {}
void hkWorld::unmarkForRead() {}

// ---------------------------------------------------------------------------
// hkPhantom / hkShapePhantom / hkCachingShapePhantom — btGhostObject
// ---------------------------------------------------------------------------
hkPhantom::hkPhantom() : m_pBulletGhost(0) {}

hkPhantom::hkPhantom(btGhostObject* pGhost)
    : m_pBulletGhost(pGhost)
{
    if (m_pBulletGhost) {
        m_pBulletGhost->setCollisionFlags(
            m_pBulletGhost->getCollisionFlags() |
            btCollisionObject::CF_NO_CONTACT_RESPONSE);
    }
}

hkPhantom::~hkPhantom()
{
    if (m_pBulletGhost) {
        btCollisionShape* pShape = m_pBulletGhost->getCollisionShape();
        delete m_pBulletGhost;
        delete pShape;   // phantom owns shape
        m_pBulletGhost = 0;
    }
}

const hkClass* hkPhantom::getClass() const
{
    static const hkClass kCls("hkPhantom", 0, 0, 0, (int)sizeof(hkPhantom));
    return &kCls;
}

void hkPhantom::getPenetratingEntityList(hkEntity** entities,
                                         int& numEntities) const
{
    numEntities = 0;
    if (!m_pBulletGhost || !entities) return;
    for (int i = 0; i < m_pBulletGhost->getNumOverlappingObjects(); ++i) {
        btCollisionObject* pObj = m_pBulletGhost->getOverlappingObject(i);
        hkEntity* pEntity = EntityRegFind(pObj);
        if (pEntity) {
            entities[numEntities++] = pEntity;
        }
    }
}

void hkPhantom::getPosition(hkVector3& position) const
{
    if (m_pBulletGhost) {
        position = ToHk(m_pBulletGhost->getWorldTransform().getOrigin());
    }
}

void hkPhantom::setPosition(const hkVector3& position)
{
    if (!m_pBulletGhost) return;
    btTransform t = m_pBulletGhost->getWorldTransform();
    t.setOrigin(ToBt(position));
    m_pBulletGhost->setWorldTransform(t);
}

hkShapePhantom::hkShapePhantom(hkShape* pShape)
    : m_pShape(pShape)
{
    btGhostObject* pGhost = new btGhostObject();
    if (pShape && pShape->getBulletShape()) {
        pGhost->setCollisionShape(pShape->getBulletShape());
    }
    m_pBulletGhost = pGhost;
    m_pBulletGhost->setCollisionFlags(
        m_pBulletGhost->getCollisionFlags() |
        btCollisionObject::CF_NO_CONTACT_RESPONSE);
}

hkShapePhantom::~hkShapePhantom() { m_pBulletGhost = 0; } // base frees

void hkShapePhantom::getAabb(hkAabb& aabb) const
{
    if (m_pShape) {
        m_pShape->getAabb(aabb);
        return;
    }
    aabb.m_min = hkVector3(0, 0, 0);
    aabb.m_max = hkVector3(0, 0, 0);
}

hkCachingShapePhantom::hkCachingShapePhantom(hkShape* pShape)
    : hkShapePhantom(pShape) {}

hkCachingShapePhantom::~hkCachingShapePhantom() {}

void hkCachingShapePhantom::getAabb(hkAabb& aabb) const
{
    hkShapePhantom::getAabb(aabb);
}

void hkCachingShapePhantom::updateCaching()
{
    // Bullet ghost overlap list manages wide-phase cache itself;
    // no extra cache needed (no-op).
}

// ---------------------------------------------------------------------------
// hkConstraint / hkAction
// ---------------------------------------------------------------------------
hkConstraint::hkConstraint(hkEntity* pEntityA, hkEntity* pEntityB)
    : m_pEntityA(pEntityA), m_pEntityB(pEntityB) {}

hkConstraint::~hkConstraint() {}

const hkClass* hkConstraint::getClass() const
{
    static const hkClass kCls("hkConstraint", 0, 0, 0,
                              (int)sizeof(hkConstraint));
    return &kCls;
}

hkAction::hkAction(hkEntity* pEntity) : m_pEntity(pEntity) {}

hkAction::~hkAction() {}

const hkClass* hkAction::getClass() const
{
    static const hkClass kCls("hkAction", 0, 0, 0, (int)sizeof(hkAction));
    return &kCls;
}

// ---------------------------------------------------------------------------
// hkCollisionFilter / hkNullCollisionFilter
// ---------------------------------------------------------------------------
hkCollisionFilter::~hkCollisionFilter() {}

const hkClass* hkCollisionFilter::getClass() const
{
    static const hkClass kCls("hkCollisionFilter", 0, 0, 0,
                              (int)sizeof(hkCollisionFilter));
    return &kCls;
}

hkBool hkCollisionFilter::isCollisionEnabled(const hkEntity* a,
                                             const hkEntity* b) const
{
    // Default: everything collides. Extendable per bitmask.
    (void)a; (void)b;
    return 1;
}

hkBool hkCollisionFilter::isCollisionEnabled(const hkEntity* e,
                                             const hkPhantom* p) const
{
    (void)e; (void)p;
    return 1;
}

hkBool hkCollisionFilter::isCollisionEnabled(const hkShape* a,
                                             const hkShape* b) const
{
    (void)a; (void)b;
    return 1;
}

hkNullCollisionFilter::~hkNullCollisionFilter() {}

hkBool hkNullCollisionFilter::isCollisionEnabled(const hkEntity* a,
                                                 const hkEntity* b) const
{
    (void)a; (void)b;
    return 1;
}
