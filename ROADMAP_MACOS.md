# CannonCruise — macOS Yol Haritası

> **Son güncelleme:** 9 Ekim 2026  
> **Durum:** Plan hazır; yerel macOS uygulaması henüz derlenip çalıştırılmadı.  
> **İlgili belgeler:** [Ana yol haritası](ROADMAP.md) · [Derleme temeli](BUILDING.md) · [SDK arayüz incelemesi](SDK_INTERFACE_AUDIT.md)

## 1. Amaç ve kapsam

Bu belge Windows/MSVC yeniden inşa planından ayrı **yerel macOS portu** yolunu tanımlar. İlk hedef temiz bir macOS statik kütüphanesi ve SDK bağımsız testler; nihai hedef pencere açan, asset yükleyen ve tek sahneyi render/fizik ile çalıştıran bir macOS uygulamasıdır.

- Orijinal PE32/x86 `CannonCruise.exe` dosyasını macOS'ta doğrudan çalıştırmak bu planın hedefi değildir. Wine/CrossOver veya sanal makine ile çalıştırma ayrı bir uyumluluk çalışmasıdır.
- Windows x86 binary ABI'sini korumak yerine kaynak seviyesinde taşınabilirlik hedeflenir. Asset/dosya biçimlerinin sabit genişlikli düzenleri ayrıca korunmalıdır.
- **Önerilen sıra:** mevcut x86_64 kanıtını temiz derlemeyle yenile → arm64 derle ve gerçek Apple Silicon makinede test et → tüm bağımlılıklar uygunsa Universal 2 paketle. Bu sıra destek garantisi değildir.
- Minimum macOS sürümü ve dağıtım mimarileri grafik/fizik backend kararından sonra sabitlenecek. Universal 2, yalnızca her iki mimari de doğrulanırsa hedeflenecek.
- Aşağıdaki backend seçenekleri öneridir; bu belge bir SDK seçimi, kurulum izni veya tamamlanmış uygulama iddiası değildir.

## 2. Başlangıç durumu ve kanıt sınırları

9 Ekim SDK incelemesinin kayıtları temel alınmıştır; bu belge hazırlanırken yeni derleme/test yapılmadı.

- `code/CMakeLists.txt`: C++14, yalnızca `CannonCruiseLib` statik hedefi. macOS yürütülebilir hedefi, `.app` paketi ve gerçek SDK bağlantıları yok.
- Non-Windows dalı `framework/startup/win32/*`, `framework/startup/startup.cpp` ve `Ocean_W32.cpp` dosyalarını dışlıyor. Derlemeden dışlamak bu işlevlerin macOS'a taşındığı anlamına gelmez.
- Önceki `code/build/` cache'i/arşivi mevcut; güncel kaynaklarla temiz derleme kanıtı değil. Mevcut çıktılar korunmalı.
- Kayıtlı Clang syntax önkontrolünde **87/87 non-Windows kaynak geçti**. Nesne üretimi, arşivleme, link ve çalışma zamanı doğrulanmış değil.
- `AnimController.cpp` tamsayı → pointer dönüşümü ve `SceneManager.cpp` format argümanı uyarıları sürüyor.
- Ek Ocean kontrolü başarısız; izole startup kontrollerinin 2/3'ü geçti. Genel SDK önkontrolü hâlâ başarısız; kalan açıklar Im3D, Open/Init sözleşmeleri.
- Havok yerel allocator Debug/Release ASan+UBSan testleri x86_64'te geçti. arm64, SDK entegrasyonu ve gerçek fizik simülasyonu doğrulanmadı.
- Önceki worker incelemesinde Clang/Python kullanılabildi, CMake/Ninja bulunamadı. Bu tarihsel bulgudur; uygulamaya geçerken araç kullanılabilirliği yeniden kontrol edilmeli.

**Durum dili:** kaynakta mevcut ≠ syntax geçti ≠ derlendi ≠ linklendi ≠ çalışırken doğrulandı. Aşağıdaki uygulama adımlarının tamamı, aksi belirtilmedikçe bekliyor.

## 3. M0 — Araç zinciri ve temiz derleme tabanı (P0)

**Amaç:** eski cache'e dayanmadan tekrarlanabilir macOS derlemesi elde etmek.

- [ ] Worker'da AppleClang/clang++, CMake ve seçilecek build aracını keşfet; Xcode/Command Line Tools ve macOS SDK erişimini doğrula. Eksik kurulum veya PATH değişikliği için önce kullanıcı onayı al.
- [ ] Başlangıç tabanı olarak C++14 ve CMake 3.25+ kullanmayı değerlendir; kesin araç sürümlerini kaydet. Projenin mevcut CMake sözdizimi minimumu 3.16'dır.
- [ ] Kaynak revizyonu/yerel değişiklikler, macOS sürümü, SDK, derleyici, generator, mimari ve deployment target değerlerini kayıt altına al.
- [ ] `code/` dışında, depo içindeki yeni `build/macos-x86_64-<run>/debug` ve `release` dizinlerinde yapılandır; eski `code/build/` cache'ini kullanma veya silme.
- [ ] Mimariyi `CMAKE_OSX_ARCHITECTURES=x86_64` ile açık seç; tekli yapılandırma generator'ünde Debug/Release için ayrı dizin ve `CMAKE_BUILD_TYPE` kullan.
- [ ] Gerçek CMake kaynak listesini önkontrol scriptiyle karşılaştır; üretilmiş CompilerId kaynaklarının hedefe girmediğini ve platform dışlamalarını kaydet.
- [ ] `CannonCruiseLib` Debug/Release derlemelerini çalıştır; configure/build loglarını, çıkış kodlarını ve arşiv mimarisini doğrula.

**Kabul kapısı:** iki temiz yapılandırma ve derleme başarılı, arşivler x86_64, kaynak kapsamı ve araç sürümleri kayıtlı. Bu kapı dış sembollerin çözülmesini veya oyunun çalışmasını gerektirmez; onları kanıtlamaz.

## 4. M1 — 64-bit taşınabilirlik ve test tabanı (P0)

**Bağımlılık:** M0; kaynak incelemesi ve bağımsız testler M0 ile paralel yürütülebilir.

- [ ] `AnimController.cpp` dönüşümünde animasyon kimliğinin pointer mı handle mı olduğunu kanıtla; yalnızca cast ekleyerek uyarıyı gizleme. `SceneManager.cpp` format sorununu beklenen dosya yolu testiyle düzelt.
- [ ] Pointer/handle dönüşümleri, `int`/`long` varsayımları, struct packing, alignment ve dosya okuma/yazmalarını tara. Bellek adresleri için pointer genişliğini; disk biçimleri için doğrulanmış sabit genişlikleri kullan.
- [ ] Büyük/küçük harfe duyarlı dosya sistemi, Windows ters eğik çizgileri, çalışma dizini, kullanıcı veri yolları ve Unicode dosya adlarını test et.
- [ ] Allocator testini CTest'e bağla; Debug/Release ve desteklenen ASan+UBSan koşularını x86_64'te yeniden çalıştır. SDK bağımsız testleri oyun linkinden ayrı tut.
- [ ] arm64 için ayrı temiz build dizinleri oluştur; derleme ve testleri Apple Silicon makinede doğrula. Çapraz derleme veya Rosetta altında x86_64 çalıştırmayı yerel arm64 testi sayma.
- [ ] 32-bit'e özgü taşma testinin 64-bit hedefte çalışmadığını açık raporla; macOS için karşılık gelen sınır/boyut testlerini ekle.

**Kabul kapısı:** hedeflenen her mimaride Debug/Release derlemesi ve bağımsız testler başarılı; ele alınan kodda sanitizer hatası yok; kalan uyarılar gerekçeli bir listede. arm64 kanıtı yoksa yalnızca x86_64 doğrulandı olarak işaretle.

## 5. M2 — SDK/backend kararı ve platform sınırı (P0, kritik yol)

**Amaç:** başlık stub'larından gerçek uygulamaya geçiş yolunu seçmek. Envanter M0/M1 ile paralel yürütülebilir.

- [ ] RenderWare ve Havok için lisanslı, hedef macOS/mimariyi destekleyen kaynak/SDK erişimi var mı doğrula; eski Windows `.lib`/DLL dosyalarını macOS bağlantı çözümü sayma.
- [ ] Native SDK uygun değilse, kullanılan API alt kümesini uygulayan bir uyumluluk katmanının kapsamını çıkar. SDK başlıkları, derleme stub'ları ve backend uygulamalarını ayrı hedef/include yollarında tut.
- [ ] Render için Metal tabanlı uygulama ile macOS destekleyen bir soyutlama kütüphanesini karşılaştır; Im2D/Im3D, kamera, dünya/geometri, texture, animasyon, plugin ve stream kapsamlarını listele. OpenGL'in macOS'taki deprecated durumunu uzun vadeli seçimde hesaba kat.
- [ ] Fizik için kullanılabilir lisanslı Havok ile alternatif backend seçeneklerini davranış, collision, constraint, ownership ve asset uyumu açısından karşılaştır. Alternatif backend'in orijinal oyunla eşdeğer olduğunu varsayma.
- [ ] Pencere/girdi için Cocoa veya SDL benzeri katmanı değerlendir; zaman, dosya sistemi, ayar saklama ve ses/video arayüzlerinin platformdan bağımsız sınırını tanımla.
- [ ] Im3D veri düzeni/sabitleri, global bellek işlevleri, `RwEngineOpen` parametresi ve `RwEngineInit` linkage/sözleşmesini netleştir. Win32 HWND ayrıntılarını ortak macOS arayüzüne taşıma.
- [ ] Seçilen backend, lisans/dağıtım koşulları, mimari/minimum OS desteği ve kabul edilen davranış farklarını kısa karar kaydına yaz; uygulama kapsamını kullanıcıyla netleştir.

**Kabul kapısı:** render/fizik/platform yaklaşımı karara bağlanmış, gerekli kaynaklara yasal erişim ve hedef mimari desteği doğrulanmış, uygulanacak API listesi belli. Sadece bildirim veya no-op fonksiyon eklemek bu kapıyı kapatmaz.

## 6. M3 — macOS uygulama kabuğu ve gerçek link (P1)

**Bağımlılık:** M0/M1 ve M2 platform kararı.

- [ ] Ortak engine yaşam döngüsünü Win32 başlangıç kodundan ayır; macOS giriş noktası ve CMake yürütülebilir hedefi ekle. Windows yolunu bozma.
- [ ] Pencere açma/kapatma, olay döngüsü, klavye/fare, zamanlama, resize, Retina piksel ölçeği ve odak değişimini uygula.
- [ ] Registry yerine uygun ayar saklama, bundle kaynak yolu ve yazılabilir kullanıcı veri dizinlerini kullan; uygulama paketine çalışma zamanı yazısı yapma.
- [ ] En küçük uygulamayı gerçek platform/backend uygulamalarıyla linkle; unresolved sembol envanterini link logundan çıkar. Test double'larını açıkça ayır ve oyun hedefinin eksik işlevlerini gizlemek için kullanma.
- [ ] Başlatma hatası ve kısmi başlatma sonrası cleanup yollarını doğrula; küçük bir pencere/draw smoke testi ekle.

**Kabul kapısı:** native Mach-O uygulaması linkleniyor; pencere ve temel çizim açılıyor, girdi/resize işleniyor, kontrollü kapanıyor. Bu sonuç tam oyun veya tam RenderWare kapsamı değildir.

## 7. M4 — Render, asset ve tek sahne (P1)

**Bağımlılık:** M2 render kararı ve M3.

- [ ] Render state, matrix/kamera, geometri, texture ve Im2D/Im3D işlevlerini küçük görsel testlerle doğrula; koordinat, winding, renk, UV ve alpha farklarını kaydet.
- [ ] `Ocean_W32.cpp` içindeki taşınabilir mesh mantığını platform ayrıntısından ayır; Ocean'ı sonsuza kadar dışlamak yerine seçilen backend üzerinden çiz.
- [ ] RWS stream/chunk/plugin ve texture/animasyon yollarını gerçek asset örnekleriyle doğrula; kaynak kullanım haklarını kontrol et. Basit bir mesh yüklenmesini tam asset uyumluluğu sayma.
- [ ] Sahne yükleme, kamera, GUI katmanları ve gerçek metin çizimini tamamla; eksik performans paneli/metin işlevlerini açık takip et.
- [ ] Ses/Bink video bağımlılıklarını incele; native ve lisansı uygun çözüm yoksa ilk tek-sahne testinde kapalı olan özellikleri açıkça belgeleyip kapsam dışında tut.

**Kabul kapısı:** hakları uygun bir sahne yükleniyor, kamera/GUI/Ocean için tanımlı görsel testler geçiyor, eksik özellikler listeli. Orijinal oyunla görsel karşılaştırma ancak referans görüntü ve aynı sahne koşullarıyla yapılır.

## 8. M5 — Fizik, oyun döngüsü ve davranış doğrulaması (P1)

**Bağımlılık:** M2 fizik kararı ve M3; SDK bağımsız simülasyon testleri M4 ile paralel yapılabilir.

- [ ] Gerçek fizik dünyası oluşturma/yok etme, body/phantom ekleme/çıkarma ve sahiplik kurallarını uygula.
- [ ] `CPhysicsEntity` konum/hız getter-setter'larını gerçek sonuç döndürecek biçimde tamamla; `void*` sınırını tipli adapter arkasında tut.
- [ ] Fizik event aboneliğini ve mesaj filtresini doğrula; her mesajda koşulsuz update yerine belgelenmiş sabit zaman adımı ve render zamanlaması kullan.
- [ ] Yerçekimi, çarpışma, hız/konum, nesne kaldırma ve farklı render hızlarında simülasyon testleri ekle.
- [ ] Tek sahnede oyuncu kontrolü, fizik/render senkronu, pause/resume ve temiz kapanışı test et; referans davranışa ilişkin toleransları kaydet.

**Kabul kapısı:** simülasyon gerçekten ilerliyor; yerel vektör kaydı/no-op değil. Tek sahne testi geçiyor, sonuçlar ve bilinen backend farkları raporlanıyor.

## 9. M6 — Paketleme ve sürekli doğrulama (P2)

**Bağımlılık:** M4/M5; temel build/test CI işi M1 sonrasında başlatılabilir.

- [ ] macOS CI'da temiz Debug/Release derlemesi, CTest, uygun sanitizer koşuları ve mimari bazlı raporlar ekle. Lisanslı SDK ve asset'leri izinsiz runner/artifact içine koyma.
- [ ] `.app` bundle, `Info.plist`, kaynak yolları, dinamik kütüphane rpath/install-name ve lisans bildirimlerini düzenle.
- [ ] x86_64/arm64 ayrı çıktıları doğrula; Universal 2 isteniyorsa tüm yerel bağımlılıkların iki mimariyi içerdiğini kontrol et ve iki donanımda test et.
- [ ] Seçilen minimum macOS sürümünde ve geliştirme makinesi dışında açılış testini yap; harici geliştirme SDK yollarına bağımlı olmadığını doğrula.
- [ ] Harici dağıtım gerekiyorsa geliştirici hesabı/sertifika erişimi ve kullanıcı onayıyla code signing/notarization sürecini ayrıca yürüt. Yerel unsigned build ile dağıtıma hazır paketi ayrı raporla.

**Kabul kapısı:** tekrar üretilebilir `.app`, desteklenen mimari/OS matrisi ve bilinen sınırlamalar belgeli; dağıtım hedefleniyorsa imza/notarization doğrulaması da tamam.

## 10. Sıradaki somut işler

1. **M0:** araç kullanılabilirliğini yeniden kontrol et; eksikse onay iste ve temiz x86_64 Debug/Release statik derlemesini loglarıyla gerçekleştir.
2. **M1:** iki mevcut uyarıyı kanıtlı düzeltmeler ve regresyon testleriyle ele al; allocator testini CTest'e bağla.
3. **M2:** RenderWare/Havok macOS erişimini ve backend seçeneklerini karar kaydına dök; gerçek uygulama yolunu seçmeden stub kapsamını büyütme.

Windows derlemesinin bitmesini beklemek M0/M1 için şart değildir. M3–M5'in kapsamı ve süresi SDK/backend kararına bağlıdır; bugün çalışır oyun tarihi veya tamamlanma yüzdesi verilmez. Bu dosyanın oluşturulması yalnızca **planlama çıktısıdır**; derleme, kurulum veya kod portu yapılmış değildir.
