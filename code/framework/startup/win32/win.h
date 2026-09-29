#pragma once

#ifndef _WIN_H_
#define _WIN_H_

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// Ana pencere handle'ı (Binary DAT_00635acc)
extern HWND theMainWindow;

// Pencere ve döngü fonksiyonları
HWND GetMainWindow();
void QuitApp();
BOOL InitApp(HINSTANCE hInstance);
HWND CreateMainWindow(HINSTANCE hInstance, int x, int y, int width, int height,
                      BOOL bFullscreen);
int MainLoop();
LRESULT CALLBACK MainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam,
                             LPARAM lParam);
int AppMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine,
            int nCmdShow);

#endif // _WIN_H_
