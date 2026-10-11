// ============================================================================
// CannonCruise - Bullet Physics Backend (PhysicsBackend.h)
// Original path: D:\Projects\CannonCruisePC\code\PhysicsBackend.h
// Tersine mühendislik ile CC_ida.asm analiz edilerek yazılmıştır.
//
// Havok yerine taşınabilir fizik motoru (Bullet). CPhysicsWorld void*
// sarmalayıcı API'sini korur; bu modül backend'i sağlar.
//
// Binary Fonksiyonları (parametre kaynakları):
//   sub_55E6D0 — oyun fizik param varsayılanları:
//     [+0x04] gravity = -9.8      [+0x20..28] AABB min = -500
//     [+0x30..38] AABB max = 500  [+0x44] friction = 0.6
//     [+0x48] = 1.0               [+0x4C] = 4
//     [+0x58] restitution = 0.3   [+0x80] solver iter = 20
//   sub_4B8710 — sabit fizik adımı 1/60 (push 3C888889h — CSoldier tick)
//   FUN_0043a040 — CPhysicsWorld::Update → hkpWorld::stepDeltaTime (1/60)
//
// SAPMALAR (belgeli): Bullet sequential-impulse solver ≠ Havok 2004 solver —
// birebir his garanti edilmez; kritik değerler binary'den reverse edilip
// backend parametrelerine girilir. AABB ±500 broadphase sınırı Havok
// cinfo'yu taklit eder (Bullet geniş aabb kullanır; oyun içi clamp).
// ============================================================================
#pragma once

#include "rwcore.h"

/**
 * @brief Fizik dünyası — opaque btDiscreteDynamicsWorld paketi
 * (collision config + dispatcher + dbvt broadphase + impulse solver).
 */
struct PhysicsWorldHandle;

/**
 * @brief Dinamik gövde oluşturma tanımı — kutu yarıkutu (yarı eksenler),
 * kutup: sol-üst-arka (RwMatrix sütun tabanlı pos).
 */
struct PhysicsBodyBoxDesc {
    RwReal halfExtents[3]; ///< yarıkutu yarı eksenleri
    RwReal mass;           ///< 0 → statik
    RwReal friction;       ///< varsayılan 0.6 (sub_55E6D0[+0x44])
    RwReal restitution;    ///< varsayılan 0.3 (sub_55E6D0[+0x58])
    RwReal pos[3];         ///< dünya konumu
    RwReal rot3x3[9];      ///< sütun tabanlı 3x3 döndürme (boş → birim)
};

/**
 * @brief Statik üçgen mesh tanımı — Clump'tan üretilir (CollisionGeometry).
 * vertexStride bayt cinsinden; tipik 12 (3×RwReal) veya vertex struct boyu.
 */
struct PhysicsTriMeshDesc {
    const RwReal *pVertices; ///< vertex akışı (stride'lı)
    RwInt32 vertexCount;
    RwInt32 vertexStride; ///< bayt
    const RwInt32 *pIndices; ///< 3'lü index listesi
    RwInt32 indexCount;
    RwReal pos[3]; ///< mesh gövde konumu
};

// PhysicsWorld_Create — btDiscreteDynamicsWorld kurulumu. Gravity (0,-9.8,0)
// (sub_55E6D0[+0x04]); solver iterasyon 20 (sub_55E6D0[+0x80]).
PhysicsWorldHandle *PhysicsWorld_Create(void);
void PhysicsWorld_Destroy(PhysicsWorldHandle *pWorld);

// PhysicsWorld_Step — sabit adım simülasyon (binary: 1/60 sabit; caller
// sağlar). Bullet stepSimulation(dt, maxSubSteps=1, dt) — tek sabit adım.
void PhysicsWorld_Step(PhysicsWorldHandle *pWorld, RwReal deltaTime);

void PhysicsWorld_SetGravity(PhysicsWorldHandle *pWorld, RwReal x, RwReal y,
                             RwReal z);
void PhysicsWorld_GetGravity(const PhysicsWorldHandle *pWorld, RwReal *pX,
                             RwReal *pY, RwReal *pZ);

// Gövde yaşam döngüsü — dönen void* doğrudan CPhysicsWorld
// Add/RemoveRigidBody ile uyumludur (btRigidBody*).
void *PhysicsBody_CreateBox(const PhysicsBodyBoxDesc *pDesc);
// Statik üçgen mesh — btBvhTriangleMeshShape + kinematik olmayan gövde
// (mass 0). btTriangleMesh kopyayı gövdeyle birlikte tutar.
void *PhysicsBody_CreateTriMesh(const PhysicsTriMeshDesc *pDesc);
void PhysicsBody_Destroy(void *pBody);

// ---------------------------------------------------------------------------
// Bağımsız çarpışma şekilleri — CCollisionGeometry API karşılığı.
// Sahiplik sözleşmesi:
//   - PhysicsShape_Create* döndürülen şekil çağırının malıdır; body'ye
//     PhysicsBody_CreateWithShape ile verildiğinde body SHAPE'İ SAHİPLENMEZ;
//     body Destroy edildikten sonra PhysicsShape_Destroy çağrılmalıdır.
//   - PhysicsBody_CreateBox/CreateTriMesh gövdeleri kendi shape'lerini
//     sahiplenir (Destroy otomatik temizler).
// ---------------------------------------------------------------------------
void *PhysicsShape_CreateBox(const RwReal halfExtents[3]);
void *PhysicsShape_CreateSphere(RwReal radius);
// konveks hull — dinamik gövdeler için (bConvex=TRUE yolu); vertex akışı
// stride'lı kopyalanır (shape kendi kopyasını tutar)
void *PhysicsShape_CreateConvexHull(const RwReal *pVertices,
                                    RwInt32 vertexCount, RwInt32 vertexStride);
// üçgen mesh shape — mesh verisi shapeBundle içinde shape ile birlikte tutulur
void *PhysicsShape_CreateTriMesh(const PhysicsTriMeshDesc *pDesc);
void PhysicsShape_Destroy(void *pShape);
// hazır şekil ile gövde (mass 0 → statik; shape sahipliği caller'da kalır)
void *PhysicsBody_CreateWithShape(void *pShape, RwReal mass,
                                  const RwReal pos[3], const RwReal *pRot3x3ColMajor);

// Trigger (hkpPhantom karşılığı) — ghost obj AABB; overlap sorgusu
// PhysicsTrigger_GetOverlaps ile. Kütlesiz, çarpışma yanıtsız.
void *PhysicsTrigger_CreateBox(const RwReal halfExtents[3],
                               const RwReal pos[3]);
void PhysicsTrigger_Destroy(void *pTrigger);

// Dünya kaydı (CPhysicsWorld Add/Remove altındaki asıl çağrı)
void PhysicsWorld_AddRigidBody(PhysicsWorldHandle *pWorld, void *pBody);
void PhysicsWorld_RemoveRigidBody(PhysicsWorldHandle *pWorld, void *pBody);
void PhysicsWorld_AddTrigger(PhysicsWorldHandle *pWorld, void *pTrigger);
void PhysicsWorld_RemoveTrigger(PhysicsWorldHandle *pWorld, void *pTrigger);

// Durum erişimi — RW frame senkronu için (gövde → kare matrisi).
void PhysicsBody_GetPosition(const void *pBody, RwReal *pX, RwReal *pY,
                             RwReal *pZ);
void PhysicsBody_GetRotation(const void *pBody, RwReal *pRot3x3ColMajor);
void PhysicsBody_SetPosition(void *pBody, RwReal x, RwReal y, RwReal z);
void PhysicsBody_SetKinematic(void *pBody, RwBool bKinematic);
void PhysicsBody_ApplyImpulse(void *pBody, RwReal x, RwReal y, RwReal z,
                              RwReal relX, RwReal relY, RwReal relZ);
void PhysicsBody_ApplyCentralImpulse(void *pBody, RwReal x, RwReal y,
                                     RwReal z);
void PhysicsBody_GetLinearVelocity(const void *pBody, RwReal *pX, RwReal *pY,
                                   RwReal *pZ);
void PhysicsBody_SetLinearVelocity(void *pBody, RwReal x, RwReal y, RwReal z);

// Trigger overlap — ghost obj içindeki aktif gövdeler (en fazla maxCount;
// dönüş: yazılan adet). CPhysicsWorldBehaviour tetikleme akışına kaynak.
RwInt32 PhysicsTrigger_GetOverlaps(const void *pTrigger, void **ppBodies,
                                   RwInt32 maxCount);

// Test erişimi — backend dünya (CPhysicsWorld m_pHavokWorld içeriği).
PhysicsWorldHandle *PhysicsWorld_FromOpaque(void *pOpaque);
void *PhysicsWorld_ToOpaque(PhysicsWorldHandle *pWorld);
