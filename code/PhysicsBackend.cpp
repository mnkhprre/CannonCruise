// ============================================================================
// CannonCruise - Bullet Physics Backend (PhysicsBackend.cpp)
// Original path: D:\Projects\CannonCruisePC\code\PhysicsBackend.cpp
// Tersine mühendislik ile CC_ida.asm analiz edilerek yazılmıştır.
//
// Gövde karşılıkları — başlık bloğuna bkz. (sub_55E6D0 parametreleri).
// btTriangleMesh sahipliği: mesh gövdelerinde shape + mesh birlikte
// tutulur (MeshBundle) — shape mesh'i kopyalamaz.
// ============================================================================

#include "PhysicsBackend.h"

#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <BulletCollision/CollisionShapes/btBoxShape.h>
#include <BulletCollision/CollisionShapes/btSphereShape.h>
#include <BulletCollision/CollisionShapes/btConvexHullShape.h>
#include <BulletCollision/CollisionShapes/btTriangleMesh.h>
#include <BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h>
#include <BulletCollision/CollisionShapes/btCompoundShape.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include <BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.h>
#include <BulletCollision/BroadphaseCollision/btDbvtBroadphase.h>
#include <BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.h>
#include <LinearMath/btDefaultMotionState.h>
#include <LinearMath/btTransform.h>

#include <cstring>

// Header'daki opaque tip — global kapsamda tanımlı olmalı (anon ns değil)
struct PhysicsWorldHandle {
    btDefaultCollisionConfiguration *pConfig;
    btCollisionDispatcher *pDispatcher;
    btDbvtBroadphase *pBroadphase;
    btSequentialImpulseConstraintSolver *pSolver;
    btDiscreteDynamicsWorld *pWorld;
    // ghost (trigger) overlap listesi için gerekli — aksi halde
    // getNumOverlappingObjects 0 döner (Bullet sözleşmesi)
    btGhostPairCallback *pGhostCallback;
};

namespace
{
    // sub_55E6D0 varsayılanları
    const btScalar kGravityY = -9.8;    // [+0x04]
    const btScalar kFriction = 0.6;     // [+0x44]
    const btScalar kRestitution = 0.3;  // [+0x58]
    const int kSolverIterations = 20;   // [+0x80]
    // AABB ±500 ([+0x20..38]) — Bullet geniş broadphase kullanır; oyun içi
    // clamp henüz uygulanmadı (belgeli sapma)

    // statik mesh gövdeleri: shape + triangle mesh birlikte yaşar
    struct MeshBundle {
        btTriangleMesh *pMesh;
        btBvhTriangleMeshShape *pShape;
        btRigidBody *pBody;
    };

    // bağımsız mesh şekilleri (CCollisionGeometry CreateShapeFromClump):
    // mesh, shape'in ömrünü taşır
    struct ShapeBundle {
        btTriangleMesh *pMesh; // non-trimesh şekillerde null
        btCollisionShape *pShape;
    };
    ShapeBundle **g_ppShapeReg = 0;
    int g_shapeRegCount = 0;
    int g_shapeRegCap = 0;

    void RegisterShape(void *pOpaque, ShapeBundle *pBundle) {
        if (g_shapeRegCount == g_shapeRegCap) {
            const int newCap = g_shapeRegCap ? g_shapeRegCap * 2 : 64;
            ShapeBundle **ppNew = new ShapeBundle *[newCap];
            for (int i = 0; i < g_shapeRegCount; ++i) {
                ppNew[i] = g_ppShapeReg[i];
            }
            delete[] g_ppShapeReg;
            g_ppShapeReg = ppNew;
            g_shapeRegCap = newCap;
        }
        (void)pOpaque;
        g_ppShapeReg[g_shapeRegCount++] = pBundle;
    }

    ShapeBundle *FindShapeBundle(void *pOpaque) {
        for (int i = 0; i < g_shapeRegCount; ++i) {
            // ShapeBundle* döndürülen işaretçidir (void* dönüşümü)
            if (static_cast<void *>(g_ppShapeReg[i]) == pOpaque) {
                return g_ppShapeReg[i];
            }
        }
        return 0;
    }

    void UnregisterShape(void *pOpaque) {
        for (int i = 0; i < g_shapeRegCount; ++i) {
            if (static_cast<void *>(g_ppShapeReg[i]) == pOpaque) {
                g_ppShapeReg[i] = g_ppShapeReg[g_shapeRegCount - 1];
                --g_shapeRegCount;
                return;
            }
        }
    }

    // verilen desc'ten btTriangleMesh doldur (CreateTriMesh/Shape ortak yol)
    bool FillTriangleMesh(btTriangleMesh *pMesh,
                          const PhysicsTriMeshDesc *pDesc) {
        const char *pBase = reinterpret_cast<const char *>(pDesc->pVertices);
        for (RwInt32 i = 0; i + 2 < pDesc->indexCount; i += 3) {
            const RwInt32 i0 = pDesc->pIndices[i];
            const RwInt32 i1 = pDesc->pIndices[i + 1];
            const RwInt32 i2 = pDesc->pIndices[i + 2];
            const RwReal *v0 =
                reinterpret_cast<const RwReal *>(pBase + i0 * pDesc->vertexStride);
            const RwReal *v1 =
                reinterpret_cast<const RwReal *>(pBase + i1 * pDesc->vertexStride);
            const RwReal *v2 =
                reinterpret_cast<const RwReal *>(pBase + i2 * pDesc->vertexStride);
            pMesh->addTriangle(btVector3(v0[0], v0[1], v0[2]),
                               btVector3(v1[0], v1[1], v1[2]),
                               btVector3(v2[0], v2[1], v2[2]));
        }
        return pMesh->getNumTriangles() > 0;
    }

    btTransform MakeTransform(const RwReal *pPos, const RwReal *pRot3x3) {
        btTransform t;
        t.setIdentity();
        t.setOrigin(btVector3(pPos[0], pPos[1], pPos[2]));
        if (pRot3x3) {
            // sütun tabanlı 3x3 → btMatrix3x3 (satır tabanlı dolgu)
            btMatrix3x3 rot;
            for (int row = 0; row < 3; ++row) {
                for (int col = 0; col < 3; ++col) {
                    rot[row][col] = pRot3x3[col * 3 + row];
                }
            }
            t.setBasis(rot);
        }
        return t;
    }

    btRigidBody *MakeBody(btScalar mass, const btTransform &worldTrans,
                          btCollisionShape *pShape, btScalar friction,
                          btScalar restitution) {
        btVector3 localInertia(0, 0, 0);
        if (mass != 0.0f) {
            pShape->calculateLocalInertia(mass, localInertia);
        }
        btDefaultMotionState *pMotion =
            new btDefaultMotionState(worldTrans);
        btRigidBody::btRigidBodyConstructionInfo info(mass, pMotion, pShape,
                                                      localInertia);
        info.m_friction = friction;      // sub_55E6D0[+0x44]
        info.m_restitution = restitution; // sub_55E6D0[+0x58]
        btRigidBody *pBody = new btRigidBody(info);
        pBody->setActivationState(DISABLE_DEACTIVATION);
        return pBody;
    }

    // void* gövde tipi ayrımı — MeshBundle işaretçisi btRigidBody* ile aynı
    // adreste başlar; ayırmak için kayıt tablosu kullanılır.
    // Sözleşme: PhysicsBody_CreateTriMesh döndürülen işaretçi MeshBundle*,
    // diğerleri doğrudan btRigidBody*.
    struct BodyRegEntry {
        void *pOpaque;
        MeshBundle *pBundle; // statik mesh ise non-null
    };
    BodyRegEntry *g_pReg = 0;
    int g_regCount = 0;
    int g_regCap = 0;

    void RegisterBody(void *pOpaque, MeshBundle *pBundle) {
        if (g_regCount == g_regCap) {
            const int newCap = g_regCap ? g_regCap * 2 : 64;
            BodyRegEntry *pNew = new BodyRegEntry[newCap];
            for (int i = 0; i < g_regCount; ++i) {
                pNew[i] = g_pReg[i];
            }
            delete[] g_pReg;
            g_pReg = pNew;
            g_regCap = newCap;
        }
        g_pReg[g_regCount].pOpaque = pOpaque;
        g_pReg[g_regCount].pBundle = pBundle;
        ++g_regCount;
    }

    MeshBundle *FindBundle(void *pOpaque) {
        for (int i = 0; i < g_regCount; ++i) {
            if (g_pReg[i].pOpaque == pOpaque) {
                return g_pReg[i].pBundle;
            }
        }
        return 0;
    }

    void UnregisterBody(void *pOpaque) {
        for (int i = 0; i < g_regCount; ++i) {
            if (g_pReg[i].pOpaque == pOpaque) {
                g_pReg[i] = g_pReg[g_regCount - 1];
                --g_regCount;
                return;
            }
        }
    }

    btRigidBody *AsBody(void *pOpaque) {
        MeshBundle *pBundle = FindBundle(pOpaque);
        if (pBundle) {
            return pBundle->pBody;
        }
        return static_cast<btRigidBody *>(pOpaque);
    }

    const btRigidBody *AsBodyConst(const void *pOpaque) {
        return AsBody(const_cast<void *>(pOpaque));
    }
} // namespace

// ---------------------------------------------------------------------------
// dünya yaşam döngüsü
// ---------------------------------------------------------------------------
PhysicsWorldHandle *PhysicsWorld_Create(void) {
    PhysicsWorldHandle *p = new PhysicsWorldHandle();
    p->pConfig = new btDefaultCollisionConfiguration();
    p->pDispatcher = new btCollisionDispatcher(p->pConfig);
    p->pBroadphase = new btDbvtBroadphase();
    p->pSolver = new btSequentialImpulseConstraintSolver();
    p->pWorld = new btDiscreteDynamicsWorld(
        p->pDispatcher, p->pBroadphase, p->pSolver, p->pConfig);
    p->pWorld->setGravity(btVector3(0, kGravityY, 0)); // sub_55E6D0[+0x04]
    p->pWorld->getSolverInfo().m_numIterations = kSolverIterations;
    // trigger (hkpPhantom → btGhostObject) overlap kaydı
    p->pGhostCallback = new btGhostPairCallback();
    p->pBroadphase->getOverlappingPairCache()->setInternalGhostPairCallback(
        p->pGhostCallback);
    return p;
}

void PhysicsWorld_Destroy(PhysicsWorldHandle *pWorld) {
    if (!pWorld) {
        return;
    }
    // gövde temizliği çağırının sorumluluğu (CPhysicsWorld dtor sözleşmesi)
    delete pWorld->pWorld;
    delete pWorld->pGhostCallback;
    delete pWorld->pSolver;
    delete pWorld->pBroadphase;
    delete pWorld->pDispatcher;
    delete pWorld->pConfig;
    delete pWorld;
}

void PhysicsWorld_Step(PhysicsWorldHandle *pWorld, RwReal deltaTime) {
    if (!pWorld || !pWorld->pWorld) {
        return;
    }
    // sabit adım: maxSubSteps=1 + fixedTimeStep=deltaTime → her çağrıda
    // tam bir deltaTime ilerler (hkpWorld::stepDeltaTime taklidi)
    pWorld->pWorld->stepSimulation(deltaTime, 1, deltaTime);
}

void PhysicsWorld_SetGravity(PhysicsWorldHandle *pWorld, RwReal x, RwReal y,
                             RwReal z) {
    if (pWorld && pWorld->pWorld) {
        pWorld->pWorld->setGravity(btVector3(x, y, z));
    }
}

void PhysicsWorld_GetGravity(const PhysicsWorldHandle *pWorld, RwReal *pX,
                             RwReal *pY, RwReal *pZ) {
    if (!pWorld || !pWorld->pWorld) {
        return;
    }
    const btVector3 g = pWorld->pWorld->getGravity();
    if (pX) *pX = (RwReal)g.x();
    if (pY) *pY = (RwReal)g.y();
    if (pZ) *pZ = (RwReal)g.z();
}

// ---------------------------------------------------------------------------
// gövdeler
// ---------------------------------------------------------------------------
void *PhysicsBody_CreateBox(const PhysicsBodyBoxDesc *pDesc) {
    if (!pDesc) {
        return 0;
    }
    btBoxShape *pShape = new btBoxShape(btVector3(
        pDesc->halfExtents[0], pDesc->halfExtents[1], pDesc->halfExtents[2]));
    btTransform t = MakeTransform(pDesc->pos, pDesc->rot3x3);
    const btScalar friction =
        (pDesc->friction >= 0.0f) ? pDesc->friction : kFriction;
    const btScalar restitution =
        (pDesc->restitution >= 0.0f) ? pDesc->restitution : kRestitution;
    return MakeBody(pDesc->mass, t, pShape, friction, restitution);
}

void *PhysicsBody_CreateTriMesh(const PhysicsTriMeshDesc *pDesc) {
    if (!pDesc || !pDesc->pVertices || !pDesc->pIndices ||
        pDesc->indexCount < 3) {
        return 0;
    }
    MeshBundle *pBundle = new MeshBundle();
    pBundle->pMesh = new btTriangleMesh();
    if (!FillTriangleMesh(pBundle->pMesh, pDesc)) {
        delete pBundle->pMesh;
        delete pBundle;
        return 0;
    }
    pBundle->pShape =
        new btBvhTriangleMeshShape(pBundle->pMesh, /*useQuantized=*/true);
    btTransform t = MakeTransform(pDesc->pos, 0);
    pBundle->pBody = MakeBody(0.0f, t, pBundle->pShape, kFriction,
                              kRestitution); // statik mesh
    RegisterBody(pBundle, pBundle); // void* → MeshBundle kaydı
    return pBundle;
}

// ---------------------------------------------------------------------------
// bağımsız çarpışma şekilleri — CCollisionGeometry karşılığı
// ---------------------------------------------------------------------------
void *PhysicsShape_CreateBox(const RwReal halfExtents[3]) {
    if (!halfExtents) {
        return 0;
    }
    ShapeBundle *pBundle = new ShapeBundle();
    pBundle->pMesh = 0;
    pBundle->pShape = new btBoxShape(btVector3(
        halfExtents[0], halfExtents[1], halfExtents[2]));
    RegisterShape(pBundle, pBundle);
    return pBundle;
}

void *PhysicsShape_CreateSphere(RwReal radius) {
    if (radius <= 0.0f) {
        return 0;
    }
    ShapeBundle *pBundle = new ShapeBundle();
    pBundle->pMesh = 0;
    pBundle->pShape = new btSphereShape(radius);
    RegisterShape(pBundle, pBundle);
    return pBundle;
}

void *PhysicsShape_CreateConvexHull(const RwReal *pVertices,
                                    RwInt32 vertexCount, RwInt32 vertexStride) {
    if (!pVertices || vertexCount < 3 || vertexStride < 3 * (int)sizeof(RwReal)) {
        return 0;
    }
    ShapeBundle *pBundle = new ShapeBundle();
    pBundle->pMesh = 0;
    btConvexHullShape *pShape = new btConvexHullShape();
    const char *pBase = reinterpret_cast<const char *>(pVertices);
    for (RwInt32 i = 0; i < vertexCount; ++i) {
        const RwReal *v =
            reinterpret_cast<const RwReal *>(pBase + i * vertexStride);
        pShape->addPoint(btVector3(v[0], v[1], v[2]), /*recalcLocalAabb=*/false);
    }
    pShape->recalcLocalAabb();
    pBundle->pShape = pShape;
    RegisterShape(pBundle, pBundle);
    return pBundle;
}

void *PhysicsShape_CreateTriMesh(const PhysicsTriMeshDesc *pDesc) {
    if (!pDesc || !pDesc->pVertices || !pDesc->pIndices ||
        pDesc->indexCount < 3) {
        return 0;
    }
    ShapeBundle *pBundle = new ShapeBundle();
    pBundle->pMesh = new btTriangleMesh();
    if (!FillTriangleMesh(pBundle->pMesh, pDesc)) {
        delete pBundle->pMesh;
        delete pBundle;
        return 0;
    }
    pBundle->pShape =
        new btBvhTriangleMeshShape(pBundle->pMesh, /*useQuantized=*/true);
    RegisterShape(pBundle, pBundle);
    return pBundle;
}

void PhysicsShape_Destroy(void *pShape) {
    if (!pShape) {
        return;
    }
    ShapeBundle *pBundle = FindShapeBundle(pShape);
    if (pBundle) {
        delete pBundle->pShape;
        delete pBundle->pMesh;
        delete pBundle;
        UnregisterShape(pShape);
        return;
    }
    // kutu/spere kayıtsız da olabilir (kayıt her Create'de yapılır;
    // defensive) — doğrudan shape olarak silme: bilinmeyen işaretçi → yok say
}

void *PhysicsBody_CreateWithShape(void *pShape, RwReal mass,
                                  const RwReal pos[3],
                                  const RwReal *pRot3x3ColMajor) {
    if (!pShape || !pos) {
        return 0;
    }
    btCollisionShape *pBtShape;
    ShapeBundle *pBundle = FindShapeBundle(pShape);
    if (pBundle) {
        pBtShape = pBundle->pShape;
    } else {
        pBtShape = static_cast<btCollisionShape *>(pShape);
    }
    btTransform t = MakeTransform(pos, pRot3x3ColMajor);
    btRigidBody *pBody =
        MakeBody(mass, t, pBtShape, kFriction, kRestitution);
    // userIndex=1 → shape dışarıdan geldi; Destroy sahiplenmez
    pBody->setUserIndex(1);
    return pBody;
}

void PhysicsBody_Destroy(void *pBody) {
    if (!pBody) {
        return;
    }
    MeshBundle *pBundle = FindBundle(pBody);
    if (pBundle) {
        btMotionState *pMS = pBundle->pBody->getMotionState();
        delete pBundle->pBody;
        delete pMS;
        delete pBundle->pShape;
        delete pBundle->pMesh;
        delete pBundle;
        UnregisterBody(pBody);
        return;
    }
    btRigidBody *pRb = static_cast<btRigidBody *>(pBody);
    btMotionState *pMS = pRb->getMotionState();
    btCollisionShape *pShape = pRb->getCollisionShape();
    // userIndex=1 → shape dışarıdan gelmiştir (CreateWithShape); sahiplenme
    const bool ownsShape = (pRb->getUserIndex() != 1);
    delete pRb;
    delete pMS;
    if (pShape && ownsShape) {
        delete pShape;
    }
}

void *PhysicsTrigger_CreateBox(const RwReal halfExtents[3],
                               const RwReal pos[3]) {
    btBoxShape *pShape = new btBoxShape(btVector3(
        halfExtents[0], halfExtents[1], halfExtents[2]));
    btGhostObject *pGhost = new btGhostObject();
    pGhost->setCollisionShape(pShape);
    pGhost->setCollisionFlags(pGhost->getCollisionFlags() |
                              btCollisionObject::CF_NO_CONTACT_RESPONSE);
    pGhost->setWorldTransform(
        MakeTransform(pos, 0));
    return pGhost;
}

void PhysicsTrigger_Destroy(void *pTrigger) {
    if (!pTrigger) {
        return;
    }
    btGhostObject *pGhost = static_cast<btGhostObject *>(pTrigger);
    btCollisionShape *pShape = pGhost->getCollisionShape();
    delete pGhost;
    delete pShape;
}

// ---------------------------------------------------------------------------
// dünya kaydı
// ---------------------------------------------------------------------------
void PhysicsWorld_AddRigidBody(PhysicsWorldHandle *pWorld, void *pBody) {
    if (pWorld && pWorld->pWorld && pBody) {
        pWorld->pWorld->addRigidBody(AsBody(pBody));
    }
}

void PhysicsWorld_RemoveRigidBody(PhysicsWorldHandle *pWorld, void *pBody) {
    if (pWorld && pWorld->pWorld && pBody) {
        pWorld->pWorld->removeRigidBody(AsBody(pBody));
    }
}

void PhysicsWorld_AddTrigger(PhysicsWorldHandle *pWorld, void *pTrigger) {
    if (pWorld && pWorld->pWorld && pTrigger) {
        pWorld->pWorld->addCollisionObject(
            static_cast<btGhostObject *>(pTrigger),
            btBroadphaseProxy::SensorTrigger,
            btBroadphaseProxy::AllFilter & ~btBroadphaseProxy::SensorTrigger);
    }
}

void PhysicsWorld_RemoveTrigger(PhysicsWorldHandle *pWorld, void *pTrigger) {
    if (pWorld && pWorld->pWorld && pTrigger) {
        pWorld->pWorld->removeCollisionObject(
            static_cast<btCollisionObject *>(pTrigger));
    }
}

// ---------------------------------------------------------------------------
// durum erişimi
// ---------------------------------------------------------------------------
void PhysicsBody_GetPosition(const void *pBody, RwReal *pX, RwReal *pY,
                             RwReal *pZ) {
    if (!pBody) {
        return;
    }
    const btRigidBody *pRb = AsBodyConst(pBody);
    const btVector3 o = pRb->getWorldTransform().getOrigin();
    if (pX) *pX = (RwReal)o.x();
    if (pY) *pY = (RwReal)o.y();
    if (pZ) *pZ = (RwReal)o.z();
}

void PhysicsBody_GetRotation(const void *pBody, RwReal *pRot3x3ColMajor) {
    if (!pBody || !pRot3x3ColMajor) {
        return;
    }
    const btRigidBody *pRb = AsBodyConst(pBody);
    const btMatrix3x3 &b = pRb->getWorldTransform().getBasis();
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col) {
            pRot3x3ColMajor[col * 3 + row] = (RwReal)b[row][col];
        }
    }
}

void PhysicsBody_SetPosition(void *pBody, RwReal x, RwReal y, RwReal z) {
    btRigidBody *pRb = AsBody(pBody);
    if (!pRb) {
        return;
    }
    btTransform t = pRb->getWorldTransform();
    t.setOrigin(btVector3(x, y, z));
    pRb->setWorldTransform(t);
    if (pRb->getMotionState()) {
        pRb->getMotionState()->setWorldTransform(t);
    }
}

void PhysicsBody_SetKinematic(void *pBody, RwBool bKinematic) {
    btRigidBody *pRb = AsBody(pBody);
    if (!pRb) {
        return;
    }
    if (bKinematic) {
        pRb->setCollisionFlags(pRb->getCollisionFlags() |
                               btCollisionObject::CF_KINEMATIC_OBJECT);
        pRb->setActivationState(DISABLE_DEACTIVATION);
    } else {
        pRb->setCollisionFlags(pRb->getCollisionFlags() &
                               ~btCollisionObject::CF_KINEMATIC_OBJECT);
    }
}

void PhysicsBody_ApplyImpulse(void *pBody, RwReal x, RwReal y, RwReal z,
                              RwReal relX, RwReal relY, RwReal relZ) {
    btRigidBody *pRb = AsBody(pBody);
    if (pRb) {
        pRb->applyImpulse(btVector3(x, y, z),
                          btVector3(relX, relY, relZ));
    }
}

void PhysicsBody_ApplyCentralImpulse(void *pBody, RwReal x, RwReal y,
                                     RwReal z) {
    btRigidBody *pRb = AsBody(pBody);
    if (pRb) {
        pRb->applyCentralImpulse(btVector3(x, y, z));
    }
}

void PhysicsBody_GetLinearVelocity(const void *pBody, RwReal *pX, RwReal *pY,
                                   RwReal *pZ) {
    const btRigidBody *pRb = AsBodyConst(pBody);
    const btVector3 v = pRb->getLinearVelocity();
    if (pX) *pX = (RwReal)v.x();
    if (pY) *pY = (RwReal)v.y();
    if (pZ) *pZ = (RwReal)v.z();
}

void PhysicsBody_SetLinearVelocity(void *pBody, RwReal x, RwReal y, RwReal z) {
    btRigidBody *pRb = AsBody(pBody);
    if (pRb) {
        pRb->setLinearVelocity(btVector3(x, y, z));
    }
}

RwInt32 PhysicsTrigger_GetOverlaps(const void *pTrigger, void **ppBodies,
                                   RwInt32 maxCount) {
    if (!pTrigger || !ppBodies || maxCount <= 0) {
        return 0;
    }
    const btGhostObject *pGhost =
        static_cast<const btGhostObject *>(pTrigger);
    RwInt32 n = 0;
    for (int i = 0;
         i < pGhost->getNumOverlappingObjects() && n < maxCount; ++i) {
        ppBodies[n++] = const_cast<btCollisionObject *>(
            pGhost->getOverlappingObject(i));
    }
    return n;
}

// ---------------------------------------------------------------------------
// opaque köprü
// ---------------------------------------------------------------------------
PhysicsWorldHandle *PhysicsWorld_FromOpaque(void *pOpaque) {
    return static_cast<PhysicsWorldHandle *>(pOpaque);
}

void *PhysicsWorld_ToOpaque(PhysicsWorldHandle *pWorld) {
    return pWorld;
}
