# CannonCruise

CannonCruisePC oyununun tersine mühendislikle yeniden inşa edilen kaynak kodu.
Orijinal 2004 tarihli 32-bit Windows oyunu (`CannonCruise.exe`) Ghidra ve IDA
analizleri (`CC.asm`, `CC_ida.asm`) temel alınarak C++14 ile yeniden yazılmaktadır.

**Durum:** Aktif geliştirme — Faz 4 (derleme/link) macOS üzerinde tamamlandı;
Windows/MSVC derlemesi ve çalışma zamanı doğrulaması bekliyor.

## Dizin yapısı

| Yol | İçerik |
|---|---|
| `code/` | Yeniden yazılmış kaynak kodu (oyun + framework) |
| `code/framework/` | RenderWare Studio framework çekirdeği (event/attribute/stream/memory) |
| `code/havok/` | Havok Physics 2.2.1 stub header'ları (Faz 4.3) |
| `code/rwstubs.cpp` | RenderWare SDK link stub'ları (Faz 4.5) |
| `code/build/` | CMake çıktıları (izlenmez) |
| `CC.asm` / `CC_ida.asm` | Ghidra / IDA tersine mühendislik dökümleri (referans) |
| `tests/` | SDK bağımsız birim testleri (ör. Havok allocator) |
| `tools/`, `validation/` | Arayüz denetim betikleri ve doğrulama JSON çıktıları |
| `DLLs/`, `Streams/`, `Sound/`, `Localiza/`, `Setup/` | Orijinal oyun dosyaları (referans/varlık) |

## Derleme

### macOS (doğrulanmış)

```bash
cd code
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j8
./build/CannonCruise        # link doğrulama girişi — "link verification OK" yazmalıdır
```

- `CannonCruiseLib` statik kütüphanesi + `CannonCruise` link doğrulama hedefi üretilir.
- Hedef, `-Wl,-force_load` ile tüm arşiv nesnelerini bağlar; eksik sembol varsa
  link hatası verir. Ayrıca `CannonCruise` hedefi yalnızca sistem kütüphanelerine
  dinamik bağlanır (oyun SDK'ları henüz link edilmiyor).

### Windows (bekliyor)

Windows/MSVC reçetesi için [BUILDING.md](BUILDING.md). macOS sonucu bu adımın
yerine geçmez (roadmap 4.4b).

## Belgeler

- [ROADMAP.md](ROADMAP.md) — fazlar, doğrulama kapıları, güncel durum
- [ROADMAP_MACOS.md](ROADMAP_MACOS.md) — macOS port yol haritası
- [BUILDING.md](BUILDING.md) — Windows/MSVC derleme reçetesi ve bağımlılık envanteri
- [SDK_INTERFACE_AUDIT.md](SDK_INTERFACE_AUDIT.md) — RenderWare/Havok arayüz denetimi
- [LICENSE](LICENSE) — GPL-3.0

## Testler

SDK bağımsız testler `tests/` altındadır. Tümü CMake/ctest ile çalışır:

```bash
cd code
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j8
ctest --test-dir build --output-on-failure
```

Kapsam: `havok_memory_manager` (allocator), `ceventhandler` (RegisterMsg/
LinkMsg/SendMsg/yaşam döngüsü), `cclassfactory` (Register/Create/Shutdown),
`cattributehandler` (instance registry + paket öngörücüler). Tek testi
sanitizer ile elle de derleyebilirsiniz:

```bash
c++ -std=c++14 -fsanitize=address,undefined \
    tests/havok_memory_manager_test.cpp code/HavokMemoryManager.cpp \
    -Icode -o /tmp/havok_test && /tmp/havok_test
```

## Mimari notlar

- **RenderWare 3.6 / RenderWare Studio:** Yerel stub başlıklar (`code/rwcore.h`
  vb.) bildirim sağlar; gerçek SDK kütüphaneleri bağlanmamıştır. Oyun kodu
  RenderWare tipleriyle (`RpClump*`, `RwFrame*` vb.) yazar.
- **Havok Physics 2.2.1:** `code/havok/` stub header'ları binary adlandırmasıyla
  (`hkWorld`, `hkRigidBody` — `hkp` öneki yok) eşleşir. `CPhysicsWorld` /
  `CPhysicsEntity` şimdilik `void*` kullanır; SDK entegrasyonu bekliyor.
- **Framework:** `RWS::CEventHandler`, `CAttributeHandler`, `CClassFactory`
  RenderWare Studio davranışını taklit eder; `FUN_*` binary adresleri kod
  yorumlarında korunur.

## Uyarı

Bu proje tersine mühendislik içerir; orijinal oyun telif hakkıyla korunur.
Depodaki orijinal oyun dosyaları yalnızca analiz/referans amaçlıdır.
