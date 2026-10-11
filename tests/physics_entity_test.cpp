// ============================================================================
// CannonCruise - Physics Entity + Trigger Test (physics_entity_test.cpp)
// Doğrulanan bağlantılar (Faz 4.6 — entity akışı):
//   CPhysicsEntity::Set/GetPosition, Set/GetLinearVelocity → PhysicsBackend
//   CPhysicsEntity::AddToWorld/RemoveFromWorld → CPhysicsWorld singleton
//     → backend dünya kaydı
//   CCollisionTrigger::InitializeWithSphere → btGhostObject phantom +
//     AddPhantom; PollOverlaps farkları OnEntityEnter/Leave'a akar.
// ============================================================================

#include "PhysicsBackend.h"
#include "PhysicsEntity.h"
#include "PhysicsWorld.h"
#include "CollisionTrigger.h"

#include <cmath>
#include <cstdio>

static int g_checks = 0;
static int g_failures = 0;

static void Check(bool cond, const char *pExpr, int line) {
    ++g_checks;
    if (!cond) {
        ++g_failures;
        std::printf("FAIL line %d: %s\n", line, pExpr);
    }
}

#define CHECK(expr) Check((expr), #expr, __LINE__)

static bool Near(float a, float b, float eps) {
    return std::fabs(a - b) <= eps;
}

static void *MakeBoxBody(float x, float y, float z, float mass) {
    PhysicsBodyBoxDesc desc;
    desc.halfExtents[0] = desc.halfExtents[1] = desc.halfExtents[2] = 0.5f;
    desc.mass = mass;
    desc.friction = -1.0f;
    desc.restitution = -1.0f;
    desc.pos[0] = x;
    desc.pos[1] = y;
    desc.pos[2] = z;
    for (int i = 0; i < 9; ++i) {
        desc.rot3x3[i] = 0.0f;
    }
    desc.rot3x3[0] = desc.rot3x3[4] = desc.rot3x3[8] = 1.0f;
    return PhysicsBody_CreateBox(&desc);
}

class CCountingTrigger : public CCollisionTrigger {
public:
    CCountingTrigger() : enters(0), leaves(0), lastEnter(0) {}
    virtual void OnEntityEnter(void *pEntity) {
        ++enters;
        lastEnter = pEntity;
    }
    virtual void OnEntityLeave(void *pEntity) {
        ++leaves;
        (void)pEntity;
    }
    int enters;
    int leaves;
    void *lastEnter;
};

static void TestEntityLifecycle() {
    CPhysicsWorld world; // singleton (CSingleton)

    CPhysicsEntity entity;
    void *pBody = MakeBoxBody(1.0f, 2.0f, 3.0f, 1.0f);
    CHECK(pBody != 0);
    entity.SetRigidBody(pBody);

    entity.AddToWorld();
    CHECK(world.RigidBodyExistsInWorld(pBody));

    // pozisyon erişimi — backend PhysicsBody_Set/GetPosition
    RwV3d pos = {4.0f, 5.0f, 6.0f};
    entity.SetPosition(&pos);
    RwV3d out = {0, 0, 0};
    entity.GetPosition(&out);
    CHECK(Near(out.x, 4.0f, 1e-5f) && Near(out.y, 5.0f, 1e-5f) &&
          Near(out.z, 6.0f, 1e-5f));

    // hız erişimi
    RwV3d vel = {7.0f, 8.0f, 9.0f};
    entity.SetLinearVelocity(&vel);
    RwV3d vout = {0, 0, 0};
    entity.GetLinearVelocity(&vout);
    CHECK(Near(vout.x, 7.0f, 1e-4f) && Near(vout.y, 8.0f, 1e-4f) &&
          Near(vout.z, 9.0f, 1e-4f));

    // simülasyon adımı — yerçekimi vy'yi düşürür; vx sürtünmesiz kalır
    world.Update(1.0f / 60.0f);
    entity.GetLinearVelocity(&vout);
    CHECK(Near(vout.x, 7.0f, 1e-4f));
    CHECK(vout.y < 8.0f); // gravity etkisi

    entity.RemoveFromWorld();
    CHECK(!world.RigidBodyExistsInWorld(pBody));

    entity.SetRigidBody(0); // dtor RemoveFromWorld — null safe
    PhysicsBody_Destroy(pBody);
    // world dtor: vektörler boş olmalı (assert)
}

static void TestTriggerEnterLeave() {
    CPhysicsWorld world;

    CCountingTrigger trigger;
    RwV3d center = {0.0f, 10.0f, 0.0f};
    trigger.InitializeWithSphere(&center, 3.0f); // yarıçap 3 AABB

    void *pBody = MakeBoxBody(0.0f, 10.0f, 0.0f, 1.0f); // tetik içinde
    world.AddRigidBody(pBody);

    world.Update(1.0f / 60.0f); // broadphase/overlap güncelle
    trigger.PollOverlaps();
    CHECK(trigger.enters == 1);
    CHECK(trigger.lastEnter == pBody);

    trigger.PollOverlaps(); // ikinci tur — yeniden giriş yok
    CHECK(trigger.enters == 1);

    // gövdeyi tetikten çıkar (teleport) → leave
    PhysicsBody_SetPosition(pBody, 100.0f, 10.0f, 100.0f);
    world.Update(1.0f / 60.0f);
    trigger.PollOverlaps();
    CHECK(trigger.leaves == 1);

    world.RemoveRigidBody(pBody);
    PhysicsBody_Destroy(pBody);
    // trigger dtor: phantom'ı dünyadan çıkarır + yok eder
}

static void TestBehaviourSingletonReuse() {
    // startup zinciri: CPhysicsWorld::Create() singleton'ı yaratır
    // (binary sub_41A680: alloc → ctor → dword_635AF4)
    CPhysicsWorld *pWorld = CPhysicsWorld::Create();
    CHECK(pWorld != 0);
    CHECK(CPhysicsWorld::GetSingletonPtr() == pWorld);

    // Create() tekrar çağrısı ikinci dünya YARATMAZ (mevcut döner)
    CHECK(CPhysicsWorld::Create() == pWorld);

    // behaviour singleton'ı KULLANIR (binary sub_43B860 — container-only ctor)
    CPhysicsWorldBehaviour *pBeh = new CPhysicsWorldBehaviour();
    CHECK(CPhysicsWorld::GetSingletonPtr() == pWorld);
    CHECK(CPhysicsWorld::GetSingletonPtr()->RigidBodyExistsInWorld(0) ==
          FALSE);

    // dtor: sahip değil → dünya yaşar
    delete pBeh;
    CHECK(CPhysicsWorld::GetSingletonPtr() == pWorld);

    delete pWorld; // singleton temizliği (CSingleton dtor)
    CHECK(CPhysicsWorld::GetSingletonPtr() == 0);
}

int main() {
    TestEntityLifecycle();
    TestTriggerEnterLeave();
    TestBehaviourSingletonReuse();

    std::printf("physics_entity_test: %d checks, %d failures\n", g_checks,
                g_failures);
    return g_failures == 0 ? 0 : 1;
}
