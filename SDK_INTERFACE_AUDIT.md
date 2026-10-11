# Havok / RenderWare Arayüz İncelemesi

> 9 Ekim 2026 · P0 devamı · **SDK entegrasyonu ve Windows derlemesi tamamlanmış değildir.**

## Kapsam ve yöntem

- `code/` altındaki kaynak ve dokuz yerel RenderWare stub başlığı incelendi; eski `code/build/` çıktıları kullanılmadı.
- Worker'da AppleClang 21.0.0, x86_64 kullanılabildi. `cmake`, `cl` ve `ninja` bulunamadı. Kurulum, PATH değişikliği, harici SDK erişimi veya Windows çalıştırması yapılmadı.
- `tools/check_sdk_interfaces.py`, mevcut non-Windows CMake dışlama kurallarını modelleyerek **87 kaynak dosyasını** `-std=c++14 -fsyntax-only` ile kontrol eder. CMake çalıştırmaz, nesne/statik arşiv üretmez.
- Buna ek olarak macOS CMake hedefinin dışladığı `Ocean_W32.cpp` doğrudan kontrol edilir; bu dosya Windows başlığı istemediğinden kontrol mümkündür. Diğer üç Windows kaynağı (`startup.cpp`, `win.cpp`, `rwreg.cpp`) atlanır.
- Startup bildirim/çağrıları kaynak metinden alınarak küçük izole kontrollerde `rwcore.h` ile karşılaştırılır. Sahte `windows.h` kullanılmaz; **bunlar startup.cpp veya Windows/MSVC derlemesi değildir.**
- İlk ölçüm: 87/87 non-Windows kaynak geçti; Ocean ve üç izole kontrol başarısızdı. Tam komutlar, tanılar ve kaynak SHA-256 değerleri `validation/sdk-before.json` içinde korunur.

## 1. RenderWare: derleme engelleri

### RW-01 — Im3D ve global bellek API'si eksik — AÇIK

Kanıt: `code/Ocean_W32.cpp:41–63`; yerel `code/rwcore.h` bu kullanım kümesini bildirmiyor:

- Tür: `RwIm3DVertex`.
- Vertex yardımcıları: `RwIm3DVertexSetPos`, `RwIm3DVertexSetRGBA`, `RwIm3DVertexSetU`, `RwIm3DVertexSetV`.
- Çizim: `RwIm3DTransform`, `RwIm3DRenderPrimitive`, `RwIm3DEnd`.
- Global bellek çağrıları: `RwMalloc`, `RwFree`.
- Sabitler: `rwRENDERSTATESHADEMODE`, `rwSHADEMODEGOURAUD`, `rwIM3D_VERTEXUV`, `rwIM3D_VERTEXRGBA`.

İlk Ocean kontrolü 14 Clang hatası verdi; bazıları ilk eksik türün zincirleme sonucudur. Hata sayısı bağımsız açık sayısı olarak yorumlanmamalı.

`RWS::CMemoryHandler::RwFree(RwFreeList*, void*)` mevcut olsa da global `RwFree(void*)` değildir; farklı kapsam ve imza nedeniyle yerine geçirilemez. `RwAlloc` da `RwMalloc` için kanıtlanmış bir SDK uygulaması değildir.

**Gerekli karar:** Doğrulanmış SDK başlıkları mı, yalnızca derleme amaçlı ve açıkça ayrılmış bir stub katmanı mı kullanılacak? Vertex layout'u, renk paketleme, bayrak/sabit değerleri ve bellek callback ABI'si SDK/binary kanıtıyla doğrulanmadan tahmini tanımlar eklenmedi. Özellikle kaynakta kullanılan `rwIM3D_VERTEXRGBA` adının hedef SDK'da gerçekten var olduğu ayrıca kontrol edilmeli.

### RW-02 — Engine kapanış bildirimlerinin dönüş tipi çakışıyor — YEREL OLARAK DÜZELTİLDİ

`startup.cpp` içindeki `extern void RwEngineTerm/Stop/Close(void)` bildirimleri, `rwcore.h` içindeki `RwBool` dönüşleriyle çakışıyordu. İlk izole kontrol üç gerçek Clang hatası üretti.

Üç yinelenen bildirim kaldırıldı; mevcut ortak başlık tek bildirim kaynağı oldu. Çağrılar ve kapanış sırası değiştirilmedi. Bu düzeltme yerel tutarlılığı sağlar, gerçek SDK imzasını doğrulamaz.

### RW-03 — RwVideoMode bilinmiyor — POINTER KULLANIMI DÜZELTİLDİ

`startup.cpp` içindeki `CameraInit` ve `RwStartupInitialize` yalnızca `RwVideoMode*` alıyor. `rwcore.h` içine opaque forward declaration eklendi. Yapı alanı/boyutu uydurulmadı; değer oluşturma veya alan erişimi ileride gerçek SDK tanımı gerektirir.

### RW-04 — RwEngineOpen çağrısında parametre eksik — AÇIK

`startup.cpp`, `RwEngineOpen()` çağırıyor; ortak stub `RwEngineOpen(void* param)` bildiriyor. İzole çağrı kontrolü başarısız.

`nullptr` ekleyerek hatayı gizlemek yerine gerçek open-param yapısı, HWND/cihaz bağlama ve `subSystemConfig` verisinin anlamı araştırılmalı. Startup'taki adres yorumları tek başına wrapper ile SDK fonksiyonunu ayırmak için yeterli değil.

### RW-05 — RwEngineInit ve bağlantı bildirimleri ayrışıyor — AÇIK

- `rwcore.h`: `RwEngineInit(void*, RwInt32, RwMemoryFunctions*)`, `extern "C"` içinde.
- `startup.cpp`: `RwEngineInit(void*, RwUInt32, RwUInt32)`, ayrı yerel bildirim.
- `memoryhandler.h` örneği callback tablosunu üçüncü argümana geçirirken startup farklı sözleşme varsayıyor.

İzole `RwEngineInit(memFuncs, 0, 0)` kontrolü Clang'da **geçti** (`validation/startup-init.json`). Bu, iki imzanın eşdeğer olduğu anlamına gelmez: derlenebilen ayrı overload, yanlış/missing dış sembol riskini ortadan kaldırmaz. Ayrıca `RwEngineSetSubSystem` ve `RwEngineSetVideoMode` yalnızca startup'ta bildiriliyor; SDK sözleşmesi ve C/C++ linkage tek yerde toplanmalı.

## 2. RenderWare: link ve ABI kapıları

Yerel başlıklar bildirim sağlar; gerçek SDK fonksiyon uygulamaları değildir. Statik kaynak kontrolü, `RwRenderStateGet`, matrix/camera erişimleri, plugin callback'leri ve stream işlevlerinin gerçek imza/layout/sabitlerini doğrulamaz.

Özellikle `RwMemoryFunctions`, `RwGlobals`, `RwFreeList`, `RwFrame`, Im2D/Im3D düzenleri; enum/chunk sabitleri; calling convention, struct packing, x86 pointer boyutu ve CRT eşleşmesi SDK entegrasyonu öncesinde karşılaştırılmalı. Yerel başlıkların gerçek SDK'yı include sırasıyla gölgelemesi engellenmeli.

Kaynak taramasında startup içinde bildirilen ve çağrılan, fakat tanımı görülmeyen wrapper adayları:

- `HavokInit`, `FileSystemInit`, `TextureCacheInit`, `RwStartEngine`, `CameraInit`.
- `RwEngineSetConfig`, `SetLightAmbient`, `RenderPipelineInit`, `GameManagerInit`.
- `CameraShutdown`, `RenderPipelineShutdown`, `SubSystemShutdown`, `TextureCacheShutdown`, `GameManagerShutdown`, `ResourceShutdown`, `GeneralShutdown`.
- Win32 döngüsünde `InitEngine`, `CleanupEngine`, `Idle`, `ResizeCameraRasters`, global `GetTime`.

`CMenuGetTimeDelegate::GetTime()` global Win32 `GetTime()` ile aynı sembol değildir. Bu liste kaynak taraması sonucudur; **linker tarafından üretilmiş tam unresolved-symbol listesi değildir**. EXE hedefi ve gerçek link koşusu hâlâ yoktur.

## 3. Havok: kullanılan yerel arayüz ve davranış açıkları

### HK-01 — Dünya yaşam döngüsü ve SDK bağlantısı yok — AÇIK

`CPhysicsWorld` içindeki `m_pHavokWorld` private `void*`; constructor sıfırlıyor, mevcut sınıfta dünyayı oluşturan veya bu üyeye dünya atayan yol yok. `Update()` içindeki `stepDeltaTime` yorum satırı. Gerçek Havok başlık include'u veya aktif `hkp*` tür çağrısı görülmedi. Yorumlardaki isimler Havok sürümünün kanıtı değildir.

### HK-02 — Body/phantom işlemleri yalnızca vektör kaydı — AÇIK

- `AddRigidBody` / `RemoveRigidBody` / `RigidBodyExistsInWorld`: `std::vector<void*>` üyeliği.
- `AddPhantom` / `RemovePhantom`: diğer vektörde üyelik.
- SDK dünyasına entity ekleme/çıkarma, refcount, ownership, lock/thread kuralları ve collision filtreleri uygulanmıyor.
- `CPhysicsEntity::SetRigidBody` yalnızca pointer atıyor; `AddToWorld` / `RemoveFromWorld` yerel kayıt katmanına gidiyor.
- `SetPosition`, `GetPosition`, `SetLinearVelocity`, `GetLinearVelocity` yalnızca assert yapıyor. Özellikle getter'lar çıkış vektörünü doldurmuyor; geçerli fizik sonucu kabul edilemez.

### HK-03 — Fizik event bağlantısı eksik — AÇIK

`CPhysicsWorldBehaviour::HandleEvents` mesaj kimliğini sınamadan her teslim edilen mesajda `Update(1/60)` çağırıyor; constructor'da fizik mesaj aboneliği yok. Fizik adımı zaten no-op olduğu için bu durum şimdilik simülasyon ilerletmiyor. Event ID/abonelik sözleşmesi kanıtlanmadan yeni bir ID uydurulmadı.

### HK-04 — Yerel allocator pointer maskesi — DÜZELTİLDİ / TEST EDİLDİ

Eski `~(alignment - 1)` işlemi `RwUInt32` genişliğinde kalıyordu. x64'te yüksek pointer bitlerini sıfırlayabiliyordu. Örnek: `0x100000018` adresini 16'ya hizalarken eski ifade `0x20`, geniş maske `0x100000020` veriyor; izole compile-time assert ile doğrulandı.

- Maske complement işleminden önce `uintptr_t` genişliğine taşındı.
- Tahsis boyutu toplama taşması kontrolü eklendi; toplama `size_t` ile yapılıyor.
- Mevcut 16-byte sözleşmesi korundu; Release'te de desteklenmeyen hizalama `nullptr` döndürüyor.
- `Initialize/Shutdown` hâlâ no-op. Bu değişiklik Havok callback kaydı veya fizik entegrasyonu değildir.
- Yanıltıcı allocator ve dünya açıklamaları mevcut davranışa göre düzeltildi.

`tests/havok_memory_manager_test.cpp`: 0, 1, 15, 16, 17, 4096 ve 1048576 bayt tahsis; hizalama; ayrılmış alanı yazma/serbest bırakma; null-free; Release'te geçersiz hizalama kontrolleri. **Debug ve Release, ASan+UBSan ile geçti.** 32-bit taşma testi test dosyasında mevcut, fakat x64 koşusunda çalışmadı; Windows/x86 koşusunda doğrulanmalı.

İlk test koşusu bir yorum düzenleme hatasını yakaladı (`validation/allocator-tests.json`); hata düzeltildi ve başarılı tekrar `validation/allocator-tests-fixed.json` dosyasına yazıldı. Ara kaynak taraması değişen kaynaklarla karışık sonuç üretmemesi için durduruldu; `sdk-after.json` oluşmadı. Başarısızlık kanıtları silinmedi.

## 4. Yeniden çalıştırma

Clang önkontrolü (depo kökünde, araç önceden kullanılabilir olmalı):

```sh
python3 tools/check_sdk_interfaces.py --output validation/sdk-next.json
```

Mevcut raporun üzerine yazmaz. Açık Ocean/Open engelleri nedeniyle **nonzero çıkış beklenir**; bu durum başarı olarak sunulmamalı. Script CMake kaynak kapsamını modellemektedir; CMake değişirse eşlemesi ayrıca güncellenmelidir.

SDK bağımsız allocator testi için örnek (depo kökü; yeni çıktı adları kullanın):

```sh
mkdir -p build/sdk-audit-tests
clang++ -std=c++14 -Wall -Wextra -Werror -fsanitize=address,undefined -Icode \
  tests/havok_memory_manager_test.cpp code/HavokMemoryManager.cpp \
  -o build/sdk-audit-tests/havok-memory-next-debug
./build/sdk-audit-tests/havok-memory-next-debug
# Release için aynı komuta -DNDEBUG -O2 ve farklı çıktı adı ekleyin.
```

## 5. Son doğrulama sonucu

`validation/sdk-final.json` son kaynaklarla yeniden çalıştırıldı; kayıtlı kaynak hash'leri mevcut dosyalarla eşleşiyor:

- **87/87** non-Windows kaynak syntax kontrolünden geçti. Uyarısız derleme iddiası yok: `AnimController.cpp:43` tamsayıdan pointer'a dönüşüm; `SceneManager.cpp:81` format argümanı uyarıları sürüyor ve bu değişikliklerde ele alınmadı.
- **Ocean: başarısız.** RW-01 açık.
- **İzole startup kontrolleri: 2/3 geçti.** Dönüş tipi çakışmaları ve opaque video türü düzeldi; Open çağrısı başarısız olmaya devam ediyor.
- Önkontrol genel sonucu **nonzero**; Windows kapısı tamamlanmadı. Üç Windows kaynağı kontrol dışında.
- `validation/allocator-tests-fixed.json`: Debug ve Release derleme/çalıştırma çıkışları sıfır; ASan+UBSan tanısı yok. Bu logdaki test ve kaynak hash'leri de mevcut dosyalarla eşleşiyor.
- Belge bağlantıları ve kod blokları ayrıca kontrol edildi. CMake yapılandırması, statik oyun kütüphanesi, Windows/MSVC, SDK link ve oyun çalışma zamanı testleri yapılmadı.

## 6. Sonraki kabul kapıları

1. RenderWare sürümü ve kullanılacak lisanslı SDK/derleme-stub stratejisini netleştir; RW-01, RW-04 ve RW-05'i doğrulanmış sözleşmeyle kapat.
2. Windows makinesinde [BUILDING.md](BUILDING.md) reçetesini çalıştır; `configure.log`, Debug/Release logları, araç sürümleri ve Win32/SDK seçimini kaydet. macOS'ta MSVC çalıştırılmış gibi raporlama yapma.
3. Havok backend seçimi, dünya/body ownership ve olay sözleşmesini belirle; sessiz no-op fiziği çalışan simülasyon sayma.
4. Daha sonra EXE/link kapısı, gerçek SDK ABI karşılaştırması ve tek sahne çalışma zamanı doğrulaması.
