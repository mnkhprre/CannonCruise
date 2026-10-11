# CannonCruise — Kod Stili Rehberi

> Faz 5.4 · Kaynak kodun mevcut konvansiyonlarını belgeler.
> Yeni kod bu rehberi izler; eski kodla çelişen yerlerde bu rehber esastır.

## 1. Temel kurallar

- **Standart:** C++14 (`CMakeLists.txt` içinde sabit).
- **Girinti:** Boşluk kullan, **tab yok**. 4 boşluk tercih edilir
  (framework kodu 4, bazı oyun sınıfları 2 kullanır — dosya içi tutarlılık
  bozulmadığı sürece mevcut girintiye sadık kalınır).
- **Kaynak kodu dosyaları:** UTF-8, LF sonlandırma.
- **Satır sonu:** Dosyanın geri kalanıyla aynı (mevcut dosyalar LF).

## 2. Dosya başlığı

Her `.cpp`/`.h` dosyası standart blokla açılır:

```cpp
// ============================================================================
// CannonCruise - <Bölüm> (<dosya adı>)
// Original path: D:\Projects\CannonCruisePC\code\...
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_0040b090 — CEventHandler vtable setup
// ============================================================================
```

## 3. İsimlendirme

| Öğe | Kural | Örnek |
|---|---|---|
| Sınıf | `C` öneki, PascalCase | `CShip`, `CPhysicsWorld` |
| Namespace (framework) | `RWS` | `RWS::CEventHandler` |
| Üye değişken | `m_` öneki + PascalCase; işaretçilerde `p` | `m_pClump`, `m_bInWorld` |
| Statik üye | `sm_` öneki | `sm_pInstance` |
| Global/statik yerel | `s_` öneki | `s_RegisteredEvents` |
| Parametre | `p` öneki (işaretçi) + camelCase | `pClump`, `deltaTime` |
| Yerel değişken | camelCase | `numVertices` |
| Sabit / makro | `rw`/`iMsg` öneki, ALL_CAPS karışık | `iMsgRunningTick`, `rwID_CLUMP` |
| Enum | `E` öneki + PascalCase | `ECannonballType` |
| Fonksiyon | PascalCase (C API'leri `RwXxxYyy`) | `HandleEvents`, `RpClumpRender` |

## 4. Stil

- **Süslü parantez:** Açılış parantezi tanım satırının sonunda (K&R) —
  oyun sınıflarında baskın stil:

  ```cpp
  void CShip::Initialize(RpClump *pClump) {
      assert(m_pClump == 0 && "Ship already has a clump");
  }
  ```

  Framework dosyalarında Allman görülebilir; yeni kod K&R kullanır.
- **Pointer/referans starı:** Değişken adına bitişik (`RpClump *pClump`,
  `const CMsg &msg`) — mevcut kodda bu yerleşim yaygındır.
- **Boş satır:** Fonksiyonlar arasında bölüm ayıracı
  (`// ---------...`) kullanılır; açıklayıcı Türkçe/İngilizce yorum serbesttir.
- **`//` yorum** gövde için, **`/** */`** başlık dosyalarındaki API için.

## 5. Dokümantasyon (Doxygen)

Başlık dosyalarında her sınıf/üye `@brief` ile belgelenir:

```cpp
/**
 * @brief Advances the physics simulation by deltaTime seconds.
 * @param deltaTime  Time step (the original game uses a fixed 1/60 s).
 */
void Update(RwReal deltaTime);
```

Üretim: `doxygen Doxyfile` → `docs/html/`.

## 6. Tersine mühendislik kanıtları

- Binary fonksiyon adresleri (`FUN_0040b090`) ve ASM referansları
  (`00419e72 CALL ...`) kod yorumlarında **korunur** — silinmez.
- Orijinal assert dizesi alıntıları İngilizce kalır:
  `"pRigidBody != NULL Failed"`.
- Bilinmeyen davranış `// TODO(Faz N+): ...` ile işaretlenir.

## 7. İşaretçi ve boş değerler

- Boş işaretçi: `0` (C++98 uyumlu; `nullptr` C++14'te mevcut olmakla
  birlikte mevcut kod `0` kullanır).
- `RwBool` dönüşler: `TRUE` / `FALSE` makroları.
- Opaque Havok/RW tipleri: `void*` + başlıkta tip yorumu.

## 8. Derleme / doğrulama

Yeni kod ekledikten sonra:

```bash
cd code
cmake --build build -j8          # statik kütüphane + link doğrulaması
./build/CannonCruise             # "link verification OK" beklenir
```

- Derleme hatasız olmalı; tanımsız sembol link'te `-force_load` ile yakalanır.
- Test: `tests/` altındaki bağımsız testler (bkz. README).

## 9. Yasaklar

- Sekme (tab) karakteri.
- Kaynak kodda `#ifdef _WIN32` çöpü — platform ayrımı zorunlu değilse yok.
- Binary kanıtı içeren yorumların silinmesi.
- Gerçek SDK sürümü doğrulanmadan `#include <gerçek-sdk-başlığı>` eklemek
  (yerel stub başlıklar gölgelenmemeli).
