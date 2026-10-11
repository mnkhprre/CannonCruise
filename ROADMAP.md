# 🗺️ CannonCruise — Proje Durum Raporu & Yol Haritası

> **Son Güncelleme:** 9 Ekim 2026  
> **Proje:** Cannon Cruise PC — Tersine mühendislik ile kaynak kod yeniden inşası  
> **Referanslar:** `CannonCruise.exe`, `CC.asm`, `CC_ida.asm`  
> **Derleme temeli ve reçete:** [BUILDING.md](BUILDING.md)  
> **SDK arayüz incelemesi:** [SDK_INTERFACE_AUDIT.md](SDK_INTERFACE_AUDIT.md)
> **Ayrı macOS port planı:** [ROADMAP_MACOS.md](ROADMAP_MACOS.md) — temiz statik derlemeden native uygulama ve paketlemeye uzanan bağımsız yol haritası.

## Durumların anlamı

**Kaynakta mevcut**, **derlendi**, **linklendi** ve **çalışırken doğrulandı** farklı aşamalardır. Gövdesi veya belgesi bulunan bir metot, orijinal oyunla eşdeğer kabul edilmez. Önceki rapordaki tamamlanma iddiaları aşağıda mevcut dosya kanıtlarıyla ayrıştırılmıştır; bu incelemede oyun çalıştırılmadı.

## Genel bakış — 9 Ekim incelemesi

- `code/` altında, `build/` hariç **194 `.h`/`.cpp` dosyası**: kökte 147 (oyun ve SDK stub başlıkları birlikte), framework altında 47. Önceki 143 + 47 sayımı güncel değil.
- **Sıfır baytlık dosya yok:** `framework/startup/videomode.cpp` dolduruldu (CameraInit puanlama mantığı + CameraShutdown). Dosyanın dolu olması da uygulamanın tamamlandığını göstermez.
- Doxygen kapsamı önceki raporda 190/190 olarak bildirilmişti; güncel dosya kümesi üzerinde tamlık kontrolü ve Doxygen üretimi yapılmadı.
- Mevcut hedef: **`CannonCruiseLib` statik kütüphanesi**; CMake'te yürütülebilir hedef ve SDK bağlantıları yok.
- `code/build/` içinde macOS derleme cache'i ve `libCannonCruiseLib.a` mevcut. Cache derleyici metadatası AppleClang 21.0.0.21000101, x86_64, 8 bayt pointer gösteriyor. Bunlar önceki derlemenin izleri; güncel kaynakların temiz derlemesini kanıtlamaz.
- Orijinal `CannonCruise.exe`: PE imzası, `Machine=0x014c` ve `OptionalHeader.Magic=0x010b` ile **PE32 / x86** doğrulandı; dosya çalıştırılmadı.
- README henüz yok; ilk derleme belgesi `BUILDING.md` oluşturuldu.

## Faz 1 — Dokümantasyon

Önceki raporda tamamlandı olarak bildirilmişti. Çok sayıda kaynakta Doxygen açıklamaları mevcut; **%100 kapsam ve açıklamaların uygulamayla tutarlılığı doğrulanmış değil**. Örneğin `PhysicsWorld.h` simülasyon adımını tarif ederken `.cpp` içindeki çağrı yorum satırıdır.

## Faz 2 — Boş dosyaları doldurma

Önceki raporda GlobalFunctions, MissionIndicator, TextureResource, WbCamera, WbRender ve WorldGeometryResource dosyalarının doldurulduğu bildirilmişti. Bu dosyalar mevcut ve sıfır bayt değil. İnceleme tarihinde boş olan `framework/startup/videomode.cpp` da dolduruldu: `CameraInit` (FUN_0041af10 — alt sistem/video modu puanlaması, "No best sub-system" assert sözleşmesi) ve `CameraShutdown` (FUN_0041ac00 — singleton shutdown sözleşmesi) binary yorumuyla yazıldı. Proje genelinde sıfır bayt dosya kalmadı; işlevsel tamamlanma ayrıca incelenmelidir.

## Faz 3 — Render ve fizik stub'ları (KISMİ)

Önceki “8/8 tamamlandı” ifadesi kaldırıldı. Listede yedi render metodu ve bir fizik güncellemesi vardır. Kaynak incelemesi sonuçları:

- `CFade::Render()`: Im2D çizim gövdesi mevcut; görsel doğrulama bekliyor.
- `CGUIHealthBar::Render()`: Im2D çizim gövdesi mevcut; görsel doğrulama bekliyor.
- `CGUIMap::Render()`: çizim gövdesi mevcut; sabit boyut, varsayımsal ölçek ve sabit işaret rengi kullanıyor. Orijinal davranış doğrulanmadı.
- `CShowText::Render()`: arka plan şeridine ek olarak gerçek metin çizimi eklendi — önce FontSystem (`fonts/*.met` + glif UV'leri, `FontSystem_DrawText`); font bulunamazsa gömülü 5x7 bitmap font (ASCII 32–126) ile Im2D quad çizimi fallback, satır bölme (`\n` + kelime sarmalama) ve `MSG_SHOWTEXT_RENDER` event bağlantısı. METRICS3 ayrıştırıcı port edildi ancak kontur raster (`sub_4D54C0`) + gerçek font varlıkları beklediği için 5x7 fallback şimdilik varsayılan yoldur.
- `COceanTile::Render()`: kontrollerden sonra `RenderWaterMesh()` çağrısı mevcut; çalışma zamanı doğrulanmadı.
- `COcean_W32::RenderWaterMesh()`: RenderWare Im3D çizim gövdesi mevcut; doğrudan tamamlanmış bir D3D9 entegrasyonu olarak değerlendirilmemeli. `Initialize()` yalnızca TRUE döndürüyor; `Shutdown()` boş.
- `CPerformanceVisualizer::Render()`: arka plan paneli çiziliyor; performans verisi/grafik/metin çizimi yok.
- `CPhysicsWorld::Update()`: **etkisiz stub**; `hkpWorld::stepDeltaTime` çağrısı yorum satırında. Fizik simülasyonu ilerlemiyor.

Bu liste proje genelindeki tüm stub'ların envanteri değildir.

## Faz 4 — Build sistemi ve derleme (AKTİF)

Hedef: Windows üzerinde MSVC ile derlenen, ardından gerçek bağımlılıklarıyla linklenen bir uygulama.

- **4.0 — Durum raporu ve derleme temeli:** Tamamlandı (belgeleme/kaynak incelemesi). Win32/x86 hedefi, önerilen araç sürümleri, bağımlılık açıkları ve temiz derleme reçetesi `BUILDING.md` içinde. Windows yürütme doğrulaması buna dâhil değil.
- **4.1 — Kaynakların CMake'e alınması:** Recursive glob mevcut. Eski `code/build/` içindeki CMake derleyici tanılama kaynakları artık hariç tutuluyor. Windows kaynakları yalnızca Windows yapılandırmasında dâhil; nihai kaynak kapsamı derleme sırasında doğrulanacak.
- **4.2 — RenderWare stub başlıkları:** **Tamamlandı (sözleşme düzeltmeleri).** Arayüz incelemesi yapıldı. Üç engine dönüş tipi çakışması kaldırıldı; opaque `RwVideoMode` bildirimi eklendi. Im3D (`RwIm3DVertex` + makrolar + `RwIm3DTransform/End/RenderPrimitive/RenderIndexedPrimitive`) ve bellek (`RwMalloc/RwFree/RwRealloc`) bildirimleri eklendi; `RwRENDERSTATESHADEMODE`/`rwSHADEMODEGOURAUD` tanımlandı. **Açık konular binary kanıtıyla kapatıldı:** `RwEngineOpen` **argümansız** (`FUN_00422960` `retn`, startup `RwEngineOpen()` çağırıyor), `RwEngineInit` **(memFuncs, flags(bit0), resEntrySize)** (`FUN_0050b640` — `arg_4` byte flag okuması; startup `(memFuncs, 0, 0)`), `RwEngineSetConfig` ise **tek pointer argümanlı** (`FUN_0050f660` — Ghidra:
`undefined4 FUN_0050f660(undefined4 * param_1)`; IDA: arg_0'ı 12 bayt okuyup
motor nesnesine +0xC'e kopyalar) → `RwEngineConfig` struct'ı + `const
RwEngineConfig*` imzası düzeltildi, startup çağrısı yapı kurularak
hizalandı. `rwstubs.cpp`'ye engine lifecycle durum makinesi (Init→Open→Start + Close→Stop→Term) eklendi; `startup.cpp`'deki duplicate extern'ler kaldırıldı, çağrı `RwMemoryFunctions*` cast'ine hizalandı. Gerçek SDK/ABI ve Windows doğrulaması bekliyor.
- **4.3 — Havok arayüzleri:** Havok **2.2.1** stub header'ları oluşturuldu: `code/havok/havok.h` şemsiyesi + `hkBaseTypes`, `hkMemory`, `hkClass`, `hkWorld`, `hkEntity`, `hkPhantom`, `hkShape`, `hkConstraint`, `hkCollisionFilter`. Adlandırma binary ile doğrulandı (`hkWorld`/`hkEntity` — `hkp` öneki Havok 4+). 11 header da C++14 ile syntax kontrolünden geçti. Backend entegrasyonu (dünya oluşturma, `stepDeltaTime`, entity konum/hız) hâlâ bekliyor; `CPhysicsWorld`/`CPhysicsEntity` `void*` kullanmaya devam ediyor.
- **4.4a — macOS/Clang statik kütüphane:** **Tamamlandı.** Temiz `code/build/` yapılandırması ile tüm non-Windows kaynakları Clang ile derlendi (0 hata); `libCannonCruiseLib.a` üretildi. Eksik Im3D bildirimleri tamamlandığı için `Ocean_W32.cpp` artık her platformda dâhil (CMakeLists'ten hariç tutma kaldırıldı). Eksik C++ tanımları eklendi: `CEventHandler(RwUInt32)`, `CAttributeHandler(CAttributePacket)`, `GamePluginAttach` (stub) ve `rwstubs.cpp` (86 RenderWare SDK stub fonksiyonu).
- **4.4b — Windows/MSVC statik kütüphane:** **Bekliyor.** macOS sonucu bu adımın yerine geçmez.
- **4.5 — Yürütülebilir hedef ve linker doğrulaması:** **Tamamlandı (macOS).** `add_executable(CannonCruise main.cpp)` hedefi eklendi; `-Wl,-force_load` ile arşivin **tüm** nesneleri bağlanıyor → eksik sembol kalırsa link hatası verir. Temiz derleme: 0 hata, link başarılı, `./CannonCruise` çalışıyor (çıkış 0). Kalan tek tanımsız sembol yok (yalnızca sistem/libs++ sembolleri — normal). Windows/MSVC link doğrulaması 4.4b ile birlikte bekliyor.

## Faz 5 — Proje altyapısı (TAMAMLANDI)

- **5.1:** **Tamamlandı.** `README.md` — proje tanımı, dizin yapısı, macOS derleme
  komutları, belge bağlantıları, test çalıştırma, mimari notlar.
- **5.2:** **Tamamlandı.** `.gitignore` — build çıktıları, CMake/IDE artıkları,
  `.DS_Store`, `*.bak`, Doxygen HTML çıktıları. (Mevcut olarak track edilen
  `.DS_Store`/kullanıcı dosyaları için `git rm --cached` gerekir; bilinçli olarak
  dokunulmadı.)
- **5.3:** **Tamamlandı.** `Doxyfile` (doxygen 1.18) + üretilmiş `docs/html/`
  (15 MB). Yeniden üretim: `doxygen Doxyfile`.
- **5.4:** **Tamamlandı.** `STYLE.md` — mevcut kod konvansiyonları (girinti,
  isimlendirme `m_`/`C` önekleri, K&R parantez, dosya başlığı bloğu, FUN kanıt
  koruma kuralı, doğrulama komutları).
- **5.5:** **Tamamlandı.** `FUN_MAP.md` — `tools/extract_fun_map.py` ile koddan
  üretilen **372** `FUN_*` adresi; **157** `Class::Method` eşleşmesi, **248**
  açıklayıcı metin, **124** yalnızca dosya bağlamı. Yeniden üretim:
  `python3 tools/extract_fun_map.py`.

README, Windows CI ve SDK'dan bağımsız temel testler, derleme çalışmalarıyla paralel yürütülebilir.

## Faz 6 — İleri hedefler

- Orijinal asset'lerle başlatma → pencere/render cihazı → asset yükleme → menü → tek sahne → kontrollü kapanış.
- Fizik, AI ve collision testleri; orijinal davranışla karşılaştırma.
- CI/CD ve mimari belgeler/wiki.

## İlerleme özeti ve sıradaki adım

Ölçülmemiş “uygulama %100 / build %85” oranları yerine doğrulama kapıları kullanılır:

1. **Derleme temeli belgeli:** Evet; Windows reçetesi henüz çalıştırılmadı.
2. **Temiz Windows statik kütüphanesi:** Bekliyor.
3. **Dış sembolleri çözülmüş EXE:** **Evet (macOS/Clang).** `-force_load` ile tüm arşiv nesneleri bağlandı; `CannonCruise` hedefi derleniyor, linkleniyor ve çalışıyor. Windows/MSVC karşılığı bekliyor.
4. **Tek sahne çalışma zamanı doğrulaması:** Bekliyor.

P0 devamında SDK arayüz envanteri, sınırlı kaynak düzeltmeleri ve tekrar çalıştırılabilir Clang önkontrolü eklendi. Ayrıntılar ve test kanıtları [SDK_INTERFACE_AUDIT.md](SDK_INTERFACE_AUDIT.md) içinde.

Sıradaki adımlar (paralel):
1. **4.4b — Windows/MSVC derlemesi:** `BUILDING.md` reçetesinin Windows
   makinesinde çalıştırılması. Bu macOS worker'ında MSVC yok. Link bekleyen
   **tüm semboller artık tanımlı**: 13 startup fonksiyonu + engine lifecycle
   + **win32'nin 5 köprü sembolü** (`Idle`, `InitEngine`, `CleanupEngine`,
   `ResizeCameraRasters`, `GetTime` → yeni `framework/startup/win32/enginewin.cpp`;
   win.cpp ile mangled çapraz kontrol edildi). `TextureCacheInit`/`GetTime`
   WINMM kullanır → MSVC `winmm.lib` bağlantısı (`#pragma comment` her iki
   dosyada mevcut). macOS'ta doğrulanamayan tek kalan derleme = Windows.
2. **Görsel doğrulama:** `CShowText::Render` metin çizimi, `videomode.cpp`
   CameraInit mod seçimi ve mevcut Im2D stub'larının gerçek RW SDK ile
   çalıştırılması. Orijinal `fonts\*.met` font sistemi — **Aşama 1 + 2 + 3
   tamamlandı** (METRICS1 + METRICS3 yükleyicileri + glif sorgusu + çizim
   yolu `FontSystem_DrawText` + UTF-16LE + `CShowText` bağlantısı); ayrı
   iş kalemleri: METRICS2 (FUN_004ce0d0 — analiz edildi, raster piksel
   taraması nedeniyle port bloklu), M3 kontur raster (sub_4D54C0 —
   Aşama 3b'de portlandı) ve binary'nin stream font yükleyicisi
   (sub_4CC690 — bu oturumda portlandı).

Tamamlanan (önceki oturum):
- `tests/CMakeLists.txt` + `code/CMakeLists.txt`'ye `enable_testing()`;
  `ctest` ile 4 SDK bağımsız test (`havok_memory_manager`, `ceventhandler`,
  `cclassfactory`, `cattributehandler`) — 4/4 geçiyor.
- `rwcore.h`/`rwstubs.cpp`: `RwVideoMode` (6 dword) + alt sistem/video modu
  sorgu API'leri (`RwEngineGetNumSubSystems` … `RwEngineSetVideoMode`).

Tamamlanan (bu oturum — startup alt sistemi tamamlama):
- **4.2 kapatıldı** (bkz. Faz 4 maddesi): rwcore.h imza düzeltmeleri,
  rwstubs engine lifecycle, startup.cpp hizalama.
- **13 eksik startup fonksiyonu binary analiziyle yazıldı** (5 yeni dosya,
  `framework/startup/` altında, `GLOB_RECURSE` ile otomatik dâhil):
  - `gamemanager.cpp` — `GameManagerInit`/`GameManagerShutdown` + `sub_449D30`
    serbest liste zinciri (10000 × 12 bayt giriş tablosu) tam restore edildi.
  - `renderpipeline.cpp` — `RenderPipelineInit`/`RenderPipelineShutdown`
    (iMsgStartSystem/iMsgStopSystem kaydı, `CLinearMemoryManager` sıfırlama,
    tick geri çağırısı) + `SetLightAmbient` (motor komut altyapısı TODO).
  - `havokstart.cpp` — `HavokInit` (0x4030 singleton, 3 refcounted alt sistem
    nesnesi, `sub_52DCA0` context takası) + `TextureCacheShutdown` (aynı
    global yaşam döngüsü: `byte_660934`, `dword_660938/3C/40`).
  - `systemservices.cpp` — `FileSystemInit` (**binary'de `nullsub_11` = 1 bayt
    `retn` — no-op olarak sadık kalındı**), `TextureCacheInit` (WINMM zaman
    çözünürlüğü), `SubSystemShutdown` (cihaz listesi sözleşmesi),
    `ResourceShutdown` → `CAttributeHandler::Shutdown()` (assert path
    `cattributehandler.cpp` kanıtı), `GeneralShutdown` → `CEventHandler::Shutdown()`.
  - `rwstartengine.cpp` — `RwStartEngine` (B:..Z: sürücü taraması, arama yolu
    oluştur/doğrula/aktifleştir; POSIX karşılığı cwd tek arama yolu).
- `ceventhandler`e `UnRegisterMsg` eklendi (`loc_40CC60` karşılığı).
- Doğrulama: 0 hata/uyarı, `link verification OK`, ctest 4/4, `nm` ile 15
  sembolün arşivde tanımlı olduğu çapraz kontrol edildi.

Tamamlanan (bu oturum — win32 köprü + sözleşme düzeltmeleri):
- **win32'nin 5 eksik sembolü yazıldı** — yeni
  `framework/startup/win32/enginewin.cpp` (yalnız Windows'ta derlenir):
  - `GetTime` (`FUN_00482BE0` — binary dword_657AA8 önbelleği; mantık adımı
    olmadığı için WINMM `timeGetTime()` aynı ms sözleşmesiyle).
  - `Idle` (`FUN_0041C380` — "MainLoop::Poll" iskeleti; varlık logları,
    mantık adımı sub_41B900, render adımı sub_41CEB0 → TODO Faz 6).
  - `ResizeCameraRasters` (`FUN_00418130` — ctx denetimi + geçici
    `iMsgResizeCameraRasters` RegisterMsg/SendMsg/UnRegisterMsg akışı;
    raster boyutlandırma alt sistemleri TODO Faz 4.4b).
  - `CleanupEngine` (`FUN_00417930` — GetWindowPlacement → rwreg'e
    PosX/PosY/SizeX/SizeY/Minimized/Maximized kaydı; motor kapatma YOK,
    o ayrı zincirde RwStartupShutdown).
  - `InitEngine` — köprü: startup.cpp `RwStartupInitialize` (binary
    sub_41A680 = RwStartup::Initialize) tam zincirine bağlanır.
  - Doğrulama: sahte SDK başlıklarıyla `-Wall -Wextra` syntax temiz;
    win.cpp extern'leri ↔ enginewin tanımları **mangled çapraz kontrol**
    (5/5 + `RwStartupInitialize` birebir eşleşti).
- **`RwEngineSetConfig` sözleşmesi düzeltildi** (3 float → `const
  RwEngineConfig*`; Ghidra/IDA kanıtı), `RwEngineIsRunning()` eklendi
  (`sub_41CDF0` geçici karşılığı); startup.cpp struct ile çağıracak
  şekilde hizalandı.
- **RenderPipelineInit adım 3 / Shutdown adım 7**: `iMsgDeleteEventHandler`,
  `iMsgDeleteEntity`, `iMsgOnDeleteEntity` (format `CEventHandler*`)
  RegisterMsg/UnRegisterMsg ile bağlandı (binary sub_40BF10 / sub_40BF60).
- Önemsiz 2 derleme uyarısı giderildi (AnimController int→ptr cast,
  SceneManager `%%s` format bug'ı).
- Doğrulama: 0 hata/uyarı, `link verification OK`, ctest 4/4.

Tamamlanan (bu oturum — font sistemi Aşama 1):
- **Yeni `code/FontSystem.cpp` + `FontSystem.h`** — orijinal font alt
  sisteminin taşınabilir çekirdeği (10 FUN → FUN_MAP'a eklendi):
  - `FontSystemInit` (`FUN_004cd660`) — "fonts/" yol tamponu + **64×6
    çeyrek-dörtgen indeks tablosu** (binary döngüsü birebir: `{b, b+2, b+3,
    b, b+3, b+1}` × 64 blok) + liste başları + dword_66DCC0 yükleyici kaydı.
  - `FontSystemAllocBuffers` (`FUN_004cefd0`) — 0x2400 glyph vertex tamponu.
  - `FontSystemLoad` (`FUN_004cd9a0`) — **`;` yedek ad zinciri** →
    `fonts/<ad>.met`; başlık METRICS1/METRICS2/METRICS3 kontrolü.
  - `ParseMetrics1` (`FUN_004cdbc0`) — tam çözüm: doku başlığı (tek token →
    `"%sm%s"` varlık yoklamasıyla maske `m<ad>`; iki token → `(ad, maske)`),
    boyut satırı, glif satırları (`%i ×5` → UV = `(piksel+0.5)/doku` —
    flt_5EF624 = 0.5), en-boy `(sağ-sol)/(alt-üst)`, 5 alan dışındaki
    satırlar atlanır, kod >0xFFFF sert hata, kapasite +0x20 büyümesi,
    son satırdan `lineExtent`/`sizeScale`.
  - `FontFinalizeExtMap` (`FUN_004ce000`) — min..max uint16 harita
    (boş = 0xFFFF — binary `or eax,-1` dolgusu) + `extMin/extCount/extTable`.
  - `FontSystemGet` (`FUN_004cee90`) — ad listesi strcmp önbelleği, isabette
    yeniden yükleme yok.
  - `FontSystemTeardown`/`FontSystemShutdown` (`FUN_004cd580`/`FUN_004cef90`)
    — renderpipeline Shutdown'a `__ioterm+62/+67` karşılığı olarak bağlandı.
  - Yerleşim denetimi: `CCFontGlyph=0x20`, `CCFont=0x144`, `CCFontNode=0x0C`
    (32-bit static_assert — binary Win32 düzeni).
- **RenderPipelineInit Adım 2 kısmen restor edildi**: `sub_4CC140` zincirinin
  font üyeleri (`sub_4CEFD0`, `sub_4CD660`) çağrılıyor; zincirin diğer
  hedefleri (sub_4D5450/sub_4D0E70/sub_4D41C0/sub_4CC1F0/sub_4CC020,
  sub_41CDF0/sub_4D6220) TODO olarak daraltıldı.
- **`rwstubs.cpp`**: eksik `RwTextureRead/Destroy/SetRaster/GetRaster`
  eklendi (rwcore.h'de bildiriliyordu; `RwTextureRead` dosya yokluğunda
  NULL → binary davranışı: doku yoksa font yüklemesi başarısız).
- **Yeni `tests/fontsystem_test.cpp`** (5/5 test): METRICS1 + CRLF, UV/aspect,
  ASCII + genişletilmiş harita, `;` yedek zinciri, iki-token başlık, maske
  yoklama (`m<ad>`), METRICS2 reddi, doku hata yolu, Get önbelleği,
  teardown → yeniden başlatma.
- Kapsam dışı kalanlar (bilinçli): METRICS2 (`FUN_004ce0d0`), METRICS3
  (`FUN_004ce870` — vektör konturları) → sonraki aşama; engine freelist
  havuz semantiği (`engine+0x144/148`) → Faz 4.4b. Çizim yolu + ShowText
  bağlantısı bu oturumda **Aşama 2** olarak tamamlandı (aşağıda).
- Doğrulama: 0 hata/uyarı, `link verification OK`, **ctest 5/5**.

Tamamlanan (bu oturum — font sistemi Aşama 2: çizim yolu + ShowText):
- **`sub_4CCC40` (font->drawHandler) tam çözümlendi ve taşınabilir porta
  aktarıldı** (`FontSystem_DrawText`, FUN_MAP'a eklendi):
  - Glif lookup (ASCII + ext harita); bulunamayan kod **atlanır** (ilerleme
    yok — binary `jl`).
  - Çeyrek-dörtgen: birim uzay y ∈ {0,1}; UV sırası birebir
    `(u0,v1)(u0,v0)(u1,v1)(u1,v0)`; ilerleme `pen += aspect + font->spacing`.
  - Batch: doku değişimi veya **63. quad**'ta flush (binary `ebx > 0x17A` =
    378 indeks; 0x2400 tampon = 64 quad × 4 vertex × 36B) →
    `RwIm2DRenderIndexedPrimitive(rwPRIMTYPETRILIST)` (rwstubs'a eklendi).
  - Renk: düz materyal baytları (+0x60..63) veya aydınlatma/degrade yolu
    (+0x64&1) — ambient + ışık × (pen/genişlik), `CCFontMaterial` binary
    saha aralıklarıyla şemalandırıldı.
  - Dönüş: `*pPosX += pen × scale` (binary son `fadd`).
  - Sapma (belgeli): matris yığını (sub_4CC2F0/4CC4D0/4CC480 → engine
    matris stack'i) yerine ekran uzayında doğrudan ölçek; z/rhw sabit.
    UTF-16 (glyphFlags&2) modu Aşama 3'te eklendi (aşağıya bkz.).
- **`sub_4CD7E0` (ölçüm) port edildi** (`FontSystem_MeasureText`):
  `(aspect + spacing)` toplamı × ölçek; eksik glifler atlanır.
- **`CShowText` FontSystem bağlantısı**: `Render` önce `FontSystemGet`
  (varsayılan ad "Arial", `SetFontName` ile değiştirilebilir) — font varsa
  `FontSystem_DrawText` + `'\n'`/kelime sarmalama (`FontSystem_MeasureText`
  ile); yoksa gömülü 5x7 fallback korunur.
- **`CCFont.flags` (+0x0C) → `spacing` (RwReal) olarak düzeltildi** —
  binary bu alanı ilerleme boşluğu float'ı olarak yazar
  (`sub_4CDBC0`: `dword ptr [esi+0Ch], 0`; çizim/ölçümde `fadd`).
- **rwcore.h/rwstubs.cpp**: `RwIm2DRenderIndexedPrimitive` + draw sayaçları
  (`RwStub_GetIm2DRenderCount` / `GetIm2DIndexedCount` / `Reset`) test
  gözlemi için.
- **test genişletildi**: ölçüm (spacing dâhil), düz/NULL materyal çizim
  (vertex pozisyon/UV/renk), aydınlatma yolu (ambient + degrade byte
  hesabı), 70 gliflik batch taşması (63+7 = 2 flush), sahte dokular gerçek
  tahsise çevrildi (`RwTextureGetRaster` dereference edilebilir olmalı).
- Not: `sub_4CC690` çizim değil **binary stream font yükleyicisi** çıktı
  (sub_491700 zinciri; 4 bayt alan okumaları + tip baytı ≥1) → sonraki
  oturumda portlandı (aşağıya bkz.).
- Doğrulama: 0 hata/uyarı, `link verification OK`, **ctest 5/5**,
  win32 sahte SDK `-fsyntax-only -Wall -Wextra` temiz.

Tamamlanan (bu oturum — font sistemi Aşama 3: METRICS3 + UTF-16):
- **`sub_4CE870` (ParseMetrics3) tam çözümlendi ve taşınabilir porta
  aktarıldı** (`FontSystem.cpp`, FUN_MAP'a eklendi):
  - Biçim: başlık `METRICS3`; ad satırı (`%[^\n]`, yerel — port atlar);
    kod satırı = `<1-bayt önek><UTF-8 kod>` — binary `buf[1]'den` 1..6
    bayt UTF-8 (eski 5/6 bayt formları dâhil) çözümleyip pending kodunu
    kurar (önek portta `'`); komutlar tam satır `strcmp`:
    `begin`/`end`/`moveto %f %f`/`lineto %f %f`/`curveto %f×6`/
    `closepath`.
  - Kontur modeli: alt yol zinciri (binary sub_4D0C90 nesnesi + moveto'da
    yeni kontur; port `CCFontContour`/`CCFontPathPoint` — nokta türleri
    0=moveto, 1=lineto, 2=curveto (ham bezier, PostScript sırası)).
  - Glif sonu (`end`): `glyph.data = kontur`, `glyph.aspect = var_A0`
    (son path-op'un ilk float'ı — glifler arası yapışkan, binary quirk),
    doku = 0; kod kaydı ortak `RegisterGlyphCode` (M1 ile paylaşıldı).
  - `[font+0x138] = &sub_4CCAF0` (vektör işleyici) — portta tek yol
    `FontSystem_DrawText`; kontur raster `sub_4D54C0` sonraki blokta
    portlandı (doku'suz gliflerde quad yerine stroke şeridi).
  - `FontFree` artık glif kontur zincirlerini serbest bırakır
    (sub_4D0C20 karşılığı).
- **UTF-16LE metin modu (glyphFlags&2) MeasureText + DrawText'e eklendi**:
  üç işleyicinin (sub_4CD7E0/sub_4CCC40/sub_4CCAF0) hepsinde aynı desen —
  ünite 2 bayt `(p[1]<<8)|p[0]`, imleç signed `<=0` ile durur (0x8000+
  kodlar binary gibi erken çıkar). Sözleşme: geniş modda çağıran UTF-16LE
  geçmeli (CCF_T1 testleri bu yüzden UTF-16LE'ye çevrildi).
- **METRICS2 (`sub_004ce0d0`) karar**: port bloklu — format raster piksel
  taramasına dayalı (sayfa başlığı doku, glif sınırları satır eşleşmesi
  ile taranır; raster lock + varlık yok) → FUN_MAP'te "analiz edildi,
  port bloklu" olarak işaretlendi.
- **test genişletildi**: sentetik `CCF_V3.met` (METRICS3 + UTF-8 'é' kodu
  → geniş tablo); kontur zinciri/nokta türleri/closepath/curveto ham
  koordinatlar, aspect = son op float'ı, UTF-16LE ölçü/çizim (double-NUL
  sonlandırıcı), M3 doku okuyucu çağırmaz, yeniden yükleme.
- Doğrulama: 0 hata/uyarı, `link verification OK`, **ctest 5/5**,
  win32 sahte SDK `-fsyntax-only -Wall -Wextra` temiz.

Tamamlanan (bu oturum — font sistemi Aşama 3b: vektör kontur raster):
- **`sub_4D54C0` (543 satır, vektör kontur raster) çözümlendi ve taşındı**
  (`FontSystem.cpp` `RenderContourGlyph` + `RenderContourStrip`/
  `RenderContourChain`, FUN_MAP'a eklendi):
  - Stroke modeli: kontur kaydı (0x18: base x/y + birim normal + birikimli
    yay uzunluğu) başına vertex çifti — A = base − n×w (tepe), B = base +
    n×w (taban); w = mat[+0x70] strokeWidth + kontur[+0x10] (portta 0);
    normal = (dy, −dx)/len (sub_4CFF40); degrade grad = acc/total
    (var_64 = 1/son kayıt — reciprocal zinciri).
  - Batching: binary TRIANGLEFAN (tip 4) + 0xFE vert flush → port statik
    indeks tamponu ile TRIANGLELIST (strip→list (a,b,c)(b,d,c)); zincir
    [contour+0x14] recursion = alt yol başına ayrı şerit/flush.
  - Renk: düz (+0x60..63) veya aydınlatma (+0x64&1, tepe +0x00/+0x18,
    taban +0x30/+0x48); flags&2 → UV u = uStart + uDelta×grad (tepe
    +0x10/+0x28, taban +0x40/+0x58 — `CCFontMaterial` pad alanları bu
    adlarla açıldı, strokeWidth +0x70 eklendi).
  - `FontSystem_DrawText`: doku'suz glif dala → bekleyen quad flush +
    `RenderContourGlyph` + ilerleme (penX += aspect + spacing); glif
    çizimi penOfseti (`*pPosX + penX×scale`) ile — binary matris yığını
    T(advance) karşılığı.
  - SAPMALAR (belgeli): miter eklem yerine segment normali (sub_4CFF40
    harmanlaması portta yok); bezier parse'da düzleşmek yerine render'da
    sabit 8 alt segment; doku format durumları (1/9/2) atlanır; z/rhw
    sabit (Faz 4.4b).
  - Clone yolu (refcount==0 → sub_4D0440/4D04D0) port ATLANDI (nadir
    paylaşımsız glif senaryosu).
- **test genişletildi**: 'A' kapanışlı üçgen şeridi — 4 çift/8 vert,
  pozisyon/normal/ölçek+kaydırma, düz renk, aydınlatma grad (acc/total
  reciprocal), strokeWidth 0 dejenere, 'é' eğri (8 alt segment → 20
  vert), iki glif → 2 flush, ilerleme semantiği korunur.
- Doğrulama: 0 hata/uyarı, `link verification OK`, **ctest 5/5**,
  win32 sahte SDK `-fsyntax-only -Wall -Wextra` temiz.

Sırada (öneri): Faz 4.4b render pipeline alt sistemleri — `sub_4D5450`
(matris yığını), `sub_4D0E70`/`sub_4D41C0`, `sub_4CC1F0`/`sub_4CC020`
(vertex tamponu/flush), `sub_41CDF0`/`sub_4D6220`.

Tamamlanan (bu oturum — Faz 4.4b pipeline taşınabilir çekirdek):
- **`sub_4CC1F0`/`sub_4CBD20`/`sub_4CC2F0`/`sub_4CC350`/`sub_4CC480`/
  `sub_4CC4D0`/`sub_4CC020` (çekirdek) çözümlendi ve taşındı**
  (`code/RenderPipeline.h/.cpp`, FUN_MAP'a eklendi):
  - Matris yığını: `CCMatrix` = 64B RwMatrix yerleşimi (right/up/at/pos +
    flags); 32 slot sabit dizi (binary lazy sub_510600 tahsisi yerine);
    push = ebeveyn kopyası (sub_4cc2f0), pushCopy = src×ebeveyn
    (sub_50f760 çarpımı — ebeveyn önce uygulanır), identity flags
    |= 0x20003 (sub_4cbd20).
  - `MatrixScaleXY(x,y)` taban sütunlarını (x,y,1) ile ölçekler;
    `MatrixTranslateXY(x,y)` pos += right×x + up×y (RwMatrixTranslate
    semantiği — öteleme ölçekten geçirilir; DrawText sırası
    translate→scale: v(1,1) → (12,23)).
  - `RenderPipeline_Reset` = sub_4cc1f0: derinlik 0, birim matrisler,
    kök matris (dword_66d604) kimlik, z (dword_66d7a0) = 1.0, half-pixel
    (dword_66d7a4) = 0.5, en-boy 1/1, önbellek sıfır.
  - `SetViewWindow` = sub_4cc020 çekirdeği: en-boy çifti (büyük/küçük →
    diğeri 1.0); kamera nesnesi/kanca ([kamera+0x1c] → sub_4cc110)
    portlanmadı.
  - `RenderPipelineInit`'e `RenderPipeline_Reset()` eklendi (sub_4CC140
    zinciri — font adımlarından sonra sub_4CC1F0 karşılığı).
  - Motor bağlı üyeler (belgeli no-op/TODO): `sub_4D5450`/`sub_4D0E70`/
    `sub_4D41C0` (sub_50dbf0 akış kayıtları + vtable+0x144 freelist
    tahsisi — portta PoolAlloc karşılar), `sub_4CC520` (projeksiyon
    param bloğu — port ekran uzayında ölçek kullanır).
  - FontSystem `FillTextVertex` z: 0 → 1.0f (dword_66d7a0 init değeri).
- **yeni test**: `tests/renderpipeline_test.cpp` — reset, push/taşma
  (31), pushCopy (src×ebeveyn + NULL kopya), DrawText sırası
  (translate→scale vs scale→translate), identity, en-boy (yatay/dikey
  baskınlık + sıfır koruması).
- Doğrulama: 0 hata/uyarı, `link verification OK`, **ctest 6/6**,
  win32 sahte SDK `-fsyntax-only -Wall -Wextra` temiz.

Tamamlanan (bu oturum — stream font yükleyici):
- **`sub_4CC690` (FontSystem_LoadFromStream) + `sub_4D11D0`
  (LoadContourFromStream) çözümlendi ve taşındı** (`FontSystem.cpp`,
  FUN_MAP'a eklendi):
  - Akış soyutlaması `CCFontStream` (read/readExact/findChunk —
    sub_50d940/50cd80/50d790); tip etiketi (etiket>>24) ≥ 1; font
    başlığındaki **refCount alanı yol seçer**: 0 → doku yolu (UV 0x10 +
    aspect + flags baytı; doku adları 0x20, ReadTexture; glyph.texture
    = textures[(RwInt8)flags]), ≠0 → vektör yolu (atık 4B + aspect +
    tip; tip==5 → findChunk(0x1A1) + kontur kaydı + post/atık okumalar).
  - Kontur kaydı (sub_4D11D0): 0x10 başlık {sürüm=1, pointCount, flags,
    extra} + pointCount×0x18 hazır stroke kaydı (type/x/y/normal/acc);
    flags&4 → zincir alt kontur. Port kayıtları CCFontContour yol
    noktalarına çevirir (type0 → moveto, diğer → lineto — curveto zaten
    parse'da düzleşmiş; normal/acc render'da yeniden — Aşama 3b ile
    aynı sapma).
  - SAPMALAR (belgeli): doku bağlamı kaydet/geri al (sub_5154C0/5154A0)
    no-op; kontur[+0x10]/node+8 vestigial; doku indeksi [0,3] + texCount
    ≤ 4 + glyphCount ≥ 0 port doğrulamaları (binary doğrulamaz);
    drawHandler = 0 (DrawText tek yol).
- **test genişletildi**: bellek akışı (MemOps) + blob kurucu; doku yolu
  ('A'/'B' iki glif, tek doku, ASCII harita, ölçü 2.25); vektör yolu
  (type-5 kapalı kontur → çizim 8 vert/1 flush — CCF_V3 'A' ile aynı
  geometri); zincir (flags&4 → head+child, child kapanışlı → 6 vert);
  hata yolları (tip baytı 0, kesik akış, sürüm≠1, findChunk yok, doku
  okuyucu fail).
- Doğrulama: 0 hata/uyarı, `link verification OK`, **ctest 6/6**,
  win32 sahte SDK `-fsyntax-only -Wall -Wextra` temiz.

Tamamlanan (bu oturum — kamera bağlamı + projeksiyon param bloğu):
- **sub_4cc020 tam yolu + sub_4cc520 çözümlendi ve taşındı**
  (`RenderPipeline.h/.cpp`, FUN_MAP'a eklendi):
  - `RCCCameraView` — taşınabilir kamera görünümü (projFlag +0x14,
    raster boyut [raster+0xC/10], pencere +0x68/6C, frame LTM);
    `SetViewWindowFromCamera` = raster doğrulama + SetViewWindow
    çekirdeği + aktif kamera önbelleği (dword_66d7cc) + ofset sıfır
    (dword_66d7b8/bc). NULL kamera → önbellek temiz + FALSE; raster
    NULL → FALSE, önbellek korunur (binary loc_4cc109). vtable kancası
    (sub_4cc110 → dword_66d818) motor tarafı — atlantır.
  - `BuildViewMatrix` (sub_4cc520): önbellekli ölçek matrisi
    unk_66d6d8 = (üst × kök) · ölçek(zScale,zScale,1) · ölçek(-w,w,1)
    (w = min(winX2,winY2)) · translate(winX2×zScale×0.5, -winY2×
    zScale×0.5, 0) — sub_510170/510300 kombinasyon tip 2 (tam ölçek /
    ham öteleme — tip 1'ler MatrixScaleXY/TranslateXY'de zaten portlu,
    testlerle ayrıştırılmıştı); zScale = (projFlag==2) ? 1.0 : dword_
    66d7a0. Tersi unk_66d718 (sub_50ff20: kimlik kopya / orthonormal
    transpoze / genel adjugate-det — sub_510010; det==0 → 1.0 —
    flt_5ef2e0). Sonuç unk_66d758 = ölçek × kamera frame — HER ÇAĞRIDA.
  - Küçük bağlam API: GetCamera (sub_4cc120), Set/Clear/GetContext
    (sub_4cc130 — dword_66d79c), GetViewParams (sub_4cbe10 — ofset +
    en-boy). Reset'e eklendi: kamera/ofset/dword_66d7b0=7 + view
    matris init.
  - SAPMALAR (belgeli): sub_4cbee0 (dünya→ekran dönüşüm tüketici) ve
    sub_4cbe50 (piksel ölçek metriği flt_66d7a8) portlanmadı — sonraki
    iş kalemi); kamera/kare yoksa ölçek matrisi döner (binary kilitlenir).
- **test genişletildi**: kamera bağlamı (640×480, win 1/0.5 → ofset 0,
  en-boy 2/1; NULL/raster-NULL yolları — önbellek korunma); BuildViewMatrix
  matematiği (temiz yığın: ölçek(-1,1,1)+translate(1,-0.5,0); ters
  pos'(1,0.5,0); frame çarpımı pos(-9,19.5,30)); önbellek geçerlilik
  (MatrixTranslateXY → yeniden kurulum); kare'siz koruma; context
  set/clear.
- Doğrulama: 0 hata/uyarı, `link verification OK`, **ctest 6/6**,
  win32 sahte SDK `-fsyntax-only -Wall -Wextra` temiz.

Tamamlanan (bu oturum — Faz 4.6: Bullet physics backend, "en yakın his"):

- **geçiş dokümanı**: `docs/HAVOK_TO_BULLET.md` — neden Bullet, binary
  reverse kaynakları (`sub_55E6D0` alan tablosu, 1/60 adımı), eşleme
  tablosu, sahiplik sözleşmeleri, sapmalar ve açık boşluklar tek yerde.

- **karar**: Havok yerine **Bullet Physics 3.25** (MIT; FetchContent ile
  `code/CMakeLists.txt`'e vendor'landı — yalnız LinearMath/Collision/
  Dynamics derlenir, demo/unit-test opsiyonları kapalı). Gerekçe: hkpWorld
  → btDiscreteDynamicsWorld, hkpRigidBody → btRigidBody, hkpPhantom →
  btGhostObject (+btGhostPairCallback), Clump → btBvhTriangleMeshShape /
  btConvexHullShape, step(1/60) → stepSimulation(dt,1,dt); Havok memory
  arena → standart malloc.
- **binary parametreleri reverse edildi** (en yakın his için):
  - `sub_55E6D0` (Havok memory callback kaydı `sub_55E630` → dword_660D58
    üzerinden; dword_660920 arena context ile uyumlu): [+0x04] gravity
    **-9.8** (0C11CCCCDh), [+0x20..28]/[+0x30..38] AABB **±500**
    (0C3FA0000h/43FA0000h), [+0x44] **friction 0.6**, [+0x58]
    **restitution 0.3**, [+0x80] **solver iter 20**, [+0x5C] 0.0163,
    [+0x8C] 10.0; [4B8710:7E] sabit adım **1/60** (push 3C888889h).
  - Bunlar PhysicsBackend katsayılarına girdi: kGravityY=-9.8,
    kFriction=0.6, kRestitution=0.3, kSolverIterations=20.
- **`PhysicsBackend.h/.cpp`** (yeni): PhysicsWorldHandle (config+
  dispatcher+dbvt+impulse solver+world+ghost callback); Create/Destroy/
  Step/Set·GetGravity; PhysicsBody_CreateBox/TriMesh/WithShape + Destroy;
  PhysicsShape_CreateBox/Sphere/ConvexHull/TriMesh + Destroy (ShapeBundle
  registry — mesh shape'lerinde btTriangleMesh ömrü shape ile); trigger
  (CreateBox ghost + Add/RemoveTrigger SensorTrigger grubu + GetOverlaps);
  pozisyon/dönüş/hız/kinematic/impuls erişimi. Sahiplik: CreateWithShape
  body shape'i SAHİPLENMEZ (userIndex=1 → Destroy shape'i silmez).
- **`PhysicsWorld.cpp`** backend'e bağlandı: ctor → dünya oluştur
  (gravity-9.8/iter20), Update → stepSimulation(1/60), Add/Remove
  RigidBody/Phantom → backend çağrıları.
- **`CollisionGeometry.cpp`** implemente edildi (FUN_0046f040):
  RpClumpForAllAtomics → vertex/triangle toplama (base offset'li;
  rpGEOMETRYTRISTRIP → alternating winding), bConvex → ConvexHullShape,
  !bConvex → TriMeshShape; CreateBox/Sphere/DestroyShape backend'e
  devredildi. rwcore/rpworld'a eklendi (stub — librw'de dolacak):
  RpGeometryGetNumVertices/NumTriangles/Vertices/TriangleIndices/Flags.
- **test**: `tests/physics_test.cpp` (7/7) — gravity varsayılan (-9.8),
  1 sn serbest düşüş (v=-9.8, semi-implicit Euler y doğrulaması), zemin
  çarpışması + sürtünme durması (friction 0.6), impuls (Δv=J/m), trigger
  overlap (ghost callback), statik triMesh zemin üzerinde oturma.
- Doğrulama: Bullet 3.25 + oyun kodu **0 hata**, `link verification OK`
  (force_load + BulletLib), **ctest 7/7**, win32 sahte SDK `-fsyntax-only`
  temiz. CMake notu: Bullet include'ları hedeflere yayılmaz → elle
  `target_include_directories(... ${bullet3_SOURCE_DIR}/src)`;
  `CMAKE_POLICY_VERSION_MINIMUM 3.5` (Bullet'ın eski cmake şartı).
- SAPMALAR (belgeli): sequential-impulse solver ≠ Havok 2004 solver
  (birebir his garanti edilmez — gameplay kalibrasyonu asset'lerle);
  AABB ±500 broadphase clamp'i uygulanmadı (Bullet geniş aabb); Clump
  atomik transform'u uygulanmıyor (noinstance varsayımı); deactivation
  varsayılanları (Havok cinfo'daki +0x6C/+0x2 alanları henüz eşlenmedi).
  Not: binary'de `hkp*` sembolü yok (istatik link) — parametreler
  içerik analiziyle bulundu.

Tamamlanan (bu oturum — Faz 4.6b: entity/trigger akışı backend'e bağlandı):

- **`PhysicsEntity.cpp` erişimcileri** dolduruldu (Set/GetPosition,
  Set/GetLinearVelocity → PhysicsBackend; m_pRigidBody null-guard; IDA
  kanıtı: sub_436F70+472 `m_pRigidBody != NULL Failed` assert'i).
  AddToWorld/RemoveFromWorld singleton üzerinden zaten kayıtlıydı —
  backend dünya kaydı PhysicsWorld üzerinden otomatik akar.
- **`CollisionTrigger.cpp`** implemente edildi: Initialize
  (FUN_0046f5e0) → `RpClumpGetBoundingSphere` (yeni stub — rwcore/
  rpworld'a eklendi, librw'de dolacak) → AABB kutu phantom
  (`InitializeWithSphere` port eklentisi — test/clump'suz yol) →
  `CPhysicsWorld::AddPhantom` → backend ghost. `HandleEvents` →
  `PollOverlaps()`: önceki turla fark → `OnEntityEnter`/`OnEntityLeave`
  (virtual; FUN_004702c0 speak tetiklemesi speak manager portlanmadığı
  için no-op — alanlar m_iEnter/LeaveSpeak* korunuyor). Dtor phantom'ı
  dünyadan çıkarır + yok eder.
- **test**: `tests/physics_entity_test.cpp` (**ctest 8/8**) — entity
  yaşam döngüsü (AddToWorld → exists; Set/GetPosition 4,5,6; Set/Get
  velocity 7,8,9; Update sonrası gravity + vx korunumu; RemoveFromWorld);
  trigger enter/leave (yarıçap 3 AABB, giriş 1×, tekrar giriş yok,
  teleport sonrası leave 1×).
- Doğrulama: 0 hata, `link verification OK`, **ctest 8/8**, win32 sahte
  SDK `-fsyntax-only` temiz.

Tamamlanan (bu oturum — Faz 4.6c: singleton yaşam döngüsü binary'ye hizalandı):

- **singleton sahipliği netleşti** (binary `sub_41A680` = RwStartup::
  Initialize): HavokInit (`sub_434FB0`) physics world singleton'ını
  YAZMAZ (dosya notu doğrulandı); onu startup init zinciri yaratır
  (alloc 0x440 → ctor `sub_49C870` → `dword_635AF4`); behaviour ctor'ı
  (`sub_43B860`) yalnız container tahsis eder — singleton'ı KULLANIR.
- **`CPhysicsWorld::Create()`** eklendi (mevcut varsa döner — ikinci new
  CSingleton assert'i tetiklemez); `startup.cpp`'de HavokInit sonrası
  çağrıldı (binary adım eşlemesi). **Behaviour ctor/dtor sahiplik
  bayrağı** (`m_bOwnsWorld`): singleton startup'tan geldiyse dtor silmez
  (shutdown zinciri lifespan sahibidir); seviye/test yolunda yoksa
  oluşturur ve siler.
- **test**: `physics_entity_test` genişletildi — Create() idempotent;
  behaviour ikinci dünya yaratmaz; dtor sahip değilken dünya yaşar;
  delete sonrası singleton null (**ctest 8/8**).
- `CPhysicsResource::Load` (IDA `sub_439460`) incelendi: pacType/pStream
  assert'leri + `wbID_STATICWORLDCOLLISION` chunk yolu — stream parse
  RwStream API'sine (kapı 4/librw) bağlı, bu adımda sayaç/assert seviyesinde
  kaldı (belgeli).
- Doğrulama: 0 hata, `link verification OK`, **ctest 8/8**.

Tamamlanan (bu oturum — Faz 4.6d: **librw entegrasyonu — kapı 4 açıldı**):

- **librw** (github.com/aap/librw, MIT) CMake FetchContent ile bağlandı:
  commit `18532c2`, `LIBRW_PLATFORM=NULL` (cihaz bağımsız — stream/clump
  parse; GL3/D3D9 render sonraki iş kalemi), `librw::librw` hem
  CannonCruiseLib hem CannonCruise'e link'li.
- **shim mimarisi**: `code/rw_librw.cpp` — oyunun C API'si (Rw*/Rp*) →
  librw C++ (`rw::`). `rwstubs.cpp`'den taşındı: engine yaşam döngüsü
  (init/open/start/stop/close/term + IsRunning durum takibi), RwStream*
  (rw::StreamFile + findChunk/readChunkHeaderInfo), RpClumpStreamRead
  (rw::Clump::streamRead), RwTexDictionaryStreamRead (rw::TexDictionary::
  streamRead). RpWorldStreamRead stub'da KALDI — librw'de World streamRead
  yok (BSP riski belgeli).
- **`rwID_TEXDICTIONARY` düzeltildi**: port'ta 0x0023 yazılıp klasik RW3
  0x0016 idi (çift tanımdan tek sabite indirildi; librw VEND_CORE ile
  aynı).
- **`tests/rwlibrw_test.cpp`**: engine init(NULL) → gerçek `Streams/MAINMENU.
  RWS` açılır → chunk başlığı döngüsü + envanter + FindChunk →
  **ctest 9/9**.
- **asset format bulguları** (MAINMENU/GLOBAL/VIKING01 hexdump):
  - Dosyalar **standart RW stream başlığı** kullanır (12 bayt; libraryID
    `0x1802FFFF` → `libraryIDUnpackVersion` = **RW 3.6.0.2** ✓ librw decode).
  - Chunk türleri **oyun plugin aralığında**: `0x0716` (asset/txd tanımlayıcı
    — payload'da length-prefixed "Texture Dictionary" + "rwID_TEXDICTIONARY"
    dizesi + kaynak yol metadatası: `x:\rwstudio\wavebreaker\build output\...`),
    `0x0704` (bileşen/behaviour kaydı — class adı dizesi, ör.
    **"CWbRenderBehavior"** — "Wb" = WaveBreaker bileşen adlandırması),
    `0x071C` (dosya başına 1× 104 bayt — manifest/index). VIKING01: 617
    chunk (0x0704=497, 0x0716=119, 0x071C=1).
  - Asset pipeline'ı **"rwstudio"** özel aracı (RW SDK stream writer DEĞİL);
    wbID_* plugin'leri muhtemelen bu 0x07xx ID'leri (CPhysicsResource::
    Load `wbID_STATICWORLDCOLLISION`).
- `RwStreamRead/Write` da shim'de tanımlandı (daha önce decl-only idi).
- Doğrulama: 0 hata, `link verification OK`, **ctest 9/9**.

Tamamlanan (bu oturum — Faz 4.6e: **rwstudio konteyner formatı terse
edildi**):

- **`code/RwStudioStream.h/.cpp`**: `Streams/*.RWS` = rwstudio (WaveBreaker
  pipeline) özel konteyneri — üst seviye RW chunk'ları (libid 0x1802FFFF =
  RW 3.6.0.2): `0x0716` asset kaydı (meta: typeName/GUID/rwIdName/
  sourcePath; string = uzunluk önekli, '\0'+0xBF doldurma 4 hizalı),
  `0x071C` manifest (sınıf adı+sürüm — CWb* behaviours), `0x0704` nesne
  kaydı (property listesi — sonraki iş kalemi).
- **gömülü RW chunk**: txd asset'lerinde meta sonrası standart chunk
  başlığı `[type=0x23][len][libid 0x1803FFFF]` + payload — **klasik RW
  konvansiyonu** (len = payload boyutu); kimlik: `metaLen+20+len ==
  chunkLength`. `rwID_TEXDICTIONARY` bu dosya ailesinde **0x23** (0x16
  değil — rwcore.h'deki düzeltme geri alındı/belirtildi).
- **varyantlar** (hexdump doğrulandı): `rwID_2DMAESTRO` (.anm — 4. string,
  marker'sız meta; Rt2dMaestro librw boşluğu — chunk atlanır),
  `wbID_WORLDGEOMETRY` (.dff, embed yok), `wbID_STATICWORLDCOLLISION`
  (.col, embed yok — inline veri CPhysicsResource::Load hedefi),
  `wbID_PATHFINDINGDATA`, `wbID_Texture` (.texref: [res][len][pngPath] —
  ör. "Font\SpecialCharacters.png"), PPB ses bankası ("Bank Win32").
  Parser **seek-tabanlı** (byte-sayımı yok): her chunk sonuna mutlak seek
  — format varyantları yorum gerektirmez. Yeni shim: `RwStreamTell` /
  `RwStreamSeekAbs`.
- **envanter** (testle sabit): MAINMENU 5 asset/4 nesne/1 manifest (txd
  1.6MB + Menu.anm maestro + .dff/.col/.path); GLOBAL 7 asset/48 nesne
  (1 gömülü txd 525KB); VIKING01 119 asset/497 nesne/1 manifest.
- **`tests/rwstudio_test.cpp`**: meta alanları + gömülü txd payload
  (i16 doku sayısı ≥1, device=1 d3d8) + manifest sınıf adları
  (CWbRenderBehavior/CMenuBehavior) + üç dosya envanteri → **ctest 10/10**.
- Not: MAINMENU son asset'i `rwID_2DMAESTRO` "Menu.anm" (menü Maestro
  animasyonu) — Rt2dMaestro portu kapı 5 riski olarak işaretlendi.

Tamamlanan (bu oturum — Faz 4.6f: **0x0704 nesne property şeması terse
edildi**):

- **şema** (MAINMENU obj0/1/2 hexdump ile sabitlendi): nesne =
  `[u32 field0]` + segment/property dizisi.
  - segment `[u32 segLen][u32 flags][içerik]`: `0x20000000` = sınıf adı
    (behaviour başlangıcı), `0x40000000` = GUID(16B), `0x80000000` =
    ilk behaviour'da ad tekrarı / SONRAKİ behaviour'ları başlatır
    (GUID'suz), `0x00000000` = string değer segmenti (ör.
    `"Audio_MenuMusic"` — CAudioStreamEntity adı).
  - property `[u32 size][u32 id][size-8 değer]`: size 12 = u32/float/
    inline-string (`\0`+0xBF pad); size 20 = 3 float (vec3 — ör.
    CWbCameraBehavior prop0 = [0.0, 2.0, -1.0]).
- **kanıt**: CWbRenderBehavior 13 property (prop0=-1, prop1=1000.0f,
  prop2=0.5f, prop9=300.0f, prop0xC=30.0f — render ayarları;
  binary ctor `sub_43B860` ile karşılaştırılabilir); CWbCameraBehavior
  (prop4=60.0f — FOV); CAudioStreamEntity `"Audio_MenuMusic"` + 5
  property (prop4=5.0f); her nesne CAttributeHandler ile biter.
- **kod**: `RwStudioStream.h`'e `RwStudioObject/Behaviour/Property`
  struct'ları; NextAsset 0x0704'ü parse eder (state machine: 0x20→GUID→
  0x80 tekrar→props; sonraki 0x80→yeni behaviour); erişim:
  `RwStudioFileGetObject()`.
- **test**: MAINMENU obj0/1/2 detay assertions (sınıf adları, prop
  sayıları, float değerleri, string segment) + VIKING01 ilk nesne
  sağlamlığı → **ctest 10/10** (rwstudio_test 58+ checks).
- Doğrulama: 0 hata, `link verification OK`, **ctest 10/10**, win32
  syntax temiz.

Sırada (öneri): property id'leri binary sınıf düzenine eşlem
(CWbRenderBehavior ctor `sub_43B860` field offset'leri — prop9=300/
prop0xC=30 kamera mesafe/FOV olabilir); CClassFactory ile sınıf adı →
constructor kaydı; gömülü txd payload decode (d3d8 IMAGE); .col inline
veri → trimesh; render platformu (GL3/D3D9).

### Faz 4.6g: binary onayı — CClassFactory + RenderBehaviourWorld
(hex-rays `CannonCruise.exe.c` + `CC_ida.asm` + PE vtable dump'tan)

- **CClassFactory::Register = `FUN_00413700`(name, factory, size,
  flag)** — kayıtlı sınıf boyutları: CWbRenderBehavior 0x18,
  CWbCameraBehavior 0x34, CAudioStreamEntity 0x54,
  CPhysicsWorldBehaviour 0x20. `FUN_0043b860` DÜZELTİLDİ: render ctor
  değil, CPhysicsWorldBehaviour **factory**'si.
- **CEventHandler**: ctor `FUN_00411010`(stream, className); process
  döngüsü `FUN_00411050` — 0x20/0x40 kayıtlar skip, **0x80000000 kaydı
  dispatch** (bizim parser'daki "ikinci sınıf adı segmenti" = behaviour
  uygulama tetikleyicisi — parser state machine'i binary ile tutarlı).
- **RwID yükleme tablosu `DAT_0065d7a4`**: isimle stream'den chunk
  yükleyen global sistem (+0x114 slot: rwID_CLUMP/2DMAESTRO/
  HANIMANIMATION yüklemeleri) — gömülü payload okuma orijinali.
- **RenderBehaviourWorld singleton `FUN_004c4410` → DAT_00635648**:
  dev nesne (pool'lar 120000B+180000B+...); **CWbRenderBehavior sub-
  object +4'te**; default'lar [0x14]=1000.0f, [0x15]=300.0f = obj0
  prop1/prop9 ile birebir eşleşti (0x0704 değerleri bu singleton'a
  yazılıyor); **12 mesaj→kamera pass** kaydı: MENU_RENDER_RT2D /
  FRONT_RT2D / IM2D, PROFILE, SHOWTEXT, THUNDER1, FADE,
  EnemyHealthBar (hepsi RwCamera), Ocean update/destroy (COcean),
  Event_INQ_PLYR_POSITION (RwMatrix). → Port mimarisi: **mesaj güdümlü
  kamera pass sistemi** (kod `RenderBehaviorWorld`/`MessageSystem`).
- Orijinal kaynak yolu hex-rays'ten: `D:\Projects\CannonCruisePC\code\`
  (proje adı birebir).
- Doğrulama: build 0 hata, ctest 10/10, link OK, win32 syntax OK.

Kalan iz sürme (sonraki oturum): property apply fonksiyonu (stream'den
[id][value] okuyup singleton offset'ine yazan — 0x80000000 dispatch
hedefi); CWbRenderBehavior prop id ↔ singleton offset tablosu;
CClassFactory create-by-name akışının portu (CEventHandler ile
birlikte).

### Faz 4.6h: RenderBehaviourWorld property apply portu (TAMAM)

- **`FUN_004c3e40` = WbRender attribute apply** hex-rays'ten çözüldü:
  switch(id) 0..0xC; değer `rec+8` (raw u32); id 6/0xA = ARGB → R,G,B
  **baytları float olarak** decode (+0x30../+0x44..); default →
  assert "Unhandled attribute in WbRender..." (kaynak satır 0xC0).
- **id → singleton offset haritası** (byte): 0→+0x14, 1→+0x18,
  2→+0x1c, 3→+0x5c, 4→+0x60, 5→+0x28, 6→+0x30/34/38, 7→+0x2c,
  8→+0x50, 9→+0x54, 0xA→+0x44/48/4c, 0xB→+0x3c, 0xC→+0x40.
  Ctor default'ları ile tutarlılık: id8=1000.0f/+0x50, id9=300.0f/+0x54
  (ctor [0x14]/[0x15] kelimeleri).
- **kod**: `code/RenderBehaviourWorld.h/.cpp` — 32-bit binary layout
  `RwUInt32` vtable alanıyla birebir (static_assert'lı, +0x04'te 24B
  CWbRenderBehavior sub-object); `Init()` ctor default'ları;
  `ApplyWbRenderAttribute(id, value)` sadık switch.
- **test**: `tests/renderworld_test.cpp` — engine init shim; gerçek
  MAINMENU obj#0 13 property'si apply → tüm alanlar doğrulandı
  (prop0=0xFFFFFFFF bit kopyası, prop1=1000.0f→+0x18, prop6=0xFF000000
  →RGB(0,0,0), propA=0x00FFD0FF→RGB(255,208,255), propC=30.0f;
  id>0xC → UNHANDLED). **59 checks, 0 failures.**
- Doğrulama: 0 hata, **ctest 11/11**, link OK, win32 syntax OK.

Sırada (öneri): CWbCameraBehavior apply switch'i (FUN_004c2b90 ailesi);
CAudioStreamEntity property'leri; CEventHandler 0x80000000 dispatch
(create-by-name akışının portu); gömülü txd decode; .col → trimesh;
render pass sistemi (MSG_MENU_RENDER_RT2D vb.); Rt2dMaestro.

### Faz 4.6i: CWbCameraBehavior property apply portu (TAMAM)

- **`sub_4C3450` = kamera apply** (vtable PTR_LAB_005f0ac4 slot1; ASM
  jump-table'dan çözüldü — hex-rays bu fonksiyonu bölünmüş parçalara
  ayırmıştı): **51 case** (id 0..0x32), hedef = `sub_452170()` kamera
  durum nesnesi (≥0x140 bayt), default **sessiz** ilerler (assert yok).
- **Yapı**: 4 parametre grubu (A: id0-7, B: id8-16, C: id17-27,
  D: id28-40) — her grupta VEC3 (12B kopya), RAD (v·π/180), TAN
  (tan(v·π/180) — projeksiyon ölçeği), raw u32 alanlar; + misc
  id41-50. Sabitler: `flt_5F0AC0`=π (0x40490FDB), `flt_5EF7B8`=1/180,
  `flt_5EF2E0`=1.0f. id42: guard(+0x13c)==0 ise +0x114 += 1.0;
  id48: guard==0 ise +0x12c = 1.0f.
- **kod**: `code/CameraState.h/.cpp` — `CCameraState` (0x140B, 32-bit
  layout static_assert'lı); `ApplyAttribute(id, bits)` (RAD/TAN/raw)
  + `CCameraStateApplyVec3(id, 12B)`; `CameraDegToRad` π·(1/180)
  sırasıyla (binary fmul sırası).
- **test**: `tests/camerastate_test.cpp` — gerçek MAINMENU obj#1
  (CWbCameraBehavior) tüm property'leri apply → dosya değerleri +
  tan(60°)=1.7320508 doğrulandı; guard davranışları; bilinmeyen id
  FALSE → **86 checks, 0 failures**.
- Doğrulama: 0 hata, **ctest 12/12**, link OK, win32 syntax OK.

Sırada (öneri): CAudioStreamEntity apply (FUN_004587d0 ailesi); CAttributeHandler paketleri; CEventHandler 0x80000000 dispatch
(create-by-name akışı); gömülü txd decode; .col → trimesh; render
pass sistemi; Rt2dMaestro.

### Faz 4.6j: CAudioStreamEntity property apply portu (TAMAM)

- **`sub_458BD0` = audio apply** (vtable PTR_FUN_005efbe8[1] / sub
  PTR_LAB_005efbe0[3]): **6 case** (id 0..5), default sessiz.
  - id0: `sub_458D10(this,1)` stop (+0x3c/+0x48=0) + slot20 temizle;
    rec+8 ilk baytı != 0 ise slot20'ye "SAudioPlayStreamData*"
    payload register (loc_40BF90).
  - id1/id2: `sub_40E0B0(this=+0x18, slot=+0x28/+0x30, src=rec+8, 0)` —
    slot detach→clear→register→list-attach (flags=0).
  - id3: +0x38 = u32 (dosya: 0xFFFFFFFF = -1).
  - id4: +0x40 = float (dosya: 5.0f).
  - id5: +0x44 = float (dosya: 0.0f).
- **Layout** (ctor FUN_00458960, 0x54B): +0x00 vtable, +0x18
  sub-vtable, +0x1c=1, +0x20/+0x28/+0x30 üç 8B mesaj slotu (FUN_0040b300),
  +0x38..+0x48 alanlar; ctor FUN_0040d170 ile DAT_00635c68/635c50
  listelerine 0x8000 ile bağlanır.
- **kod**: `code/AudioStreamEntity.h/.cpp` — `CAudioStreamEntity` struct
  (static_assert'lı 0x54 layout); `CAudioStreamEntityInit`,
  `CAudioStreamEntityApplyAttribute(id, pValue)`. Slot öneği NUL'a kadar
  max 8 bayt; "kayıtlı" durum binary'de slot[1] flag'inde, portta önekin
  ilk baytından türetilir (0x00 = boş); tam slot makinesi mesaj alt
  sistemi (todo).
- **test**: `tests/audiostream_test.cpp` — MAINMENU obj#2
  ("Audio_MenuMusic", 5 prop id1..5) apply → slot önekleri, loopCount
  0xFFFFFFFF, float0=5.0f, float1=0.0f; id0 stop+register; bilinmeyen id
  FALSE → **48 checks, 0 failures**.
- Doğrulama: 0 hata, **ctest 13/13**, link OK, win32 syntax OK.

Sırada (öneri): CAttributeHandler paketleri; CEventHandler 0x80000000
dispatch (create-by-name akışı); gömülü txd decode; .col → trimesh;
render pass sistemi; Rt2dMaestro.

### Faz 4.6k: BehaviourLoader create-by-name dispatch (TAMAM)

- **Pipeline kapatıldı**: 0x0704 OBJECTRECORD → sınıf adı (0x20/0x80000000
  segmenti) → `CClassFactory::Create` → create → apply virtual (vtable
  slot1) — render/camera/audio apply portları uçtan uca bağlandı.
- **Binary semantiği** (FUN_00411010/1050/10e0): CEventHandler ctor
  (stream, className) **hemen Process** çalıştırır; Process 0x20/0x40
  atlar, 0x80000000'de `(*(DAT_0065d7a4+0x114))(className, rec+8)` ile
  sınıf adı karşılaştırır (strcmp, 0=eşleşme), eşleşen behaviour'ın ilk
  property'sinde durar; eşleşmeyen property'ler atlanır; FUN_004110e0 =
  Advance+Process(1) sonraki behaviour'a geçer. FUN_00407210 aynı +0x114
  strcmp slotunu isim tablosu aramasında da kullanır (DAT_0063557c list).
- **kod**: `code/BehaviourLoader.h/.cpp` — `BehaviourClassEntry`
  (className/size/create/apply), `BehaviourLoader::Register`
  (CClassFactory::Register + apply tablosu), `Create` (fabrika köprüsü),
  `ProcessObject` (behaviour başına create+apply; bilinmeyen sınıf
  atlanır — factory NULL).
- **test**: `tests/behaviourloader_test.cpp` — üç sınıf kaydı
  (boyutlar 0x18/0x34/0x54), MAINMENU 4 OBJECTRECORD: obj#0 render
  singleton alanları (prop0 bit kopyası, idC=30.0f, idA RGB 255/208/255),
  obj#1 kamera (VEC3 pos dosya değeri, tan(60°)), obj#2 audio
  (loopCount 0xFFFFFFFF, float0=5.0f, float1=0), obj#3 bilinmeyen
  sınıf → 0 apply → **31 checks, 0 failures**.
- Doğrulama: 0 hata, **ctest 14/14**, link OK, win32 syntax OK.

Sırada (öneri): gömülü txd decode (d3d8 IMAGE); .col → trimesh;
render pass sistemi (MSG_MENU_RENDER_RT2D vb.); CAttributeHandler
paketleri; Rt2dMaestro.

### Faz 4.6l: Gömülü TXD (d3d8 IMAGE) decode (TAMAM)

- **Format tersine edildi** (MAINMENU.RWS gömülü txdsi, 1,685,444B,
  libId=0x1803FFFF): başlık `[u16 textureCount][u16 deviceID]`; her
  texture: `[u32 flag=1]` + `0x18` raster (nested `[id=1][16B: w,h,depth,
  pitch]` + piksel `pitch*height` B, 4-aligned) + `0x06` struct (nested
  `[id=1][4B filterFlags]`, `[id=2][name]`, `[id=2][maskName]`, gövde
  kuyruğunda `[id=3][0][ver]` terminator).
- **İçerik**: 16 texture, deviceID=1 (d3d8) — 11×128×32 animasyon karesi
  (Menu_137..117), 3×32×32 (Menu_095/087/084), 512×128 şerit (Menu_076),
  640×480 arka plan (Menu_074); tümü 32bpp; ilk piksel 0x00FFFFFF.
- **kod**: `code/TxdDecode.h/.cpp` — `RwTxdTexture/RwTxdImage` struct,
  `RwTxdDecodeImage(payload, len, out)` (chunk/nested sınır doğrulamalı;
  piksel işaretçileri payload içi).
- **test**: `tests/txddecode_test.cpp` — gömülü payload okuma
  (RwStudioAssetReadEmbedded) + decode → 16 texture, isimler (Menu_137 →
  Menu_074), boyutlar (128×32 / 512×128 / 640×480), pitch*height
  tutarlılığı, piksel aralığı, bozuk girdi FALSE → **32 checks, 0
  failures**.
- Doğrulama: 0 hata, **ctest 15/15**, link OK, win32 syntax OK.

### Faz 4.6m: Statik dünya çarpışma (.col) decode + trimesh (TAMAM)

- **Format tersine edildi** (0x0716 wbID_STATICWORLDCOLLISION payload'ı;
  ctor FUN_0043bfa0 + section sub_58BEF0 asm): payload düzeni
  `[u32 vertexCount][u32 bilinmeyen][u8 flag=2 + pad][u32 12]
  [u32 triangleCount][12B sıfır][f32 bboxMin.xyz][f32 bilinmeyen]
  [u32 sectionBytes(f32 bit deseni)]` + section blob (materyal/primitif,
  opak) + `u32 indices[triCount×3]` + `f32 verts[vcCount×3]` + 0–4B pad.
  Ctor başlığı asset'in embA alanından (payload−8) başlar; count1=[+8]
  (=vertexCount), flag bayt=[+10], count2=[+18] (=triangleCount).
  Boş dünya (MAINMENU vb.): countA=0 → arraysiz skip → decode FALSE.
- **İçerik**: VIKING01 38391v/71303t (section 753474, bboxMin
  −475.77/−9.08/−333.67), IDUNA03 6726/11592, BEAVER01 55902/102833 —
  üçünde de layout byte-sınırı slack 0/1/2; indeksler 0..vc-1.
- **kod**: `code/StaticCollisionData.h/.cpp` —
  `StaticCollisionData_Decode(payload, len, mesh)` (sınırsal doğrulama +
  indeks kontrolü) + `StaticCollisionData_CreateShape(mesh)` →
  `PhysicsShape_CreateTriMesh` (vc×float3 + tri×u32); RwStudioStream'e
  `resourceLength` + `RwStudioAssetReadResource` eklendi (embed olmayan
  resource payload okuma — .col yolu).
- **test**: `tests/coltrimesh_test.cpp` — üç gerçek RWS'ten payload
  okuma + decode (sayı/bbox/slack/ilk indeks) + Bullet shape üretimi +
  bozuk girdi FALSE → **53 checks, 0 failures**.
- Doğrulama: 0 hata, **ctest 16/16**, link OK, win32 syntax OK.

Sırada (öneri): aşağıdaki **Kritik Yol** planı — render platformu →
WorldGeometry instantiate → attribute apply → döngü entegrasyonu.

---

## Kritik Yol: RWS → Çalışan Dünya (Plan)

Hedef: `Streams/VIKING01.RWS` açılıp dünyanın ekranda görünmesi, çarpışmanın
Bullet'te aktif olması, menü/akış zincirinin kurulması. Mevcut varlıklar:
RwStudioStream (konteyner parse), StaticCollisionData (trimesh), Bullet
backend, RenderPipeline (Im2D matris yığını), RenderBehaviourWorld pass
singleton iskeleti, CollisionGeometry (Clump→shape), CameraState, librw
(stream shim; engine init testlerde ayakta).

### Faz 4.6n: Render platformu + temel sahne (librw) — TAMAM

- `RenderPlatform.h/.cpp`: librw C++ API sahne kurulumu — World (BBox
  ±500), perspektif kamera (near 0.1/far 5000, viewWindow 1×1), ambient
  + directional ışık; kare zinciri clear(IMAGE|Z) → BeginUpdate →
  RenderPipeline_Reset + SetViewWindowFromCamera (RCCCameraView — frame
  LTM CCMatrix yerleşimi) → uygulama çizimi → EndUpdate → (frameBuffer
  varsa) ShowRaster. Im2D test karesi: ScaleXY+TranslateXY üst matrisi
  2B afin uygulanır, RwIm2DVertex (gl3::Im2DVertex ile aynı yerleşim).
- main.cpp: link doğrulaması + 3 kare platform smoke (`./CannonCruise`
  → "3/3 frames OK"); NULL platform headless (device no-op); GL3
  build'de gerçek piksel (frameBuffer nil ise ShowRaster atlanır).
- **Doğrulama**: ctest **17/17** (renderplatform_test: kurulum/idempotent/
  3 kare/clear/terminate/yeniden kurulum/kurulu değilken FALSE); link
  OK; win32 syntax OK.
- **Sonraki**: 4.6o — WorldGeometry instantiate (clump/BSP → sahne +
  trimesh zemin).

### Faz 4.6o: WorldGeometry instantiate — TAMAM

- **Format çözüldü** (binary FUN_004c9b70 + hexdump): payload =
  `[u32 prefixSectionLength][u32 libid=0x1803FFFF]` + prefix (STRUCT(16:
  master u32×4) + GEOMETRY + EXTENSION; preLen = ilk ATOMIC ofseti) +
  **ATOMIC × N** (her biri STRUCT(16: artifact,0,flags=5,0) + GÖMÜLÜ
  GEOMETRY (standart RW, d3d8-native değil — TEXTURED2|PRELIT) +
  EXTENSION) + trailing chunk (dünya varyantı: VIKING01=0x22,
  IDUNA03=0x1E, BEAVER01=0x39). Binary "sektörler" = ATOMIC (assert
  type==0x14; librw ID_ATOMIC=0x14).
- `WorldGeometry.h/.cpp`: `WorldGeometry_Inspect` (yapısal gezinme +
  envanter), `WorldGeometry_CreateClump` (StreamMemory +
  Geometry::streamRead — **VIKING01 96/96 atomic**, 69657 verts;
  d3d8 IMAGE MATLIST dahil tüm geometriler çözüldü), `DestroyClump`
  (Clump::destroy frame hiyerarşisini de siler — ek destroyHierarchy
  double-free).
- `RenderPlatform_AddClump` + `RenderPlatform_RenderScene`
  (world->render — Begin/EndFrame arası).
- **Doğrulama**: worldgeometry_test 32 check (üg dünya envanter sayıları
  birebir: VIKING01 96/69657/70190, IDUNA03 31/10074/11226, BEAVER01
  119/105358/107669; bozuk libid/truncate → FALSE; ASAN temiz);
  ctest **18/18**; win32 OK.
- **Sonraki**: 4.6p TAMAMlandı (kamera 0x0704 CWbCameraBehavior'dan +
  spawn zinciri 97 örnek); 4.6q TAMAMlandı (freecam + ocean wire-in +
  auto-framing); sıradaki 4.6r (Rt2dMaestro menü) ∥ 4.6s (entegrasyon +
  tekne follow + trailing chunk decode).

### Görsel Deneme (GL3 Spike) — TAMAM (2026-10-10)

- `CANNONCRUISE_RW_PLATFORM=GL3` (ayrı `build-gl3/`, brew glfw3) —
  librw gl3-glfw cihazı: `RwEngineOpen` EngineOpenParams (1280×720);
  kamera CAMERA/ZBUFFER raster; `EndFrame` glfwPollEvents; pencere
  kapatma ShouldClose (rwcore.h platform API).
- main'e RWS argümanı: envanter → clump → AddClump → .col trimesh →
  kamera pose (0,180,420)→origin → pencere kapanana dek render.
- **Kök çökme**: `gl3::instance` meshHeader NULL — fix
  `rw::registerMeshPlugin()` (RwEngineInit; NULL platformda instance
  yoktu → gizli kalmıştı).
- **Doğrulama**: VIKING01 GL3'te 3600/3600 kare (60s pencere), üç dünya
  headless'da uçtan uca; null build + ctest 18/18 yeşil.

### Faz 4.6p: CAttributeHandler paketleri + behaviour apply — TAMAM (2026-10-10)

- 0x0704 property kayıtları → attribute dispatch: WbRender (FUN_004c3e40
  apply), CPhysicsWorldBehaviour, kamera — gerçek dünya nesnelerine.
- Object record'lerden entity spawn zinciri (create-by-name 4.6k +
  property apply); ocean wire-in (+0x60/+0x68 GUID) TODO'dan çıkar.
- **Çıktı**: sahne nesneleri binary'deki attribute değerleriyle kurulur.
- **Test**: attribute paket fixture (MAINMENU 0x0704 kayıtları) apply →
  beklenen alan değerleri (render/camera/audio uçtan uca 4.6k gibi).

#### Uygulama notları (2026-10-10)

- **Parser düzeltmeleri** (`RwStudioStream.cpp` 0x0704):
  - `0x80000000` segment: ad tekrarı YALNIZCA ad==mevcut sınıf ise;
    farklı ad (ör. VIKING01 obj[264] `CSystemCommands`) = YENİ behaviour.
  - `flags=0` segment: içerik uzunluğu kesin (`segLen-8`) tüketilir;
    16B blob (GUID — CSystemCommands alt segmentleri) ReadPadString'le
    AŞIRI OKUNUP stream'i kaydırıyordu (eski hata: kamera kaydında
    props=0 + bozuk ad). İlk bayt yazdırılabilir ise string (ad).
- **Dünya kamerası**: VIKING01 obj[264] = CWbCameraBehavior×2 +
  CSystemCommands + **32 property** (id0 pos=(0.35,7,0), id4 FOV=40°,
  id6 dir=(0.85,0.85,0.85) — gerçek dünya yerleşimi). `main.cpp`
  `RwsCamera_LoadPose`: ilk props'lu CWbCameraBehavior → CCameraState
  apply → `RenderPlatform_SetCameraPose(pos, pos+dir)`; bulunamazsa
  görsel-deneme fallback (0,180,420)→origin.
- **Spawn zinciri** (`WorldSpawn.h/.cpp`): `GameBehaviours_Register()`
  (render/camera/audio/physics adaptörleri — behaviourloader_test
  deseni) + `WorldSpawn_FromFile()`: tüm 0x0704 kayıtları walk →
  `BehaviourLoader::ProcessObject` → CClassFactory create + apply.
  - VIKING01: **497 kayıt → 97 örnek üretildi** (CTowerCatapult ×20,
    CCollisionTrigger ×19, CBirdSeaGull ×10 … + render/camera/audio),
    152 bilinmeyen sınıf sessiz atlandı (binary davranışı).
  - MAINMENU: 4 kayıt → 5 örnek (render+camera+audio apply uçtan uca).
- **`RWS_REGISTER_CLASS` makro düzeltmesi** (cclassfactory.h): 3-arg
  biçim `RWS_REGISTER_CLASS(Class, factory, size)` **fabrikayı
  yok sayıyordu** (her zaman NULL-factory kaydediyordu — 33 sınıf ad
  kayıtlıydı ama `Create()` NULL dönerdi). Artık 1/2/3-arg dispatch
  (GET_MACRO); 33 kayıtlı sınıfın factory'si gerçekten çalışıyor.
- **Trailing chunk** (0x22/0x1E/0x39, len=20): üç dünyada aynı yapı —
  `[u32 sectionBytes][libid 0x1803FFFF][0x1][0x10][libid]` + ATOMIC
  akışı devam ediyor (0x22 payload'ı sonrası artifact 0x01DCF840 vb.);
  geçici `[DBG]` satırı WorldGeometry.cpp'den kaldırıldı. Decode → 4.6s.
- **Test**: `worldspawn_test` (VIKING01 497/97/152 + MAINMENU 4/5 +
  ClassIsRegistered smoke) + rwstudio_test VIKING01 obj[264] kamera
  fixture (32 prop, id0/id4 değerleri) → **ctest 19/19**.
- **Doğrulama**: null + GL3 VIKING01 3600/3600 kare, exit=0
  (GL3: WorldSpawn 97 örnek + kamera (0.35,7,0)→(1.2,7.85,0.85));
  sdk probe 108/113 (4 `rw.h`/Bullet include-path artefaktı —
  pre-existing; 1 build-gl3 glob yanlış pozitifi).

### Faz 4.6q: Kamera + oyuncu kontrolü — TAMAM (2026-10-10, freecam; tekne follow 4.6s)

Uygulama notları (2026-10-10):
- **Kamera auto-framing**: 0x0704 kamera verisi bir chase-rig (id6 MUTLAK
  lookAt, takip hedefi olmadan dünya koordinatı değil) → render'da dünya
  bbox'ından genel bakış: pos=(cx, cy+span*0.28, cz+span*0.55),
  lookAt=(cx,cy,cz). `CANNONCRUISE_CAM=DATA` ile veri kamerası seçeneği
  (RwsCamera_LoadPose). API: `WorldGeometry_GetLastBounds` (CreateClump'da
  biriken min/max), main'de `RenderPlatform_SetCameraPose`.
- **GL3 quirk (belgeli)**: `setViewWindow(0.6,0.6)` GL3 frustum culling'i
  bozup TAMAMEN BOŞ kare veriyor → viewWindow (1,1) sabit (RenderPlatform).
- **Freecam**: `RwPlatformKeyDown` (GL3: glfwGetKey; NULL: FALSE) + main
  döngüsünde WASD (bakış düzleminde) / Q/E (dikey); delta pos+look'a eşit
  uygulanır (bakış ofseti korunur); hız = camSpan*0.10/frame.
- **Boot akışı**: argsız → MAINMENU.RWS dene (resLen=0, external {GUID}.dff
  elimizde yok → clump boş) → VIKING01.RWS fallback.
- **Okyanus wire-in (4.6p'ten dev)**: `COcean::Initialize` boş stub idi →
  gerçek ızgara (5×5 tile, tile=span/2, sub=8, y=0 su seviyesi);
  `COceanTile::Initialize` planar grid + UV + su mavisi doldurma + Update
  tam dalga formülü (CalculateWaterHeight ile aynı: sin(x*0.05+t*2)*1.5 +
  cos(z*0.08+t*1.5)); `COcean_W32::RenderWaterMesh` tek TRISTRIP (grid'i
  temsil edemezdi) → indeksli TRILIST + doğrudan `rw::im3d` (rwstubs
  no-op C API'ye düşmez; GL3: gl3::Im3DVertex dönüşümü; NULL: çizim yok).
  main: bbox XZ kapsayan 5×5 tile + frame döngüsünde Update/Render.
- **Testler**: `tests/ocean_test` (20/20 ctest — ızgara/dalgalar/render
  smoke + clamp + reinit), worldgeometry_test'e bbox assertion
  (min.y<0, max.y>100, |x|>400).
- Kalan: tekne placeholder + PhysicsWorld_Step + kamera follow (4.6s
  entegrasyonla); su dokusu/specular (görsellik borcu).
- **Çıktı**: dünyada gezilebilir (fly), su ızgarası çizilir.
- **Test**: ocean_test + bbox assertion TAMAM (ctest 20/20); GL3 smoke
  3600/3600 exit=0.

### Faz 4.6r: Rt2dMaestro menü (Menu.anm) — KAPI 5

#### Giriş akışı envanteri (2026-10-10 RWS incelemesi)

**Boot zinciri (binary)**: startup → GLOBAL.RWS → INTRO1 → INTRO2 →
INTRO3 → **MAPMENU** (INTRO3 kaydının hedefi) → MAINMENU → dünya.

**INTRO1/2/3.RWS** (1196 B — video DEĞİL, sahne tanımlayıcı):
- Her biri 4× `0x0716` varlık kaydı: TextureDictionary / WorldGeometry /
  StaticCollisionData / PathFindingData — TÜMÜ aynı GUID'li DIŞ build
  output dosyaları (`{3681F6C4…}` / `{A716C2EC…}` / `{FEEACA57…}` .txd/
  .dff/.col/.pfm) — elimizde YOK.
- ObjectRecord: sınıf **`CFMVBehavior`** ("Intro"/"Intro2"/"Intro3"),
  sahne adları "Nordic01"/"Nordic02" + CAttributeHandler kayıtları.
- INTRO3'ün bir sonraki hedefi **MAPMENU** (MAINMENU değil!).
- Sonuç: intro'lar Bink FMV oynatımı (kapsam dışı erteli) + eksik dış
  geometri; rekonstrüksiyonda **skip** (fallback zaten mevcut).

**MAINMENU.RWS** (1.86 MB):
- Gömülü: **Menu.anm 172 KB** (Rt2dMaestro2D menü zaman çizelgesi —
  asıl menü UI'ı: logo/butonlar). Dış (yok): `{B834571C…}` txd/dff/col/
  pfm → 3D menü arkaplanı atlanır.
- ObjectRecords: CWbRenderBehavior, CWbCameraBehavior,
  CAudioStreamBehaviour, **CMenuBehaviour**.

**MAPMENU.RWS** (2.88 MB):
- **CAfterGameMenuBehavior** + **CAG_Menu.anm** (gömülü) + seviye listesi
  kayıtları: `/Nordic_1/`, `/Viking_1..5/`, `/Iduna_1/`, `/Beaver_1..5/`
  → kampanya seviye seçimi (INTRO3'ün gerçek hedefi).

**GLOBAL.RWS** (6.18 MB — global boot sahnesi):
- Gömülü: 525 KB txd bloğu + **5.6 MB "Image_Win32.rws"** (UI
  font/atlas image paketi) — elimizde GÖMÜLÜ.
- Global sınıflar (objectRecord/string): CAudioManager, CAudioObject,
  CAudioStream, CAttributeHandler, CDebugTools, CDifficultyLevels,
  CEventProxy, CProgressIndicator, CSystemCommands + rwID_2DMAESTRO.

#### Uygulama planı

1. **rt2danim parser** (Menu.anm + CAG_Menu.anm fixture): headers mevcut
   (rt2d.h/rt2danim.h); adım/kare sayısı + sprite/frame dizini çıkar →
   test fixture.
2. **Rt2dMaestro Im2D render pass**: MSG_MENU_RENDER_RT2D (RenderBehaviourWorld
   kaydı) → menü UI'ı ekrana; font: GLOBAL image_Win32 paketinden.
3. **CMenuBehaviour state machine**: menü attribute'ları → seçim olayları
   → dünya yükleme tetikleme (VIKING01/IDUNA03/BEAVER01).
4. **MAPMENU**: CAfterGameMenuBehavior + CAG_Menu.anm + seviye kayıtları
   (aynı rt2d yolu; seviye seçimi → world load).
5. **GLOBAL load**: gömülü txd/image decode + global objectRecord
   register (CAudioManager stub — ses erteli).

- Risk: kapı 5 (binary'nin en bilinmeyen alanı). Fallback: menüsüz
  direkt dünya yükleme (Faz 4.6s) — zaten çalışıyor.
- **Test**: anim stream fixture parse (adım/kare sayısı); render manuel.

#### Durum (2026-10-11) — adım 1–3 TAMAM

1. **rt2danim parser TAMAM** — `code/Rt2dMaestroStream.h/.cpp`:
   `Rt2dMaestro_Inspect` (header 40B + node walk + sahne üçlüsü) +
   `Rt2dMaestro_FindAnimInAsset` (0x0716 payload içinde `rwID_2DMAESTRO`
   back-up tarama). Test: `tests/rt2danim_test.cpp` — Menu.anm (32 node,
   canvas 640×480, field2=433) + CAG_Menu.anm (44 node) fixture birebir;
   **21/21 ctest**.
2. **Rt2dMaestro Im2D render TAMAM (placeholder)** — `code/Rt2dMaestro.h/.cpp`:
   gerçek nesne (parse + selection + time); `Rt2dMaestroCreateFromMemory`
   + `Update/Render/SelectNext/SelectPrev/GetSelectedCommand`;
   Im2D render: koyu zemin + başlık şeridi + 6 satır seçim çubuğu +
   zaman (canlı Update kanıtı) — `RenderPlatform_RenderTestQuad` deseni
   (`rw::im2d` + üst matris). rwstubs no-op'ları silindi.
   Tam timeline decode + font (GLOBAL image paketi) → görsellik erteli.
3. **Menü durum makinesi TAMAM (boot düzeyi)** — `main.cpp` menü fazı:
   MAINMENU.RWS chunk taraması → `Rt2dMaestroCreateFromMemory` → menü
   döngüsü (W/S/↑/↓ seçim, Enter/Space onay, Esc/Q kaçış; kare kapak
   `CANNONCRUISE_MENU_FRAMES` otomatik onay — null/headless uyum).
   Onay → mevcut boot adayları (MAINMENU clump boş → VIKING01) devam.
   CMenuBehaviour sınıf içi wire (m_pMaestro) → 4.6r-4.6s arası.
4. **MAPMENU** — CAfterGameMenuBehavior + CAG_Menu.anm + seviye kayıtları
    → bekliyor.
 5. **GLOBAL load** — gömülü txd/image decode + global register → bekliyor.
 6. **INTRO skip** — not: mevcut fallback zaten skip ediyor (menü fazı
    INTRO'ları hiç dener değil).

#### Im2D görünürlük fix (2026-10-11) — KÖK NEDENLER
- Ekran görüntüsü dünyayı gösterip overlay'i göstermiyordu → **üç kök neden**:
  1. **Koordinat uzayı**: Im2D vertex'leri ekran uzayında **piksel**
     koordinatı ister; eski `DrawQuad` üst-matrisle view-window (-1..1)
     üretiyordu → ~1 piksel köşede çizim (görünmez). Yeni
     `RenderPlatform_DrawOverlayQuad(x,y,w,h,argb)` piksel koordinatlı.
  2. **Render state**: 3D sahne derinlik yazar; Im2D ZTEST açıkken eleniridi.
     `RenderPlatform_BeginOverlay2D/EndOverlay2D` (doku yok, Z test/write
     kapalı, vertex alpha + srcalpha blend). librw C++ API'si
     (`rw::SetRenderState`) — C stub `RwRenderStateSet` GL3'te yok.
  3. **No-op stub**: `ShowText`/`DrawPixel` C API `RwIm2DRenderPrimitive`
     (rwstubs.cpp) GL3'te no-op → metin hiç çizmiyordu. Artık metin
     `RenderPlatform_DrawOverlayQuad` üzerinden gerçek librw Im2D'ye gider.
- Boot akış düzeltmesi: VIKING01 fallback'i `pWorldPath`'i dolduruyordu →
  `if (pWorldPath == nullptr)` intro/menü kapısı ölü koddı. Yeni
  `bBootFlow = (argc <= 1)` — argümansız çalıştırmada giriş zinciri dünya
  üstünde overlay olarak akar; explicit dünya argümanı doğrudan interaktife.
- **Doğrulama aracı**: `CANNONCRUISE_SCREENSHOT=<yol.png>` — GL3 framebuffer
  PNG (swap öncesi, `Raster::toImage` → `rw::writePNG`). Intro ve menü
  kareleri piksel örneklemesiyle doğrulandı: menü zemini (12,20,32), başlık
  şeridi (32,64,104) + metin (220,230,255); intro kartı metni de aynı.

#### Video-tarzi giriş menüsü (2026-10-11) — "hala aynı" fixi
- Node dökümü (32): butonlar = **girdi bağları** (btnUp/Down/Left/Right +
  gamepad btnTriangle/Circle/Cross/Square/Select/Start), görünür etiket
  yok; gruplar = ses-dalga/Single-Difficulty durumları. Görünür metinler
  `Localiza/lang_eng.txt`'den (UTF-16): MAIN (0200-0203+0001), OPTIONS
  (0300-0303), SOUND (0400-0403), NEWGAME onayı (0500/0501 SAVE GAME?),
  DIFFICULTY (0600-0603), SINGLE/TWO (0070/0071), PAUSE (0000-0003/0007).
- `CMenuBehavior` durum makinesi: MAIN→OPTIONS→SOUND/DIFFICULTY,
  MAIN→CONFIRMNEW(SAVE GAME?/DON'T SAVE); terminal eylemler
  (NEWGAME/LOADGAME/QUITGAME/CREDITS) main.cpp'ye `TakePendingAction`
  ile; Esc = üst ekran (MAIN'de çıkış); A/D/←/→ ses barı + zorluk.
- Render: parşömen zemin (0xC7A05E) + yanık çerçeve, "Hugo"+"CANNONCRUISE"
  başlık, topuz imleç, 10 segment ses barları, Enter/Esc ipucu kutuları —
  piksel doğrulamalı (başlık mürekkebi, imleç topuzu).
- Maestro `Update` hâlâ tikler (timeline animasyonu artığı); placeholder
  `Rt2dMaestroRender` menü yolunda artık çağrılmıyor.

#### Gömülü menü art'ı (2026-10-11) — "RWS içinde olmalı" doğrulandı
- Kullanıcı haklı: MAINMENU.RWS içinde **rwID_TEXDICTIONARY** (1.68MB
  0x0716) — 16 doku (`TxdDecode` + `txddecode_test` ile çözülü): Menu_074
  (640×480 parşömen zemin), Menu_076 (512×128 "Hugo CANNONCRUISE" logo,
  alfalı), 11× 128×32 (bar/duğme parçaları), 3× 32×32 (ikon/imleç).
- Piksel RGBA (TxdDecode çıktısı; BGRA swizzle MAVİ yapar — düzeltildi).
- Sahne (421 kayıt) dokuları **isimle** referanslar (her Menu_* 2× geçer:
  texdict + anm). Kayıt grameri: libid (0x1803FFFF) sınırlı alanlar +
  float'lar (x=-256/-64 gibi) — decode artığı (buton/logo yerleşimi).
- `RenderPlatform_UploadTexture/DrawTexturedQuad/HasTexture/ClearTextures`
  (librw Image→Raster→Texture; LINEAR+CLAMP, mipmap yok) + `Menu_Main`:
  CPU kompoziti (074 + 076 @canvas 70,35; 640×480 opak) — tam ekran zeminde
  %98.9 referans eşleşme + logo harf sınırları doğrulandı. Öğeler/imleç/
  barlar bitmap overlay (sahne decode'una kadar).
- Sıradaki: sahne kayıt grameri → dokulu buton/bar/imleç yerleşimi
  (Menu_117..137 bar segmentleri, 084/087/095 ikonlar) + MAPMENU art'ı
  (CAG_Menu.anm + texdict).

#### MAPMENU fazı (2026-10-11) — 4.6r-4 TAMAM
- MAPMENU.RWS: 0x716 texdict (29 doku `CAG_Menu_*`) + 0x716 CAG_Menu.anm
  (44 node: 6 komut SoundMove/NoSaveGame/MapMenuEnter/SoundSelect/
  SaveGame/' SoundSelect' + 10 btn + 28 grup). Seviye grupları (12):
  Viking_1..5, Nordic_1, Beaver_1..5, Iduna_1 → "VIKING 1"+`VIKING01`…
  Skor basamak grupları (Died/Hits/Killed/Accuracy ×100/10/1) + Rating/
  LevelName/LastRating = özet ekranı altyapısı.
- RWS isim tuzağı: `Menu_079` araması `CAG_Menu_079` kuyruğuna takılır;
  gerçek isimler `CAG_Menu_*`. Art: CAG_Menu_079 (640×480 ızgara harita —
  video birebir), CAG_Menu_042 (512² parşömen), CAG_Menu_067 (32² gülle).
- `MenuTextures_LoadFromRws` (direkt 0xe8; RwStudio MAPMENU/GLOBAL'u
  atlar — tüm menü/harita/seviye texdict'lerinde sabit düzen).
- `CAfterGameMenuBehavior`: maestro sahiplenme + grup→seviye parse +
  seçim/onarım + `ShowSummary` (skor/isabet/derece ekranı) + video render
  (harita zemin + "TRICKY TROLL FJORD" + "YOUR RATING - NOT COMPLETED" +
  liste + topuz + SELECT/BACK).
- main.cpp: setup bloğu boot-flow SONRASINA taşındı (tek yükleme;
  reload/RemoveClump/çifte-spawn gerekmez); MAPMENU NEW/LOAD sonrası
  (otomatik kapakta da) → onayda unload+load+restore-güvenceli seçim →
  kurulum seçili dünyada. `CANNONCRUISE_MAPMENU_FRAMES`,
  `CANNONCRUISE_SCREENSHOT_MAP`.
- Doğrulama: 12 seviye listeli, `level 'VIKING 1' already loaded`,
  2/2 kare; başlık mürekkebi + liste metni pikselli; ctest 21/21.
- "Direkt viking 1" fixi (2026-10-11): basılı Enter fazlar arası sızıp
  haritayı görünmez onaylıyordu (menü/map/intro `s_prev*` hep FALSE
  başlıyordu). Üç döngü de girişte fiziksel tuş durumuyla tohumlanıyor;
  NORDIC 1 reload yolu doğrulandı (347 kayıt, assets=439).

### Altyapı (2026-10-11) — yapısal sadeleştirme
- `Rt2dMaestro_LoadFromRwsFile` (tek-yol maestro yükleme): main.cpp'deki
  üç kopya 0x0716-walk kalktı (menü/harita fazları + testler aynı yol).
- Doku yükleme tek-yol: `MenuTextures_LoadFromRws` (direkt 0xe8;
  RwStudio MAPMENU/GLOBAL'u atlıyordu). `Menu_Main` kompoziti helper'a
  taşındı (`UploadMenuMainComposite`). C API stub ikiliği sürüyor
  (RwStudio dünya hattında) — yeni render kodu RenderPlatform köprüsünü
  kullanır (kural).
- `menustate_test` (22. test): menü hiyerarşi geçişleri + harita 12-seviye
  parse/seçim + geçersiz girdi; headless/enginesiz (Render çağrılmaz).
- `code/build.sh`: stale-binary tuzağına karşı tek komut (rm + null + GL3
  + ctest). Girdi edge-pattern üç fazda tohumlu tutarlı; InputManager'a
  taşıma ertelendi (risk/fayda).
- main.cpp hâlâ monolit (~1150 satır) — faz fonksiyonlarına bölme sonraki
  altyapı adımı.

- Doğrulama: **ctest 21/21**, GL3 smoke (`CANNONCRUISE_MENU_FRAMES=180
  CANNONCRUISE_FRAMES=30`): `Menu: nodes=32 commands=15 canvas=640x480` →
  otomatik VIKING01 dünya (30/30 frames OK); sdk probe **113/119**
  (5=`rw.h` artefakt + 1 glob — RenderPlatform.cpp ile aynı Im2D deseni),
  ocean-extra 1/1, isolated-contract 3/3.
- Sonraki: 4.6r-4 (MAPMENU behaviour wire) ∥ 4.6s (boot zinciri +
  trailing chunk decode).

### Video Analizi — Oyun Yapısı (SCR.mov + SCR1.mov, 2026-10-11)

Hugo CannonCruise (ITE Media). SCR.mov (112 sn, 2880×1800): boot→menü→
credits akışı; SCR1.mov (75 sn, 1024×768): tutorial seviyesi (oynanış +
kamera eğitimi + pause). Aşağıdaki kurgu video-gerçektir; kod mimarisi
buna göre düzenlenir.

#### SCR.mov zaman çizelgesi (5 sn örnekleme)
| # | İçerik | Karşılık |
|---|--------|----------|
| 1–2 | ITE krom logo animasyonu + "It's all about entertainment" kartı | INTRO1-3 (FMV/logo; bizde skip+başlık kartı) |
| 4,7,22,23 | Yükleme ekranı: siyah üstünde parşömen şerit, yelkenli + dalga (gemi ilerler = progress) | LOADING durumu (şu an yok → eklenecek) |
| 5,13,20 | MAINMENU (parşömen): NEW GAME / LOAD GAME / OPTIONS / CREDITS / QUIT GAME; topuz imleç; 20'de **kayar geçiş** (OPTIONS kayarak çıkar) | Menu.anm (Rt2dMaestro **animasyonlu** geçişler!) |
| 12,15,16 | OPTIONS: SOUND / DIFFICULTY•EASY→HARD / SINGLE PLAYER; SOUND alt: VOICES/EFFECTS/MUSIC barları; SELECT(Enter)/BACK(Esc) | 15 komutun görünür ağacı (aşağıda) |
| 6 | MAPMENU: "TRICKY TROLL FJORD / YOUR RATING•NOT COMPLETED" — ızgaralı parşömen harita, ada şekilleri, sol-üst gemi (oyuncu konumu), SELECT | CAG_Menu.anm + MapMenuEnter (4.6r-4) |
| 9 | Seviye girişi: takip kamerası arkasında top donanımlı gemi, iskele/kale; bulut banner "TRICKY TROLL FJORD."; HUD: skor kadranı (200), HP barı, pusula | LEVELINTRO + GAMEPLAY |
| 10–11 | Oyun-içi OPTIONS•SOUND overlay (VOICES/EFFECTS/MUSIC) — arka planda 3D donar | Pause-alt menü (maestro overlay) |
| 17,19 | CREDITS (kayan parşömen: V.P. Development, PM, Art Director, artists…) | CREDITS.RWS |

#### SCR1.mov zaman çizelgesi (tutorial)
| # | İçerik | Karşılık |
|---|--------|----------|
| 1–4 | Takip kamerası: Hugo dümende, top, direkten martı (Fernando), uyandırma izleri, iskelede top mermisi fıçısı; skor 200→199→197→195 (**geri sayım**) | GAMEPLAY Normal view |
| 4 | "WELL DONE, CAPTAIN." övgü banner'ı (hedef tamamlandı geri bildirimi) | CShowText (NEXT yok, zaman aşımı) |
| 5–6 | "IN NORMAL VIEW, YOU CAN TURN AND TILT THE CAMERA…" + "THIS IS HANDY WHEN YOU ARE SAILING IN ONE DIRECTION AND SHOOTING IN ANOTHER ONE." | **Yelken yönü ⇄ top namlusu bağımsız!** |
| 7–9 | "PRESS BINOCULAR BUTTON TO GET FERNANDO'S BINOCULARS." → dürbün görüntüsü (**dairesel vinyet**, zoom) | Binoculars kipi (Fernando = papağan gözcü) |
| 10–11 | "CANNON VIEW IS HANDY WHEN YOU ARE CLOSE TO YOUR TARGET." → top görüntüsü (dairesel vinyet) | Cannon kipi (yakın hedef) |
| 12 | "YOU NOW KNOW HOW TO OPERATE THE CAMERA." | Eğitim tamamlama |
| 13 | Fernando direkte dürbünle bakıyor | Karakter detayı |
| 14 | PAUSE: CONTINUE GAME / OPTIONS / RESTART LEVEL / QUIT GAME (+SELECT/BACK) | Esc menüsü (maestro) |

#### Durum makinesi (video-gerçek; kod hedefi)
```
BOOT → LOGO_ITE → LOADING ⇄ MAINMENU ⇄ OPTIONS ⇄ SOUND
                                    ├→ CREDITS → MAINMENU
                                    ├→ LOADGAME → MAPMENU
                                    └→ NEWGAME → MAPMENU
MAPMENU → LOADING → LEVELINTRO → GAMEPLAY ⇄ PAUSE ⇄ OPTIONS
   ↑________ LEVELCOMPLETE (ShowSummary: score/accuracy/rating) ┘
GAMEPLAY iç kipler: NORMAL ⇄ CANNON ⇄ BINOCULARS; TUTORIAL banner'ları
(CShowText, NEXT=Enter) modal overlay; OUTRO1-3 → CREDITS.
```

#### Menü ağaçları (görünür etiket ↔ 15 maestro komutu)
- MAINMENU: NEW GAME (NewSave/NewNoSave) / LOAD GAME (LoadGame) /
  OPTIONS (OptionsEnter) / CREDITS (Credits) / QUIT GAME (QuitGame)
- OPTIONS: SOUND (SoundSelect+EnterSound) / DIFFICULTY (SetDifficulty:
  EASY/HARD) / SINGLE PLAYER (SetMultiPlayer: single/multi)
- SOUND: VOICES (SpeakVolSet) / EFFECTS (EffectVolSet) / MUSIC
  (MusicVolSet) + gezinme (SoundMove) + değer (SoundChange)
- PAUSE: CONTINUE / OPTIONS / RESTART LEVEL / QUIT GAME (aynı Menu.anm,
  farklı durum — maestro durum parametreli)
- Tüm menüler **kayar animasyonlu** (Rt2dMaestro timeline — sahne bölümü
  decode'u 4.6r artığı), parşömen zemin + topuz imleç + klavye ipucu
  (SELECT=Enter, BACK=Esc) — gerçek görseller {GUID} txd/dff (yok).

#### HUD (oynanış overlay — Rt2d 2D katman hedefi)
- Sol-alt: skor kadranı (**geri sayımlı** sayı, örn. 200→189) + altında
  kırmızı HP barı (GUIHealthBar) + top mermisi sayacı ("2")
- Sağ-alt: pusula (N/E/S/W, gemi ikonu, ortada hedef oku — CGUIMap +
  MissionIndicator; yeşil ok = hedef yönü)
- Üst-orta: bulut/banner (seviye adı, tutorial metni — CShowText)
- Dairesel vinyet maskesi (Cannon/Binocular kipleri)

#### Seviye/RWS eşlemesi
- Bölge: "TRICKY TROLL FJORD" (MAPMENU). Seviyeler: NORDIC01/02,
  VIKING01-05, IDUNA01-03, BEAVER01-05 (15 seviye adayı); INTRO1-3
  (logo/FMV), OUTRO1-3, MAPMENU, MAINMENU, CREDITS, GLOBAL, LANGSEL, JINGLE.
- Derece: NOT COMPLETED → ShowSummary sonrası rating (CAfterGameMenuBehavior).

#### Sınıf eşlemesi (kodda mevcut → video rolü)
- CMenuBehavior (Menu.anm) → MAINMENU/OPTIONS/SOUND/PAUSE; m_pMaestro wire
  (4.6r-4.6s arası → bu fazda sahiplenme eklendi)
- CAfterGameMenuBehavior (CAG_Menu.anm, MapMenuEnter) → MAPMENU + ShowSummary
- CShowText → tutorial/banner/övgü + altyazı (zaten render ediyor)
- CGUIMap + MissionIndicator → pusula + hedef oku; GUIHealthBar → HP+skor
- CWbCamera → NORMAL/CANNON/BINOCULARS kipleri (SetDistance/SetElevation)
- FernandoActor (papağan), Cannonball, ShipEnemy, TowerCatapult (kıyı
  kuleleri), Soldier, AmmoCrate, Lifebuoy, Bird (martılar), Fish, Flotsam,
  Debris; Fade (geçişler), JINGLE (ses).

### Canlı Debug — Orijinal Oyun (Wine/winedbg, 2026-10-11) — INT3 ÇÖZÜLDÜ

**Bağlam**: Oyun `/Applications/CannonCruise.app` (Wine 10.0, MoltenVK) altında
çalışırken winedbg'ye bağlanıldı; `0x004c56b8: int3` yakalandı.

**Kök neden (binary + canlı doğrulama)**: INT3 oyunun **kendi assert breakpoint'**i.
- `FUN_004c5230` = `CWbRenderBehavior::DoRender` (WbRender.cpp).
- Assert: **`pCameraResult != NULL Failed`** (`s_pCameraResult_!=_NULL_Failed_00614254`),
  dosya `D:\Projects\CannonCruisePC\code\WbRender.` (`0x00619618`), **satır 626** (`0x272`).
- Akış: `call 0x50ba60` (**vtable[0x18] thunk** = kamera-result sorgusu) →
  `test eax,eax; jnz 0x4c5723` (non-NULL → assert bloğunu atla) → sıfırsa assert.
- Assert makro deseni (358 site binary'de): `push 0x40; push "Assert failed:";
  push msg; push 0;` ardından **ya `ff d5` (MessageBoxA — kardeş site 0x4c596e)
  ya `cc` (INT3)**. 0x4c56b8 INT3 sürümü.
- **Canlı kanıt**: `0x4c559f`'e breakpoint kondu; `DoRender` her karede çağrılıyor,
  **EAX (pCameraResult) tutarlı non-NULL = 0x01aa3254** → assert **normal
  çalışmada/sırada ateşlenmiyor**. Sadece vtable[0x18] NULL dönerse (kamera alt
  sistemi hazır değil / belirli geçiş durumu) tetiklenir.
- Ek: `ms_pSingleton != NULL` assert @ `0x4c549b` (satır 66) aynı ailede.

**Sonuç**: Oyuncunun debugger'da gördüğü int3 = orijinal oyunun kamera-result
assert'i; bu, tersine mühendislik için **DoRender'ın kamera alt sistemine bağımlı
giriş sözleşmesini** doğrular (port hedefi: `WbRender.cpp` + `FUN_0050ba60` semantiği).

### Faz 4.6s: Oyun döngüsü entegrasyonu + seviye akışı

- Boot zinciri (2026-10-10 düzeltmesi): startup → GLOBAL.RWS (global
  sınıflar + UI image) → INTRO1→2→3 (FMV: Bink yok → **skip**; dış
  geometri de yok) → MAPMENU (seviye seçimi ∥ skip) → MAINMENU (menü
  ∥ skip) → VIKING01.RWS load (geometry + collision + behaviours
  + kamera) → play loop → seviye geçişi (IDUNA03/BEAVER01).
- Hata yolları: asset bulunamazsa skip + log; unload zinciri (Unload
  vtable slot[4] karşılıkları).
- **Çıktı**: `./CannonCruise` → oyun dünyası açılır, gezilir, seviye
  değişir. (Ses PPB/XAudio2 ve Bink video kapsam dışı — opsiyonel faz.)
- **Test**: entegrasyon smoke (iki dünya load/unload stress, leak yok);
  ctest genişlemesi.

### Bağımlılık sırası ve tahmin

4.6n → 4.6o → 4.6p → 4.6q → (4.6r ∥ 4.6s). Her faz kendi testiyle
kapanır; 4.6r paralel/ertelenbilir. Kritik bilinmeyenler: WorldGeometry
iç yapısı (clump mı RpWorld mü — 4.6o ilk işi doğrulamak), librw GL3
pencere kurulumu (macOS dev ortamı), input iskeleti.

Sırada (öneri): Faz 4.6r-4 (MAPMENU behaviour wire: CAfterGameMenuBehavior
+ CAG_Menu.anm + seviye kayıtları → seviye seçimi) ∥ 4.6s (boot zinciri +
trailing chunk 0x22/0x1E/0x39 decode). Adım 1–3 (parser + Im2D placeholder
render + boot menü durum makinesi) TAMAM — 21/21 ctest, GL3 smoke 30/30.

Gerçek SDK/ABI uyumluluğu, bağımlılık kurulumu ve oyun çalıştırması
doğrulanmadı; kapı 4'ün stream doğrulaması AÇILDI (librw + gerçek asset),
tek sahne çalışma zamanı bekliyor.
