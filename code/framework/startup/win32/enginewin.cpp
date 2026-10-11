// ============================================================================
// CannonCruise - Win32 engine köprü fonksiyonları (enginewin.cpp)
// Orijinal dosya: bilinmiyor (win.cpp'in beklediği motor tarafı sembolleri)
// Tersine mühendislik ile CC_ida.asm analiz edilerek yeniden yazılmıştır.
//
// win32/win.cpp bu beş sembolü extern ile bekler; daha önce hiçbir kaynak
// dosyada tanımları yoktu (Faz 4.4b link engeli). Binary eşdeğerleri:
//   FUN_0041C380 — Idle               (MainLoop::Poll; sub_4189E0 çağırır)
//   FUN_00418130 — ResizeCameraRasters (WM_SIZE; MainWndProc jumptable case 5)
//   FUN_00417930 — CleanupEngine       (WM_CLOSE; MainWndProc jumptable case 16)
//   FUN_00482BE0 — GetTime             (MainLoop kare zamanlayıcısı)
//   InitEngine  — AppMain'in motor başlatma çağrısı; binaryde karşılığı
//                 sub_41A680 = RwStartup::Initialize'dır ve startup.cpp'de
//                 RwStartupInitialize olarak restor edilmiştir.
//
// Not: Bu dosya yalnızca Windows'ta derlenir (CMakeLists win32/* hariç
//      tutar); macOS oturumunda derleme/doğrulama yapılamaz — Faz 4.4b
//      reçetesi (BUILDING.md §4) ile denenir.
// ============================================================================

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>   // timeGetTime

#include "rwcore.h"
#include "eventhandler/ceventhandler.h"
#include "rwreg.h"

#ifdef _MSC_VER
#pragma comment(lib, "winmm.lib")
#endif

// startup.cpp (FUN_0041A680 — RwStartup::Initialize; yalnızca Windows derlenir)
extern RwBool RwStartupInitialize(void *memFuncs, void *subSystemConfig,
                                  RwVideoMode *videoMode, RwUInt32 flags,
                                  int colorDepth, HWND hWnd);

// ---------------------------------------------------------------------------
// GetTime (FUN_00482BE0)
// Binary: mov eax, dword_657AA8 ; retn
//   dword_657AA8, mantık adımı (sub_41B900 → sub_481660) tarafından yazılan
//   önbellekli milisaniye sayacıdır. Mantık adımı Faz 6'ya kadar restor
//   edilmediği için doğrudan WINMM saati okunur — binary ile aynı ms
//   birimi ve aynı sözleşme (kare hızı sabitleyici MainLoop'ta kullanır).
// ---------------------------------------------------------------------------
DWORD GetTime()
{
    return timeGetTime();
}

// ---------------------------------------------------------------------------
// Idle (FUN_0041C380) — "MainLoop::Poll"
// Binary sub_4189E0 (MainLoop) her karede çağırır. Yapısal iskelet:
//   1. "MainLoop::Poll" profil bölümü — sub_49C7D0 kaydı + atexit;
//      Begin sub_49C940 / End sub_49C9B0 (CRealtimeProfiler modeli)
//   2. Varlık sayaç logları (değişince): bird  dword_657178,
//      fish dword_6574E8, soldier dword_6582A0, flotsam dword_65763C,
//      ship dword_6580C0, towercatapult dword_658438,
//      "MAX CannonBall" dword_6572F4
//   3. "Logic" — saat sub_428DE0 (TextureCacheInit / WINMM) ile gecikme
//      yakalanır; sub_41B900 sabit adımlı mantık çağrısı (≥33ms gecikmede
//      1000 birim/adım, en fazla 10000 birim = 10 sn tavan)
//   4. "Render" — sub_41CEB0 render adımı (çift sarmalı Begin/End)
//   5. Motor singleton'ı (dword_635AF4) varsa sub_49CA00 (gösterim)
// Adımlar 2-5 için gerekli oyun mantığı/render alt sistemleri kod
// tabanında yok: şu an iş yapmadan döner.
// ---------------------------------------------------------------------------
void Idle()
{
    // TODO(Faz 6): adımlar 1-5 — profil bölümleri (CRealtimeProfiler),
    // varlık sayaç logları, mantık adımı (sub_41B900), render adımı
    // (sub_41CEB0) ve motor gösterimi (sub_49CA00).
}

// ---------------------------------------------------------------------------
// ResizeCameraRasters (FUN_00418130) — WM_SIZE
// Binary akışı:
//   1. ctx = sub_41CDF0() = dword_6361B4 — sub_41CE00'un oluşturduğu render
//      hedefi (RenderPipelineInit adımı 1). NULL ise FALSE döner ve mesaj
//      gönderilmez. Bu nesne Faz 4.4b'de restor edilecek; şimdilik geçici
//      karşılık: rwstubs engine durum makinesi START'ta olmalı.
//   2. width/height 0 değilse (minimize değilse): ctx+68h/+6Ch mevcut
//      boyutlar alınır; sub_510660 / sub_50E6E0 raster yeniden
//      boyutlandırma ve sub_429B10 kamera aspect/derinlik ayarı çağrılır
//      — raster/kamera nesneleri restor edilmediği için TODO.
//   3. Her yolda (0 boyutta dahil): "iMsgResizeCameraRasters" geçici kaydedilir
//      (loc_40BF90 = RegisterMsg), abone varsa gönderilir (sub_40E0F0 =
//      SendMsg), kayıt silinir (loc_40CC60 = UnRegisterMsg).
// ---------------------------------------------------------------------------
BOOL ResizeCameraRasters(int width, int height, int depth)
{
    // Adım 1 — ctx denetimi (binary: dword_6361B4 NULL → FALSE, mesaj yok)
    if (!RwEngineIsRunning())
        return FALSE;

    if (width != 0 && height != 0)
    {
        (void)depth;
        // TODO(Faz 4.4b): sub_510660 / sub_50E6E0 — raster yeniden
        // boyutlandırma (ctx+68h/+6Ch mevcut boyutlarla); sub_429B10 —
        // kamera aspect/derinlik. Raster ve kamera nesneleri yok.
    }

    // Adım 3 — iMsgResizeCameraRasters (binary: loc_40BF90 → sub_40E0F0 →
    // loc_40CC60); abone yoksa SendMsg zaten no-op.
    RWS::CEventId id =
        RWS::CEventHandler::RegisterMsg("iMsgResizeCameraRasters");
    if (id)
    {
        RWS::CMsg msg(id, 0, 0);
        RWS::CEventHandler::SendMsg(msg);
        RWS::CEventHandler::UnRegisterMsg(id);
    }
    return TRUE;
}

// ---------------------------------------------------------------------------
// CleanupEngine (FUN_00417930) — WM_CLOSE
// Binary: GetWindowPlacement ile pencere durumunu okur ve rwreg kökü
//   (Software\Criterion\RenderWare\ — rwreg.cpp 0x00612e88) altındaki
//   GameFrameWork/GameSettings anahtarına yazar:
//     PosX, PosY  — rcNormalPosition.left / .top (int)
//     SizeX, SizeY — genişlik / yükseklik (int, sub_417740 ile string'e
//                    çevrilir — RwRegSetInt aynı işi yapar)
//     Minimized   — showCmd == SW_SHOWMINIMIZED (2) → "true"/"false"
//     Maximized   — showCmd == SW_MAXIMIZE (3)      → "true"/"false"
//   Motor kapatma YOKTUR: sub_417930 yalnız pencere durumunu kaydeder;
//   kapatma ayrı zincirdir (RwStartupShutdown, FUN_0041AB90).
//   path/subKey sırası sub_416B90 bağlamından %100 ayırt edilemedi;
//   okuma tarafı (window restore) henüz yazılmadığı için restore
//   yazılırken doğrulanacak.
// ---------------------------------------------------------------------------
void CleanupEngine(HWND hWnd)
{
    if (!hWnd)
        return;

    WINDOWPLACEMENT wndpl;
    ZeroMemory(&wndpl, sizeof(wndpl));
    wndpl.length = sizeof(WINDOWPLACEMENT);
    if (!GetWindowPlacement(hWnd, &wndpl))
        return;

    static const char *const kPath = "GameFrameWork";
    static const char *const kSubKey = "GameSettings";

    const RECT &rc = wndpl.rcNormalPosition;
    RwRegSetInt(kPath, kSubKey, "PosX", rc.left);
    RwRegSetInt(kPath, kSubKey, "PosY", rc.top);
    RwRegSetInt(kPath, kSubKey, "SizeX", rc.right - rc.left);
    RwRegSetInt(kPath, kSubKey, "SizeY", rc.bottom - rc.top);
    RwRegSetString(kPath, kSubKey, "Minimized",
                   wndpl.showCmd == SW_SHOWMINIMIZED ? "true" : "false");
    RwRegSetString(kPath, kSubKey, "Maximized",
                   wndpl.showCmd == SW_MAXIMIZE ? "true" : "false");
}

// ---------------------------------------------------------------------------
// InitEngine — win.cpp motor başlatma köprüsü (AppMain adımı 5)
// Binary: AppMain (FUN_00418ce0) → sub_41A680 = RwStartup::Initialize.
// Tam zincir startup.cpp'de RwStartupInitialize olarak restor edilmiştir:
//   HavokInit → FileSystemInit → TextureCacheInit → RwEngineInit →
//   RwEngineSetSubSystem(-1) → RwEngineOpen → RwStartEngine →
//   AttachPlugins → RwEngineSetVideoMode → CameraInit → RwEngineStart →
//   RwEngineSetConfig → SetLightAmbient → RenderPipelineInit → GameManagerInit
// win.cpp'in 6 argümanı köprüde haritalanır:
//   width/height/depth → RwVideoMode (startup registry'den okunan değerler
//   win.cpp AppMain'de zaten okunmuştu), colorDepth = depth, hWnd doğrudan.
// hInstance/bFullscreen kullanılmaz: pencere çoktan oluşturulmuştur
//   (sub_418700) ve startup.cpp s_fullScreen'i binary ile aynı şekilde
//   sabit 1'e kurar — tam ekran akışı ayrı bir iş kalemidir (Faz 4.4b).
// ---------------------------------------------------------------------------
BOOL InitEngine(HWND hWnd, HINSTANCE hInstance, int width, int height,
                int depth, BOOL bFullscreen)
{
    (void)hInstance;
    (void)bFullscreen;

    RwVideoMode videoMode;
    ZeroMemory(&videoMode, sizeof(videoMode));
    videoMode.width = width;
    videoMode.height = height;
    videoMode.depth = depth;

    return RwStartupInitialize(0, 0, &videoMode, 0, depth, hWnd) ? TRUE
                                                                 : FALSE;
}
