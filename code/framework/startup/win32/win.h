#pragma once

#ifndef _WIN_H_
#define _WIN_H_

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

/**
 * @file win.h
 * @brief Windows platformu için ana giriş noktası ve pencere yönetim işlevleri.
 *
 * CannonCruise'un PC sürümü için Win32 pencere oluşturma, mesaj döngüsü
 * ve genel uygulama yaşam döngüsü işlevlerini barındırır.
 */

/// Ana pencere handle'ı (Binary DAT_00635acc)
extern HWND theMainWindow;

/**
 * @brief Ana pencere handle'ını döndürür.
 * @return HWND — Uygulamanın ana penceresi.
 */
HWND GetMainWindow();

/**
 * @brief Uygulamadan çıkış yapılmasını (WM_QUIT gönderilmesini) sağlar.
 */
void QuitApp();

/**
 * @brief Windows uygulamasını başlatır (pencere sınıfı kaydı vb.)
 * @param hInstance Mevcut uygulama örneğinin handle'ı.
 * @return BOOL — Başarılıysa TRUE.
 */
BOOL InitApp(HINSTANCE hInstance);

/**
 * @brief Oyun için ana render penceresini oluşturur.
 * @param hInstance    Mevcut uygulama örneğinin handle'ı.
 * @param x            Pencerenin X koordinatı.
 * @param y            Pencerenin Y koordinatı.
 * @param width        Pencerenin genişliği.
 * @param height       Pencerenin yüksekliği.
 * @param bFullscreen  Tam ekran modunda mı açılacak?
 * @return HWND — Oluşturulan pencerenin handle'ı.
 */
HWND CreateMainWindow(HINSTANCE hInstance, int x, int y, int width, int height,
                      BOOL bFullscreen);

/**
 * @brief Ana Windows mesaj döngüsünü (Message Loop) işletir.
 * @return int — Çıkış kodu (WM_QUIT mesajındaki wParam).
 */
int MainLoop();

/**
 * @brief Ana pencere prosedürü (Window Procedure).
 *
 * Windows olaylarını (WM_PAINT, WM_DESTROY vb.) yakalayıp işler.
 */
LRESULT CALLBACK MainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam,
                             LPARAM lParam);

/**
 * @brief Uygulamanın asıl giriş noktası mantığını yürütür (WinMain tarafından çağrılır).
 */
int AppMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine,
            int nCmdShow);

#endif // _WIN_H_

