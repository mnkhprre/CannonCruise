// ============================================================================
// CannonCruise - Startup Manager (startup.cpp)
// Orijinal dosya: D:\Projects\CannonCruisePC\code\framework\startup\startup.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// FUN_0041a680 — RwStartup::Initialize()
// FUN_0041ab90 — RwStartup::Shutdown()
// FUN_0041a640 — RwStartup::SetLightAmbient()  (yardımcı)
//
// Bu dosya, RenderWare 3.6 motorunun üst-düzey başlatma ve kapatma 
// sürecini yönetir. Win32 penceresi oluşturulduktan sonra çağrılır.
//
// Binary Akışı (FUN_0041a680):
//   1. Havok fizik sistemi singleton kontrolü (DAT_00635af4)
//   2. Dosya sistemi başlatma (FUN_005ca6c0)
//   3. Texture dictionary/cache başlatma (FUN_00428de0)
//   4. RwEngineInit (FUN_0050b640) — motor çekirdeği
//   5. RwEngineSetSubSystem (FUN_004cab40) — grafik alt sistemi
//   6. RwEngineOpen (FUN_00422960) — motor aç
//      -> Başarısızsa assert: "RenderWare Audio not initialized"
//   7. Video modu & çözünürlük ayarları
//   8. RwEngineStart (FUN_00418ac0) — motor başlat
//   9. AttachPlugins (FUN_00419e40) — eklentileri bağla
//  10. RwEngineSetVideoMode (FUN_0050b440)
//  11. RwEngineSetCamera (FUN_0041af10) — kamera oluştur
//  12. RwEngineStart tamamla (FUN_0050b340)
//  13. Ambient ışık ayarla (FUN_0041a640)
//  14. Render pipeline başlat (FUN_0041c190)
//  15. Game manager başlat (FUN_0044aaf0)
// ============================================================================

#include "attachplugins.h"
#include "win32/win.h"

#include <rwcore.h>
#include <rpworld.h>
#include <cassert>

// RenderWare grafik subsystem ayarları
// Binary'deki global değişkenler:
//   DAT_00612f0c = grafik derinliği (varsayılan: 16 bit)
//   DAT_00612f10 = z-buffer derinliği (varsayılan: 16 bit) 
//   DAT_00612f14 = tam ekran bayrağı
//   DAT_00635acc = hwnd (pencere handle'ı)
//   DAT_00635af4 = Havok singleton işaretçisi
//   DAT_006356a0 = durum bayrağı
static int  s_colorDepth   = 16;    // DAT_00612f0c
static int  s_zBufferDepth = 16;    // DAT_00612f10
static int  s_fullScreen   = 0;     // DAT_00612f14
static HWND s_hWnd         = NULL;  // DAT_00635acc

// ============================================================================
// İleri tanımlı fonksiyonlar (framework modüllerinden)
// ============================================================================

// FUN_00434fb0 — Havok fizik sistemi singleton oluşturucu
extern void HavokInit(void);

// FUN_005ca6c0 — Dosya sistemi başlatma (RtFS veya custom)
extern void FileSystemInit(void);

// FUN_00428de0 — Texture dictionary/cache sistemi başlatma
extern RwBool TextureCacheInit(void);

// FUN_00418ac0 — RenderWare motor başlatma (RwEngineStart wrapper)
extern RwBool RwStartEngine(void);

// FUN_0041af10 — Kamera ve viewport başlatma
extern RwBool CameraInit(RwVideoMode* videoMode, int width, int height, int depth);

// FUN_0050b640 — RwEngineInit
extern RwBool RwEngineInit(void* memFuncs, RwUInt32 flags, RwUInt32 resEntrySize);

// FUN_0050b600 — RwEngineTerm (motor temizleme)
extern void RwEngineTerm(void);

// FUN_0050b440 — RwEngineSetVideoMode
extern RwBool RwEngineSetVideoMode(RwInt32 modeIndex);

// FUN_004cab40 — RwEngineSetSubSystem
extern RwBool RwEngineSetSubSystem(RwInt32 subSystemIndex);

// FUN_0050b340 — RwEngineStart
extern RwBool RwEngineStart(void);

// FUN_0050b3d0 — RwEngineStop
extern void RwEngineStop(void);

// FUN_0050b2f0 — RwEngineClose
extern void RwEngineClose(void);

// FUN_0050f660 — RwEngineSetZBufferDepth veya light config
extern void RwEngineSetConfig(float near_clip, float far_clip, float fog);

// FUN_0041a640 — Ortam ışığı oluştur ve sahneye ekle
extern void SetLightAmbient(void);

// FUN_0041c190 — Render pipeline başlat (viewport, framebuffer vs.)
extern void RenderPipelineInit(RwUInt32 width, RwUInt32 height, RwUInt32 flags);

// FUN_0044aaf0 — Oyun yöneticisi (GameManager) başlat
extern void GameManagerInit(void);

// FUN_0041ac00 — Kamera/sahne temizleme
extern void CameraShutdown(void);

// FUN_0041c270 — Render pipeline temizleme
extern void RenderPipelineShutdown(void);

// FUN_004cac40 — SubSystem temizleme
extern void SubSystemShutdown(void);

// FUN_004356f0 — Texture cache temizleme
extern void TextureCacheShutdown(void);

// FUN_0044aba0 — GameManager temizleme
extern void GameManagerShutdown(void);

// FUN_0040fd60 — Kaynak (resource) temizleme
extern void ResourceShutdown(void);

// FUN_00405670 — Genel temizleme (memory pool vs.)
extern void GeneralShutdown(void);

// Registry okuma — çözünürlük, renk derinliği ve tam ekran
// FUN_0059d4e0 — Registry nesne oluşturucu
// FUN_0059d4f0 — Registry aç ("Software\ITE\CannonCruise", READ, HKCU)
// FUN_0059d5b0 — Registry değer var mı diye kontrol et
// FUN_0059d5f0 — Registry tamsayı oku
// FUN_0059d590 — Registry kapat

// ============================================================================
// RwStartup::Initialize (FUN_0041a680)
// Binary Adresi: 0x0041a680
// XREF: FUN_00418ce0:004190c2(c) — WinMain'den çağrılır
//
// Parametreler (thiscall — ECX = this):
//   param_1: void* memFuncs — bellek fonksiyonları (veya callback tablosu)
//   param_2: void* subSystemConfig — alt sistem ayarları 
//   param_3: RwVideoMode* videoMode — video modu bilgisi (genişlik, yükseklik)
//   param_4: RwUInt32 flags — motor bayrakları (0x1000000)
//   param_5: int colorDepth — renk derinliği
//   param_6: HWND hWnd — pencere handle'ı
// ============================================================================
RwBool RwStartupInitialize(void* memFuncs, void* subSystemConfig,
                           RwVideoMode* videoMode, RwUInt32 flags,
                           int colorDepth, HWND hWnd)
{
    // -----------------------------------------------------------------
    // Adım 1: Havok Fizik Singleton Kontrolü
    // ASM: 0041a6a5  CALL FUN_00434fb0
    //       0041a6aa  CMP  [DAT_00635af4], 0
    //       0041a6b0  JZ   LAB_0041a7aa (başarılıysa atla)
    // Singleton zaten oluşturulduysa assert hatası verir.
    // -----------------------------------------------------------------
    HavokInit();
    // assert(DAT_00635af4 == NULL) — "ms_pSingleton == NULL Failed" (Singleton.h line 0x33)

    // -----------------------------------------------------------------
    // Adım 2: Registry'den Çözünürlük Ayarlarını Oku
    // ASM: "Software\ITE\CannonCruise" HKCU altından:
    //   - "Width"  (0x613068) → varsayılan 640 (0x280)
    //   - "Height" (0x613058) → varsayılan 480 (0x1e0)
    //   - "Depth"  (0x613048) → varsayılan 16
    // -----------------------------------------------------------------
    RwUInt32 width  = 640;   // 0x280
    RwUInt32 height = 480;   // 0x1e0
    s_colorDepth    = 16;
    s_zBufferDepth  = 16;

    // TODO: Registry okuma (FUN_0059d4f0, FUN_0059d5b0, FUN_0059d5f0)
    // "Software\ITE\CannonCruise" altından Width, Height, Depth oku
    // Başarılıysa değerleri güncelle, başarısızsa varsayılanları kullan

    s_fullScreen = 1;
    s_hWnd = hWnd;

    // -----------------------------------------------------------------
    // Adım 3: Dosya Sistemi & Texture Cache Başlat
    // ASM: 0041a7f8  CALL FUN_005ca6c0  (FileSystemInit)
    //       0041a7fd  CALL FUN_00428de0  (TextureCacheInit)
    // -----------------------------------------------------------------
    FileSystemInit();
    TextureCacheInit();

    // -----------------------------------------------------------------
    // Adım 4: RwEngineInit — Motor Çekirdeği Başlat
    // ASM: 0041a816  CALL FUN_0050b640
    //       0041a81e  CMP  EAX, 0
    //       0041a820  JNZ  (başarılıysa devam)
    // Başarısızsa: RwEngineTerm + return FALSE
    // -----------------------------------------------------------------
    if (!RwEngineInit(memFuncs, 0, 0))
    {
        RwEngineTerm();
        return FALSE;
    }

    // -----------------------------------------------------------------
    // Adım 5: Grafik Alt Sistemi Seç
    // ASM: 0041a830  CALL FUN_004cab40 — RwEngineSetSubSystem(-1)
    // -1 = varsayılan alt sistem (DEFAULT)
    // Başarısızsa: RwEngineTerm + return FALSE
    // -----------------------------------------------------------------
    if (!RwEngineSetSubSystem(-1))
    {
        RwEngineTerm();
        return FALSE;
    }

    // -----------------------------------------------------------------
    // Adım 6: RwEngineOpen — Motoru Aç
    // ASM: 0041a851  CALL FUN_00422960
    //       0041a856  CMP  EAX, 0
    //       0041a858  JNZ  LAB_0041aa59 (başarılıysa devam)
    // Başarısızsa: assert "RenderWare Audio not initialized" (satır 0x88)
    // -----------------------------------------------------------------
    RwBool openResult = RwEngineOpen();
    if (!openResult)
    {
        // Assert: "RenderWare Audio not initialized" (0x006131b0)
        assert(openResult && "RenderWare Audio not initialized");
        return FALSE;
    }

    // -----------------------------------------------------------------
    // Adım 7: RwEngineStart — Motoru Başlat
    // ASM: 0041aa66  CALL FUN_00418ac0
    //       0041aa6b  TEST EAX, EAX
    //       0041aa6d  JNZ  (başarılıysa devam)
    // Başarısızsa: RwEngineTerm + return FALSE
    // -----------------------------------------------------------------
    if (!RwStartEngine())
    {
        RwEngineTerm();
        return FALSE;
    }

    // -----------------------------------------------------------------
    // Adım 8: Plugin'leri Bağla
    // ASM: 0041aa84  CALL FUN_00419e40 (AttachPlugins)
    //       0041aa89  TEST AL, AL
    //       0041aa8b  JNZ  (başarılıysa devam)
    // Başarısızsa: RwEngineTerm + return FALSE
    // -----------------------------------------------------------------
    if (!AttachPlugins())
    {
        RwEngineTerm();
        return FALSE;
    }

    // -----------------------------------------------------------------
    // Adım 9: Video Modu Seç
    // ASM: 0041aab3  CALL FUN_0050b440 (RwEngineSetVideoMode)
    //       param_5 = colorDepth
    // Başarısızsa: RwEngineTerm + return FALSE
    // -----------------------------------------------------------------
    if (!RwEngineSetVideoMode(colorDepth))
    {
        RwEngineTerm();
        return FALSE;
    }

    // -----------------------------------------------------------------
    // Adım 10: Kamera Oluştur ve Viewport Ayarla
    // ASM: 0041aae5  CALL FUN_0041af10 (CameraInit)
    //       Parametreler: videoMode, width, height, depth (ESI'den)
    // Başarısızsa: RwEngineStop + RwEngineTerm + return FALSE
    // -----------------------------------------------------------------
    RwBool cameraOk = CameraInit(videoMode, width, height, colorDepth);
    if (!cameraOk)
    {
        RwEngineStop();
        RwEngineTerm();
        return FALSE;
    }

    // -----------------------------------------------------------------
    // Adım 11: Motor Başlatmayı Tamamla
    // ASM: 0041ab0b  CALL FUN_0050b340 (RwEngineStart)
    // Başarısızsa: RwEngineStop + RwEngineTerm + return FALSE
    // -----------------------------------------------------------------
    if (!RwEngineStart())
    {
        RwEngineStop();
        RwEngineTerm();
        return FALSE;
    }

    // -----------------------------------------------------------------
    // Adım 12: Z-Buffer & Clip Plane Yapılandırması
    // ASM: 0041ab26-0041ab37  float sabitler yükle:
    //   0x3727c5ac ≈ 0.00001f (near clip)
    //   0x3727c5ac ≈ 0.00001f (near clip duplicate)
    //   0x3c23d70a ≈ 0.01f    (far fog)
    // 0041ab3f  CALL FUN_0050f660
    // -----------------------------------------------------------------
    RwEngineSetConfig(0.00001f, 0.00001f, 0.01f);

    // -----------------------------------------------------------------
    // Adım 13: Ortam Işığı Ayarla
    // ASM: 0041ab4a  CALL FUN_0041a640
    // -----------------------------------------------------------------
    SetLightAmbient();

    // -----------------------------------------------------------------
    // Adım 14: Render Pipeline Başlat
    // ASM: 0041ab5d  CALL FUN_0041c190
    //       Parametreler: width, height, flags (ESI, EDI'den)
    // -----------------------------------------------------------------
    RenderPipelineInit(width, height, flags);

    // -----------------------------------------------------------------
    // Adım 15: Oyun Yöneticisi Başlat
    // ASM: 0041ab65  CALL FUN_0044aaf0
    // -----------------------------------------------------------------
    GameManagerInit();

    // ASM: 0041ab6a  MOV AL, 1  — başarılı dönüş
    return TRUE;
}

// ============================================================================
// RwStartup::Shutdown (FUN_0041ab90)
// Binary Adresi: 0x0041ab90
// XREF: FUN_00418ce0:00419832(c) — WinMain'den çağrılır (çıkışta)
//
// Kapatma sırası (tersine başlatma sırasıyla):
//   1. CameraShutdown        (FUN_0041ac00)
//   2. RenderPipelineShutdown (FUN_0041c270)
//   3. RwEngineClose          (FUN_0050b2f0)
//   4. RwEngineStop           (FUN_0050b3d0)
//   5. RwEngineTerm           (FUN_0050b600)
//   6. SubSystemShutdown      (FUN_004cac40)
//   7. TextureCacheShutdown   (FUN_004356f0)
//   8. GameManagerShutdown    (FUN_0044aba0)
//   9. FileSystemInit         (FUN_005ca6c0) — tekrar çağrılır (reset/cleanup)
//  10. ResourceShutdown       (FUN_0040fd60)
//  11. GeneralShutdown        (FUN_00405670)
// ============================================================================
void RwStartupShutdown(void)
{
    CameraShutdown();           // 0041ab91
    RenderPipelineShutdown();   // 0041ab96
    RwEngineClose();            // 0041ab9b
    RwEngineStop();             // 0041aba0
    RwEngineTerm();             // 0041aba5
    SubSystemShutdown();        // 0041abaa
    TextureCacheShutdown();     // 0041abaf
    GameManagerShutdown();      // 0041abb4
    FileSystemInit();           // 0041abb9 — dosya sistemi sıfırlama
    ResourceShutdown();         // 0041abbe
    GeneralShutdown();          // 0041abc3
}
