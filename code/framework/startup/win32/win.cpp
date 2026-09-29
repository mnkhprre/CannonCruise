#include "win.h"
#include <windows.h>
#include <cassert>

// ============================================================================
// CannonCruise - Framework Startup Win32 (win.cpp)
// Orijinal dosya: D:\Projects\CannonCruisePC\code\framework\startup\win32\win.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

// Global Değişkenler (Hafıza adresleri: DAT_00635acc, DAT_00635ad0, DAT_00635ad1)
HWND theMainWindow    = NULL;   // DAT_00635acc
static HINSTANCE ghInstance  = NULL;   // DAT_00635ac8
static BOOL bSuspended        = FALSE;  // DAT_00635ad0 (Pencere odağı kaybedildiğinde askıya alma)
static BOOL bQuit             = FALSE;  // DAT_00635ad1 (Oyun döngüsünden çıkış bayrağı)
static DWORD gLastFrameTime   = 0;      // DAT_00635690 (Kare zamanlayıcısı)

// Sabitler (Binary string havuzundan)
static const char* const APP_CLASS_NAME  = "CannonCruise";             // 0x005ef420
static const char* const APP_WINDOW_NAME = "CannonCruise";             // 0x005ef430
static const char* const APP_MUTEX_NAME  = "CANNONCRUISE";             // 0x00613094
static const char* const APP_ATOM_NAME   = "cannoncruise";             // 0x00612fe4
static const char* const REG_KEY_APP     = "Software\\ITE\\CannonCruise"; // 0x00613078
static const char* const REG_VAL_WIDTH   = "VideoResWidth";            // 0x00613068
static const char* const REG_VAL_HEIGHT  = "VideoResHeight";           // 0x00613058
static const char* const REG_VAL_DEPTH   = "VideoResDepth";            // 0x00613048

// Harici Motor Geri Çağrıları (startup.cpp / RenderWare motor bağlantıları)
extern void Idle();
extern void ResizeCameraRasters(int width, int height, int depth);
extern void CleanupEngine(HWND hWnd);
extern BOOL InitEngine(HWND hWnd, HINSTANCE hInstance, int width, int height, int depth, BOOL bFullscreen);
extern DWORD GetTime();

// ----------------------------------------------------------------------------
// MainWndProc (0x00418230): Ana Pencere Mesaj Prosedürü
// ----------------------------------------------------------------------------
LRESULT CALLBACK MainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_ACTIVATEAPP:
        // Uygulama odağını kaybettiğinde çizimi durdur ve bekleme moduna geç
        bSuspended = (wParam == 0);
        return 0;

    case WM_SIZE:
        // Pencere boyutu değiştiğinde RenderWare kamera rasterlarını güncelle
        if (wParam != SIZE_MINIMIZED) {
            int width  = LOWORD(lParam);
            int height = HIWORD(lParam);
            ResizeCameraRasters(width, height, 32);
        }
        return 0;

    case WM_KEYDOWN:
        // Hile / konsol tuş kombinasyonu kaydı (A-Z arası tuşlar)
        if (wParam >= 'A' && wParam <= 'Z') {
            // Dahili tampon kaydı
        }
        return 0;

    case WM_CLOSE:
        // Motor kaynaklarını temizle ve pencereyi sonlandır
        CleanupEngine(hWnd);
        DestroyWindow(hWnd);
        return 0;

    case WM_DESTROY:
        // Mesaj döngüsünü bitir
        theMainWindow = NULL;
        PostQuitMessage(0);
        return 0;

    default:
        break;
    }
    return DefWindowProcA(hWnd, uMsg, wParam, lParam);
}

// ----------------------------------------------------------------------------
// InitApp (FUN_00418420): Pencere Sınıfını Kaydeder (RegisterClass)
// ----------------------------------------------------------------------------
BOOL InitApp(HINSTANCE hInstance) {
    // Assert kontrolü (0x00612fb8)
    assert(hInstance != NULL && "Failed PRE-condition: processInstance");

    ghInstance = hInstance;

    WNDCLASSA wc;
    ZeroMemory(&wc, sizeof(wc));
    wc.style         = CS_BYTEALIGNCLIENT;
    wc.lpfnWndProc   = MainWndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    wc.hIcon         = LoadIconA(hInstance, MAKEINTRESOURCE(101)); // IDI_MAIN_ICON (0x65)
    wc.hCursor       = LoadCursorA(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = APP_CLASS_NAME;

    return (RegisterClassA(&wc) != 0);
}

// ----------------------------------------------------------------------------
// CreateMainWindow (FUN_00418700): Ana Oyun Penceresini Oluşturur
// ----------------------------------------------------------------------------
HWND CreateMainWindow(HINSTANCE hInstance, int x, int y, int width, int height, BOOL bFullscreen) {
    assert(hInstance != NULL && "Failed PRE-condition: processInstance");

    DWORD dwStyle = 0;
    if (bFullscreen) {
        dwStyle = WS_POPUP;
    } else {
        dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
        RECT rect = { 0, 0, width, height };
        AdjustWindowRect(&rect, dwStyle, FALSE);
        width  = rect.right - rect.left;
        height = rect.bottom - rect.top;
    }

    HWND hWnd = CreateWindowExA(
        0,
        APP_CLASS_NAME,
        APP_WINDOW_NAME,
        dwStyle,
        x, y,
        width, height,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    return hWnd;
}

// ----------------------------------------------------------------------------
// MainLoop (FUN_004189e0): Ana Oyun Mesaj ve Kare Döngüsü
// ----------------------------------------------------------------------------
int MainLoop() {
    MSG msg;
    while (!bQuit) {
        Sleep(0); // İşlemci zaman dilimini bırak

        if (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                bQuit = TRUE;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        } else {
            if (bSuspended) {
                // Pencere simge durumundayken veya odaksızken CPU harcamamak için bekle
                WaitMessage();
            } else {
                // Oyun çizimi ve güncelleme adımı
                Idle();

                // Kare hızı sabitleyici (Kare süresi < 10ms ise uyu -> Max ~100 FPS)
                DWORD currentTime = GetTime();
                DWORD elapsed = currentTime - gLastFrameTime;
                if (elapsed < 10) {
                    Sleep(10 - elapsed);
                }
                gLastFrameTime = GetTime();
            }
        }
    }
    return (int)msg.wParam;
}

// ----------------------------------------------------------------------------
// GetMainWindow & QuitApp (FUN_00418c70 & FUN_00418c80)
// ----------------------------------------------------------------------------
HWND GetMainWindow() {
    return theMainWindow;
}

void QuitApp() {
    if (theMainWindow) {
        PostMessageA(theMainWindow, WM_CLOSE, 0, 0);
    }
}

// ----------------------------------------------------------------------------
// AppMain (FUN_00418ce0): Uygulama Başlatma ve Yapılandırma Mantığı
// ----------------------------------------------------------------------------
int AppMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // 1. Zaten çalışan başka bir kopya var mı kontrolü
    if (FindWindowA(APP_CLASS_NAME, APP_WINDOW_NAME) != NULL) {
        return -1;
    }

    // 2. Pencere sınıfını kaydet
    if (!InitApp(hInstance)) {
        return -1;
    }

    // 3. Kayıt defterinden (Registry) video ayarlarını oku
    int width  = 640;
    int height = 480;
    int depth  = 16;
    BOOL bFullscreen = TRUE;

    HKEY hKey;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, REG_KEY_APP, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        DWORD dwType, dwVal, dwSize = sizeof(dwVal);
        if (RegQueryValueExA(hKey, REG_VAL_WIDTH, NULL, &dwType, (LPBYTE)&dwVal, &dwSize) == ERROR_SUCCESS) {
            width = (int)dwVal;
        }
        dwSize = sizeof(dwVal);
        if (RegQueryValueExA(hKey, REG_VAL_HEIGHT, NULL, &dwType, (LPBYTE)&dwVal, &dwSize) == ERROR_SUCCESS) {
            height = (int)dwVal;
        }
        dwSize = sizeof(dwVal);
        if (RegQueryValueExA(hKey, REG_VAL_DEPTH, NULL, &dwType, (LPBYTE)&dwVal, &dwSize) == ERROR_SUCCESS) {
            depth = (int)dwVal;
        }
        RegCloseKey(hKey);
    }

    // Tam ekranda imleci gizle
    if (bFullscreen) {
        ShowCursor(FALSE);
    }

    // 4. Ana pencereyi oluştur
    theMainWindow = CreateMainWindow(hInstance, 0, 0, width, height, bFullscreen);
    if (!theMainWindow) {
        return -1;
    }

    // Pencereli modda ekranın ortasına konumlandır
    if (!bFullscreen) {
        int screenW = GetSystemMetrics(SM_CXSCREEN);
        int screenH = GetSystemMetrics(SM_CYSCREEN);
        int posX = (screenW - width) / 2;
        int posY = (screenH - height) / 2;
        SetWindowPos(theMainWindow, NULL, posX, posY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    }

    ShowWindow(theMainWindow, nCmdShow);
    UpdateWindow(theMainWindow);

    // 5. RenderWare ve motor sistemlerini başlat
    if (!InitEngine(theMainWindow, hInstance, width, height, depth, bFullscreen)) {
        DestroyWindow(theMainWindow);
        return -1;
    }

    // 6. Ana oyun döngüsüne gir
    int exitCode = MainLoop();

    // 7. Döngüden çıkış sonrası denetim
    assert(theMainWindow == NULL && "theMainWindow hasn't been destroyed");

    return exitCode;
}

// ----------------------------------------------------------------------------
// WinMain (FUN_00418ca0): Win32 Giriş Noktası
// ----------------------------------------------------------------------------
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    ATOM atom = GlobalAddAtomA(APP_ATOM_NAME);
    int result = AppMain(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
    GlobalDeleteAtom(atom);
    return result;
}
