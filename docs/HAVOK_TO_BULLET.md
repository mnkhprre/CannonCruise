# Havok → Bullet Geçiş Notları (Faz 4.6)

CannonCruisePC tersine mühendislik projesi — Havok fizik motorunun yerine
taşınabilir **Bullet Physics 3.25** konuldu. Bu doküman, **neden**, **neyi,
nasıl** değiştirdiğimizi ve binary'den reverse edilen parametrelerin
kaynaklarını tek yerde toplar.

İlgili kod:

| Dosya | Rol |
|---|---|
| `code/PhysicsBackend.h/.cpp` | Bullet sarmalayıcı (yeni — backend'in kendisi) |
| `code/PhysicsWorld.cpp/.h` | `CPhysicsWorld`/`CPhysicsWorldBehaviour` — backend'e bağlı sarmalayıcı API (eski adlar korundu) |
| `code/CollisionGeometry.cpp/.h` | Clump → Bullet şekil çevirici (`FUN_0046f040`) |
| `code/PhysicsEntity.cpp`, `CollisionTrigger.*` | ince sarmalayıcılar (bağlantı bekliyor) |
| `code/HavokMemoryManager.cpp`, `code/framework/startup/havokstart.cpp` | oyunun Havok yaşam döngüsü portu — **korundu** (HavokInit çağrı yolları oyun tarafında; arena artık standart malloc) |
| `tests/physics_test.cpp` | parametre doğrulama testi |

---

## 1. Neden Bullet?

- Orijinal motor **Havok Physics 2004/2005** (kod adı "Havok Animation +
  Physics", MSVC4.2b statik kütüphane). SDK kapalı/erişilemez; repoda
  yalnızca oyunun kendi soyutlama katmanı (`hkpWorld` vb. için `void*`
  sarmalayıcılar) ve link stub'ları (`rwstubs.cpp` benzeri) var.
- Alternatifler karşılaştırıldı:
  - **Jolt** — modern ve hızlı, ama C++17 şart; projenin C++14 standardıyla
    uyuşmuyor.
  - **PhysX** — lisans/toplama maliyeti ve kurulum ağırlığı.
  - **Bullet 3.25** — MIT, C++14 uyumlu, D3D9/OpenGL çağında yaygın
    (2004-2010 oyun port'ları için en yakın ekosistem), hafif
    (yalnız LinearMath/Collision/Dynamics derlenir).

## 2. Binary'den reverse edilen parametreler ("en yakın his")

Kullanıcı direktifi: *önce binary'nin gerçek fizik ayarları bulunacak,
sonra motor o değerlerle bağlanacak.* `hkp*`/`hka*` sembolü binary'de yok
(istatik link, PDB yok) — değerler içerik analiziyle bulundu.

### 2.1 Oyun fizik ayar struct'ı — `sub_55E6D0`

Havok memory callback kaydı `sub_55E630` bu fonksiyonu dword_660D58'ye
kaydeder; `sub_55E680` dword_660920 (Havok arena context — havokstart.cpp
ile aynı) üzerinden 0x260 baytlık struct tahsis eder. `sub_55E6D0` struct'ı
şöyle doldurur:

| Offset | Binary değeri | Anlamı | Bullet karşılığı |
|---|---|---|---|
| +0x04 | `0C11CCCCDh` = **-9.8** | gravity.y | `world->setGravity(0,-9.8,0)` |
| +0x14 | 0x400 | kapasite? | — (kullanılmadı) |
| +0x20..28 | `0C3FA0000h` = **-500** | broadphase AABB min | belgeli boşluk (aş.3) |
| +0x30..38 | `43FA0000h` = **500** | broadphase AABB max | belgeli boşluk (aş.3) |
| +0x44 | `3F19999Ah` = **0.6** | friction | `btRigidBodyConstructionInfo.m_friction` |
| +0x48 | 1.0 | bilinmiyor (damping?) | — |
| +0x4C | 4 | bilinmiyor | — |
| +0x58 | `3E99999Ah` = **0.3** | restitution | `m_restitution` |
| +0x5C | `3A83126Fh` ≈ 0.0163 | ~1/61 — deactivation zamanı olabilir | — |
| +0x6C/+0x88 | 0.2 | eşik (deactivation?) | — |
| +0x80 | 0x14 = **20** | solver iterasyon | `solverInfo.m_numIterations` |
| +0x8C | 10.0 | hız sınırı olabilir | — |

### 2.2 Sabit zaman adımı — 1/60

- `CPhysicsWorld::Update` (`FUN_0043a040`) binary'de
  `hkpWorld::stepDeltaTime` çağırır; oyun kodunun her yerinde **1/60 s**
  sabiti geçer (`PhysicsWorld.h` sözleşmesi, `HandleEvents`).
- Bağımsız kanıt: `CSoldier::HandleEvents` (`sub_4B8710+7E`)
  `push 3C888889h` (=1/60) ile fizik çağrısı yapar.
- Backend: `stepSimulation(dt, /*maxSubSteps=*/1, /*fixedTimeStep=*/dt)`
  — her çağrıda tam bir sabit adım (hkpWorld::stepDeltaTime taklidi).

### 2.3 Dünya oluşturma zinciri

- `CPhysicsWorldBehaviour` ctor (`sub_43B860` → RWS sınıf kaydı
  `sub_43B9A0`, string "CPhysicsWorldBehaviour", size 0x20) oyunun
  dünya singleton'ını kurar; HavokInit (`sub_434FB0`) arena/context
  hazırlar. Port tarafında: behaviour ctor → `CPhysicsWorld` ctor →
  `PhysicsWorld_Create()` (gravity/iter yukarıdaki değerlerle).

## 3. Havok → Bullet eşleme tablosu

| Havok (binary/API) | Bullet (backend) | Not |
|---|---|---|
| `hkpWorld` | `btDiscreteDynamicsWorld` | config+dispatcher+dbvt broadphase+sequential impulse solver |
| `hkpWorld::stepDeltaTime(dt)` | `stepSimulation(dt,1,dt)` | sabit adım |
| `hkpRigidBody` (+Cinfo) | `btRigidBody` | mass/friction/restitution Cinfo'dan |
| `hkpBoxShape` | `btBoxShape` | yarıkutu |
| `hkpSphereShape` | `btSphereShape` | |
| `hkpConvexVerticesShape` (DFF çarpışma) | `btConvexHullShape` | `bConvex=TRUE` yolu |
| `hkpMeshShape`/BSP? | `btBvhTriangleMeshShape` | statik dünya; BSP riski belgeli |
| `hkpPhantom` (trigger) | `btGhostObject` | CF_NO_CONTACT_RESPONSE; **`btGhostPairCallback` kayıt şart** (yoksa overlap listesi boş kalır — testle bulundu) |
| `hkpConstraintSolver` varsayılan | `btSequentialImpulseConstraintSolver` | iter=20 |
| Havok memory arena (dword_660920/sub_52DC80, 0x4000, align16) | standart `new`/`malloc` | arena artık yok; `HavokMemoryManager` oyun tarafı API'si korundu ama backend allocate etmez |
| `hkpWorldCinfo::m_broadPhaseBounds` ±500 | (uygulanmadı) | Bullet geniş aabb kullanır — belgeli sapma |

## 4. Kod değişiklikleri (ne yaptık)

1. **`code/CMakeLists.txt`** — Bullet FetchContent (tag 3.25, shallow);
   demo/unit-test/OpenGL opsiyonları kapalı; yalnız üç alt kütüphane.
   Notlar:
   - Bullet include'ları hedeflere INTERFACE olarak yayılmaz →
     `target_include_directories(... ${bullet3_SOURCE_DIR}/src)` eklendi.
   - Bullet'ın eski `cmake_minimum_required` şartı CMake 4.x'te kırılır →
     `CMAKE_POLICY_VERSION_MINIMUM 3.5`.
2. **`PhysicsBackend.h/.cpp` (yeni)** — backend sarmalayıcı:
   - Dünya: `PhysicsWorld_Create/Destroy/Step/Set·GetGravity`.
   - Gövde: `PhysicsBody_CreateBox` (yarıkutu Cinfo),
     `PhysicsBody_CreateTriMesh` (statik mesh — MeshBundle),
     `PhysicsBody_CreateWithShape` (dış shape ile), Destroy.
   - Şekil: `PhysicsShape_CreateBox/Sphere/ConvexHull/TriMesh` +
     `PhysicsShape_Destroy` (ShapeBundle registry — mesh ömrü shape ile).
   - Trigger: `PhysicsTrigger_CreateBox` (ghost) + Add/RemoveTrigger
     (SensorTrigger grubu) + `PhysicsTrigger_GetOverlaps`.
   - Durum: pozisyon/dönüş (sütun-tabanlı 3x3)/hız/kinematic/impuls.
   - **Sahiplik sözleşmesi**: `CreateBox/CreateTriMesh` gövdeleri kendi
     shape'ini sahiplenir (Destroy otomatik); `CreateWithShape`
     sahiplenmez (`userIndex=1` etiketi) — caller sonra
     `PhysicsShape_Destroy` çağırır.
   - MeshBundle ayrımı kayıt tablosuyla (void* hem `btRigidBody*` hem
     `MeshBundle*` olabildiği için).
3. **`PhysicsWorld.cpp`** — ctor dünya oluşturur; `Update` →
   `PhysicsWorld_Step`; Add/Remove RigidBody/Phantom → backend'e devredildi
   (vektör kaydı `RigidBodyExistsInWorld` için korundu); dtor
   `PhysicsWorld_Destroy`.
4. **`CollisionGeometry.cpp`** — `CreateShapeFromClump` gerçek
   implementasyon: `RpClumpForAllAtomics` → vertex/triangle toplama
   (atomic bazlı base offset), `rpGEOMETRYTRISTRIP` → alternating winding,
   `bConvex` → ConvexHull / değilse TriMesh. `CreateBoxShape`/
   `CreateSphereShape`/`DestroyShape` backend'e devredildi.
5. **`rpworld.h` + `rwstubs.cpp`** — vertex erişim API'si eklendi (stub:
   0/NULL; **librw entegrasyonunda gerçek veri akacak**):
   `RpGeometryGetNumVertices/NumTriangles/Vertices/TriangleIndices/Flags`.
6. **`tests/physics_test.cpp` (yeni)** + `tests/CMakeLists.txt` —
   `physics_test` hedefi (Bullet link'li).
7. **`FUN_MAP.md`** — `FUN_0043a040/43aa20/43ae30/43b9a0/46f040`
   satırları backend'e güncellendi; `FUN_0055e6d0` eklendi.
8. **`ROADMAP.md`** — Faz 4.6 bloğu eklendi.

## 5. Test kapsamı (`physics_test`, 7/7)

| Test | Doğrulanan |
|---|---|
| TestGravityDefault | varsayılan gravity (0,-9.8,0) |
| TestFreeFall | 1 sn = 60×(1/60): v≈-9.8, semi-implicit Euler pozisyon eşleşmesi |
| TestGroundRestAndFriction | zemin üstünde oturma bandı; vx=5 → sürtünme 0.6 ile durma |
| TestImpulse | merkezî impuls → Δv = J/m (12) |
| TestTriggerOverlap | ghost tetik overlap (1 hit, doğru gövde) |
| TestTriMeshStatic | üçgen mesh zemin üzerinde kutu oturması |

Çalıştırma:

```sh
cd code
cmake -B build -DCMAKE_BUILD_TYPE=Release   # ilk kurulum Bullet'ı indirir
cmake --build build -j8
ctest --test-dir build --output-on-failure  # 7/7
./build/CannonCruise                        # link verification OK
```

## 6. Belgeli sapmalar ve açık boşluklar

- **Solver**: sequential-impulse (Bullet) ≠ Havok 2004 solver — birebir
  his garanti edilmez; kritik değerler binary'den alındı, ince kalibrasyon
  asset'lerle (gemi ağırlığı, su itmesi) yapılmalı.
- **AABB ±500**: broadphase clamp'i uygulanmadı (Bullet geniş aabb);
  gerektiğinde oyun içi clamp olarak eklenebilir.
- **Clump atomik transform'u**: `CreateShapeFromClump` atomic frame
  matrisini uygulamıyor (noinstance/tek atomic varsayımı) — çok-atomic
  DFF'lerde yanlış konum verir.
- **Deactivation alanları**: `sub_55E6D0` +0x5C/+0x6C/+0x88 (0.0163/0.2/0.2)
  henüz Bullet deactivation ayarlarına eşlenmedi; gövdeler şimdilik
  `DISABLE_DEACTIVATION` (her zaman aktif — orijinalden sapma, stabilite
  için bilinçli).
- **BSP/BSPWorld**: binary `RpWorldStreamRead` + game plugin chunk'ları
  kullanıyor; Bullet BSP convexification yok — asset gelince bakılacak.
- **Entity bağlantısı**: `CPhysicsEntity` (FUN_00436480/437410/437c60)
  hâlâ backend'e bağlı değil (sonraki iş kalemi).
- **HavokInit**: `sub_434FB0` yaşam döngüsü portlandı (havokstart.cpp) ama
  physics world singleton bağlantısı ev ödevi.
- **Ses `.rws` / Rt2dMaestro**: fizik dışı — librw işi (bkz. ROADMAP).

## 7. Sırada ne?

1. ~~`CPhysicsEntity` → gövde fabrikası bağlantısı~~ **tamam** (Faz 4.6b):
   erişimciler backend'e bağlı; AddToWorld singleton → backend kayıt;
   test `physics_entity_test`.
2. ~~`CollisionTrigger` akışını phantom'a eşlemek~~ **tamam** (Faz 4.6b):
   Initialize (clump sphere stub → AABB ghost) + PollOverlaps
   enter/leave akışı.
3. ~~HavokInit'te dünya singleton bağlantısı~~ **tamam** (Faz 4.6c):
   binary'de singleton'ı startup init (`sub_41A680`) yaratır, behaviour
   kullanır — port `CPhysicsWorld::Create()` + `m_bOwnsWorld` sahiplik
   bayrağıyla hizalandı.
4. `CPhysicsResource::Load` → statik dünya trimesh
   (wbID_STATICWORLDCOLLISION; IDA `sub_439460`) — ~~sayaç/assert portlu;
   stream parse~~ **rwstudio konteyner parse'ı aktif** (Faz 4.6e —
   RwStudioStream .col kaydını buluyor). Kalan: .col inline veri düzeni
   → `PhysicsShape_CreateTriMesh`.
5. ~~Kapı 4: gerçek `.rws` asset + librw entegrasyonu~~ **açıldı**
   (Faz 4.6d/4.6e): librw NULL platform engine + StreamFile shim
   (`rw_librw.cpp`), `rwlibrw_test` + **rwstudio konteyner parser**
   (`RwStudioStream`) gerçek `Streams/*.RWS` dosyalarını envanterliyor
   (MAINMENU 5 asset/4 nesne, GLOBAL 7/48, VIKING01 119/497; gömülü txd
   payload doğrulandı — ctest 10/10). Kalan: render platformu (GL3/D3D9)
   + sahnede sahne testi; txd payload decode (d3d8 IMAGE) ve 0x0704
   property şeması; o zaman `RpGeometryGetVertices`/
   `RpClumpGetBoundingSphere` gerçek veriye dönecek.
