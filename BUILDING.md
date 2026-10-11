# CannonCruise — Derleme Temeli

> 9 Ekim 2026 · P0 / durum ve derleme reçetesi  
> Durum: **Belgelenmiş doğrulama hedefi; Windows'ta henüz çalıştırılmadı.**
> **macOS için:** [Ayrı macOS yol haritası](ROADMAP_MACOS.md). Aşağıdaki Win32/x86 hedefi ve PowerShell reçetesi macOS portuna uygulanmaz.

## 1. Hedef ve kapsam

İlk hedef `CannonCruiseLib` statik kütüphanesini Windows/MSVC ile temiz biçimde derlemektir. Mevcut `code/CMakeLists.txt` C++14 kullanır ve yalnızca `add_library(... STATIC ...)` tanımlar. Henüz EXE üretmez, gerçek SDK kütüphanelerini bağlamaz ve çalışma zamanı testi yapmaz.

**Hedef mimari: Win32 / x86 (32 bit).** Orijinal `CannonCruise.exe` dosyasının PE başlığında `Machine=0x014c` (i386) ve optional-header magic `0x010b` (PE32) okundu. Bu, ilk yeniden inşa hedefinin x86 seçilmesinin gerekçesidir; yeniden yazılmış sınıfların ikili uyumluluğunu kanıtlamaz. x64 portu ilk kabul kapsamı dışındadır.

### İlk doğrulama için seçilen araç zinciri

Bunlar mevcut kurulum veya başarılı derleme iddiası değil, ilk Windows denemesi için tabandır:

- Ana makine: Windows 10/11 x64; hedef işlemci: x86.
- Visual Studio 2022 veya Build Tools 2022, **Desktop development with C++** bileşenleri.
- MSVC **v143**, x86/x64 derleme araçları. Kesin 19.xx sürümü ilk koşunun loguna kaydedilmeli; gerçek SDK uyumu henüz bilinmiyor.
- Windows SDK **10.0.22621.0**; komutta açıkça seçilir. Başka sürüm gerekirse bilinçli değişiklik ve yeni doğrulama kaydı gerekir.
- CMake **3.25 veya üzeri**; `Visual Studio 17 2022` generator'ü (CMake 3.21+ gerektirir). Proje dosyasındaki 3.16 minimumu eski generator'lerle proje sözdizimi alt sınırıdır, bu reçetenin minimumu değildir.
- Dil standardı: C++14. İlk yapılandırma: **Debug**, ardından aynı kaynaklarla **Release**.
- Ninja/MinGW bu ilk tabanın parçası değildir. CRT seçimi mevcut CMake/MSVC varsayılanına bırakılır; SDK bağlantısından önce Debug/Release, `/MD`–`/MT`, çağrı kuralları, packing ve yapı boyutları ayrıca kararlaştırılmalıdır.

Tam araç yama sürümleri sabitlenmeden bit düzeyinde yeniden üretilebilirlik iddia edilmez. Bu reçete tekrarlanabilir temiz derleme sürecini tanımlar. Eski proprietary SDK'lar farklı bir derleyici/ABI gerektirebilir; v143 henüz SDK uyumluluğu doğrulanmış seçim değildir.

## 2. Bağımlılık envanteri

### RenderWare / RenderWare Studio

- Yerel stub başlıkları: `code/rwcore.h`, `rpworld.h`, `rpcollision.h`, `rphanim.h`, `rpskin.h`, `rtanim.h`, `rtcmpkey.h`, `rt2d.h`, `rt2danim.h`.
- `rwcore.h` kendini minimal 3.x tür/imza stub'ı olarak tanımlıyor. Proje açıklamalarındaki 3.6 sürümü gerçek SDK arşiviyle doğrulanmış değil.
- Framework kaynakları `code/framework/` altında. Windows başlangıç kodu harici motor girişlerine ihtiyaç duyuyor.
- Çalışma alanında RenderWare/Havok için `.lib` arşivi bulunmadı; bulunan `.a`, önceki macOS oyun kütüphanesi. Gerçek SDK'nın başka yerde varlığı araştırılmadı.
- Stub başlıkları derleyiciye bildirim sağlar; fonksiyon uygulaması, doğru layout ve binary uyumu sağlamaz. Gerçek SDK eklenirken yerel stub'ların include sırasıyla SDK başlıklarını gölgelemesi önlenmelidir.

### Havok

- `PhysicsWorld.h` dünya/gövde nesnelerini `void*` olarak tutuyor; `PhysicsWorld.cpp` içindeki `stepDeltaTime` çağrısı yorum satırı.
- `HavokMemoryManager.cpp` tahsis yardımcısı mevcut; bu, fizik SDK entegrasyonu değildir.
- Havok sürümü, başlıkları, x86 kütüphaneleri ve uyumlu ABI **belirlenmedi**. Kaynaktaki `hkpWorld` yorumundan kesin SDK sürümü çıkarılmamalı.
- Sonraki iş: kullanılan tür/fonksiyon envanteri ve derleme stub'ları; gerçek simülasyon için SDK veya uyumluluk katmanı kararı ayrı tutulmalı.

### Windows / grafik / diğer çalışma zamanı bileşenleri

- `windows.h`, Win32 pencere ve registry kaynakları Windows SDK gerektiriyor.
- `Ocean_W32.cpp` şu anda RenderWare Im3D API'si kullanıyor; doğrudan `d3d9.h` include'u yok. Legacy DirectX SDK'nın gerekli olduğu henüz gösterilmedi; otomatik kurulum yapılmamalı.
- `DLLs/` altında D3D9, BINK32 ve Windows DLL'leri bulunması geliştirme başlıkları/import `.lib` dosyaları veya dağıtım izni bulunduğunu kanıtlamaz. Bunları sistem klasörlerine kopyalamayın.
- `startup.cpp` içinde `HavokInit`, `FileSystemInit`, `TextureCacheInit`, `RwStartEngine` gibi extern bildirimler var. Bunlar tam eksik sembol listesi değildir; EXE hedefi ve gerçek link denemesiyle envanter çıkarılmalı.
- SDK lisansı/erişimi ve orijinal asset'lerin kullanımı ayrıca doğrulanmalı. Bu P0 kapsamında indirme, kurulum veya SDK seçimi yapılmadı.

## 3. Mevcut yerel kanıt ve sınırları

9 Ekim 2026 incelemesinde:

- `code/build/CMakeCache.txt`: Unix Makefiles, `/usr/bin/c++`, boş `CMAKE_BUILD_TYPE`.
- `code/build/CMakeFiles/4.4.3/CMakeCXXCompiler.cmake`: AppleClang 21.0.0.21000101, x86_64 ve 8 bayt pointer.
- `code/build/libCannonCruiseLib.a`: mevcut; içeriği/güncel kaynaklarla eşleşmesi doğrulanmadı. Önceki yol haritası macOS derlemesini başarılı bildirmişti; bu koşuda yeniden çalıştırılmadı.
- Non-Windows CMake dalı `framework/startup/win32/*`, `framework/startup/startup.cpp` ve `Ocean_W32.cpp` dosyalarını hariç tutuyor. Dolayısıyla macOS sonucu Windows kapsamını test etmez.
- `code/build/CMakeFiles/` altında dört üretilmiş `.c`/`.cpp` tanılama dosyası bulundu. Recursive kaynak glob'una yanlışlıkla girmemeleri için CMake'te `build/` alt dizinleri hariç tutuldu; önceki çıktılar silinmedi veya taşınmadı.
- Bu oturumun worker PATH'inde `python3` ve `clang` bulundu; `cmake`, `cl` ve `ninja` bulunmadı. Kurulum ve PATH değişikliği yapılmadı. Cache'teki eski CMake yolu, çalışan worker için kullanılabilirlik kanıtı değildir.

## 4. Temiz Windows derleme reçetesi

**Önkoşul:** Yukarıdaki araçlar Windows makinesinde kurulmuş olmalı. Eksikse önce kullanıcı onayıyla kurulmalı; aşağıdaki komutlar kurulum yapmaz. Bu komutlar Windows için hazırlanmıştır, mevcut macOS oturumunda doğrulanmamıştır.

Visual Studio'nun **Developer PowerShell for VS 2022** terminalinde depo köküne geçin (`code/` değil). Yeni bir build dizini kullanın; mevcut `code/build` cache'ini Windows'ta yeniden kullanmayın. Tüm yeni çıktılar depo kökündeki `build/` altında, CMake kaynak ağacı `code/` dışında kalır.

```powershell
# Çalışma dizini: ROADMAP.md ve code/ içeren depo kökü.
$ErrorActionPreference = 'Stop'
if (-not (Test-Path '.\code\CMakeLists.txt')) {
    throw 'Komutları depo kökünden çalıştırın.'
}
Get-Command cmake, cl, msbuild -ErrorAction Stop | Out-Null

# Önceki çıktıları silmeden benzersiz, temiz dizin.
$build = Join-Path $PWD ('build\win32-v143-' + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $build | Out-Null
cmake --version | Tee-Object -FilePath "$build\tools.log"
if ($LASTEXITCODE -ne 0) { throw 'CMake sürüm sorgusu başarısız.' }
# cl /Bv çıktısını yapılandırma/derleme loguyla birlikte ayrıca kaydedin.
# Parametresiz kaynak derlemesi yapmadığı için cl /Bv sıfır dışı dönebilir.
cmd /c 'cl /Bv 2>&1' | Out-File -Append "$build\tools.log"

cmake -S code -B "$build" -G 'Visual Studio 17 2022' -A Win32 -T v143 `
    -DCMAKE_SYSTEM_VERSION=10.0.22621.0 2>&1 |
    Tee-Object -FilePath "$build\configure.log"
if ($LASTEXITCODE -ne 0) { throw "Yapılandırma başarısız; $build\configure.log dosyasını inceleyin." }

cmake --build "$build" --config Debug --target CannonCruiseLib --parallel 2>&1 |
    Tee-Object -FilePath "$build\build-debug.log"
if ($LASTEXITCODE -ne 0) { throw "Debug derlemesi başarısız; $build\build-debug.log dosyasını inceleyin." }

cmake --build "$build" --config Release --target CannonCruiseLib --parallel 2>&1 |
    Tee-Object -FilePath "$build\build-release.log"
if ($LASTEXITCODE -ne 0) { throw "Release derlemesi başarısız; $build\build-release.log dosyasını inceleyin." }
```

Visual Studio çoklu yapılandırma generator'ünde `CMAKE_BUILD_TYPE` yerine `--config` kullanılır. SDK veya v143 bulunamadığında sürümü sessizce değiştirmeyin; eksik bileşeni ve kararı kaydedin. Tekrar temiz deneme için komut bloğunu yeniden çalıştırmak yeni bir dizin üretir, eskisini korur.

**Başarılı olursa beklenen çıktılar:** `$build\Debug\CannonCruiseLib.lib` ve `$build\Release\CannonCruiseLib.lib`. Bunlar statik arşivdir; çalıştırılabilir oyun değildir. Mevcut tür/imza açıkları nedeniyle ilk deneme hata verebilir; reçete başarı garantisi değildir.

## 5. İlk Windows koşusunun kabul/kayıt ölçütleri

- [ ] Kaynak revizyonu ve varsa yerel değişiklikler, Windows sürümü, CMake/MSVC tam sürümleri, v143 ve seçilen SDK kaydedildi.
- [ ] Cache/proje dosyaları Win32/x86 hedefi ve `10.0.22621.0` SDK seçimini doğruluyor.
- [ ] Kaynak listesinde Windows başlangıç dosyaları ve `Ocean_W32.cpp` mevcut; hiçbir CMake CompilerId kaynağı yok.
- [ ] Yapılandırma ve Debug/Release derlemeleri sıfır çıkış koduyla tamamlandı; her iki `.lib` mevcut.
- [ ] Loglar korundu, uyarılar ve başarısızlıklar sınıflandırıldı (başlık/bağımlılık, tür/imza, platform/ABI).
- [ ] `ROADMAP.md` 4.4b yalnızca bu kanıtlarla güncellendi; 4.5 link ve çalışma zamanı durumları ayrı bırakıldı.

Sonraki aşamada yürütülebilir hedef eklendiğinde link logu, import kütüphaneleri, çözümlenmemiş semboller, SDK/CRT uyumu ve uygulama başlatma sonucu ayrıca kaydedilmeli. Statik arşiv başarısı, çalışır fizik/render veya orijinal binary eşdeğerliği anlamına gelmez.

## 6. P0 devamı — SDK arayüz önkontrolü

[SDK_INTERFACE_AUDIT.md](SDK_INTERFACE_AUDIT.md), 9 Ekim devam incelemesinin açıklarını, sınırlı düzeltmelerini ve doğrulama sınırlarını kaydeder. `tools/check_sdk_interfaces.py` ile CMake gerektirmeyen Clang syntax kontrolü eklenmiştir; Windows reçetesinin yerine geçmez.

- Non-Windows kaynaklar ayrı, `Ocean_W32.cpp` ek kontrol olarak, startup sözleşmeleri ise izole parçalar halinde değerlendirilir.
- Im3D bildirimleri ve parametresiz `RwEngineOpen()` çağrısı hâlâ açık derleme engelleridir.
- Yerel Havok allocator testi Debug/Release ASan+UBSan ile geçti; Havok SDK veya Windows/x86 testi değildir.
- Açık engeller bulunduğunda önkontrol nonzero döner; raporlarda kaynak SHA-256, komutlar ve tanılar korunur. Kurulum yapmaz.
