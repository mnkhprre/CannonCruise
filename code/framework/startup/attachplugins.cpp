// ============================================================================
// CannonCruise - RenderWare Plugin Attachment (attachplugins.cpp)
// Orijinal dosya: D:\Projects\CannonCruisePC\code\framework\startup\attachplugins.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// FUN_00419e40 — AttachPlugins()
// Bu fonksiyon, oyun motorunun kullandığı tüm RenderWare 3.6 eklentilerini
// başlatır ve motora bağlar (attach). Uygulama başlangıcında, RwEngineInit()
// çağrısından önce çalıştırılır.
//
// Binary Adresleri:
//   AttachPlugins = 0x00419e40
//   Plugin tablosu = PTR_FUN_005ef440 (5 fonksiyon işaretçisi)
//   RtAnimInitialize = 0x005a7180 (rwID 0x1b7)
//   RtAnimRegisterScheme = 0x005a8b30
// ============================================================================

#include <cassert>

// RenderWare 3.6 SDK Headers
#include <rwcore.h>         // RwEngineRegisterPlugin, vs.
#include <rpworld.h>        // RpWorldPluginAttach
#include <rphanim.h>        // RpHAnimPluginAttach (rwID = 0x116)
#include <rpcollision.h>    // RpCollisionPluginAttach (rwID = 0x11e)
#include <rpskin.h>         // RpSkinPluginAttach (rwID = 0x10c)
#include <rtanim.h>         // RtAnimInitialize (rwID = 0x1b7)
#include <rtcmpkey.h>       // RtAnimRegisterInterpolationScheme (compressed keyframe)

// Oyun-spesifik eklenti (FUN_0042b880)
// D:\Projects\CannonCruisePC\code\framework içinden gelir
extern RwBool GamePluginAttach(void);

// ============================================================================
// Plugin Fonksiyon Tablosu (PTR_FUN_005ef440)
// Binary'de 0x005ef440 adresinde 5 fonksiyon işaretçisi bulunur.
// Döngü pcVar9 = 0'dan 4'e kadar (do-while, pcVar9 < 5) iterasyon yapar.
//
// Tablo Sırası ve RW Plugin ID Eşlemesi:
//   [0] FUN_004dd760 = RpWorldPluginAttach   (sub-IDs: 0x501–0x50b)
//   [1] FUN_0059f690 = RpHAnimPluginAttach   (rwID = 0x116)
//   [2] FUN_005a44a0 = RpCollisionPluginAttach (rwID = 0x11e)
//   [3] FUN_005a65a0 = RpSkinPluginAttach    (rwID = 0x10c)
//   [4] FUN_0042b880 = GamePluginAttach      (oyun-spesifik)
// ============================================================================

// Plugin attach fonksiyonları — hepsi RwBool (başarı=TRUE, hata=FALSE) döner
typedef RwBool (*PluginAttachFunc)(void);

static PluginAttachFunc s_pluginTable[] = {
    RpWorldPluginAttach,        // [0] 0x004dd760 — World sistemi
    RpHAnimPluginAttach,        // [1] 0x0059f690 — Hiyerarşik animasyon
    RpCollisionPluginAttach,    // [2] 0x005a44a0 — Çarpışma sistemi
    RpSkinPluginAttach,         // [3] 0x005a65a0 — Karakter skin/mesh
    GamePluginAttach,           // [4] 0x0042b880 — Oyun-spesifik eklenti
};

static const int NUM_PLUGINS = sizeof(s_pluginTable) / sizeof(s_pluginTable[0]);

// ============================================================================
// AttachPlugins (FUN_00419e40)
// Binary Adresi: 0x00419e40
// XREF: FUN_0041a680:0041aa84(c) — startup.cpp'den çağrılır
//
// Akış:
//   1. RtAnimInitialize() çağır (0x005a7180)
//      -> Başarısızsa assert: "Unable to initialize RtAnim" (line 0xdf = 223)
//   2. Plugin tablosunda 5 eklenti için döngü:
//      -> Her biri için CALL [EDI*4 + PTR_FUN_005ef440]
//      -> Başarısızsa assert: "Unable to Attach Plugin: Index [N]" (line 0xf9 = 249)
//   3. RtAnimRegisterInterpolationScheme() çağır (0x005a8b30)
//      -> Başarısızsa assert: "Compressed keyframe scheme registration failed" (line 0x10e = 270)
//   4. Başarılı dönüş
// ============================================================================
RwBool AttachPlugins(void)
{
    // ---------------------------------------------------------------
    // Adım 1: RtAnim Modülünü Başlat
    // ASM: 00419e72  CALL FUN_005a7180
    //       00419e77  TEST EAX,EAX
    //       00419e79  JNZ  LAB_00419fce (başarılıysa döngüye atla)
    // Başarısızsa: assert "Unable to initialize RtAnim" (satır 0xdf = 223)
    // ---------------------------------------------------------------
    RwBool result = RtAnimInitialize();
    assert(result && "Unable to initialize RtAnim");

    // ---------------------------------------------------------------
    // Adım 2: Plugin Tablosu Döngüsü
    // ASM: LAB_00419fce — EDI=0, EBX=DAT_005ef324
    //       LAB_00419fdf — CALL dword ptr [EDI*4 + PTR_FUN_005ef440]
    //       pcVar9++ ; pcVar9 < 5 ise tekrarla
    // Başarısızsa: assert "Unable to Attach Plugin: Index [N]" (satır 0xf9 = 249)
    // ---------------------------------------------------------------
    for (int i = 0; i < NUM_PLUGINS; i++)
    {
        result = s_pluginTable[i]();
        assert(result && "Unable to Attach Plugin: Index [i]");
    }

    // ---------------------------------------------------------------
    // Adım 3: Sıkıştırılmış Keyframe Animasyon Şemasını Kaydet
    // ASM: 0041a165  CALL FUN_005a8b30
    //       0041a16a  TEST EAX,EAX
    //       0041a16c  JNZ  (başarılıysa devam)
    // Başarısızsa: assert "Compressed keyframe scheme registration failed" (satır 0x10e = 270)
    // ---------------------------------------------------------------
    result = RtAnimRegisterInterpolationScheme(
        RtCompressedKeyFrameGetCustomKeyFrameScheme()
    );
    assert(result && "Compressed keyframe scheme registration failed");

    return TRUE;
}
