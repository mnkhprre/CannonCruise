@echo off
REM CannonCruise CMD Build Script
REM Visual Studio 18 Community + CMake
REM Guncellenme: 15 Ekim 2026

echo ======================================
echo CannonCruise Build Script
echo ======================================
echo.

REM Yol tanimlari
set VS_ROOT=C:\Program Files\Microsoft Visual Studio\18\Community
set MSVC_VERSION=14.51.36231
set MSVC_BIN=%VS_ROOT%\VC\Tools\MSVC\%MSVC_VERSION%\bin\Hostx86\x86
set CMAKE_BIN=%VS_ROOT%\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin
set PROJECT_ROOT=C:\Users\mnkhprre\Documents\GitHub\CannonCruise
set CODE_DIR=%PROJECT_ROOT%\code
set BUILD_DIR=%CODE_DIR%\build_win32_v180
set PLATFORM=NULL

echo Proje koku: %PROJECT_ROOT%
echo Build dizini: %BUILD_DIR%
echo Platform: %PLATFORM%
echo.

REM 1. Environment ayarla
echo [1/6] Environment ayarlaniyor...
set PATH=%MSVC_BIN%;%CMAKE_BIN%;%PATH%

REM 2. Araclari dogrula
echo [2/6] Araclar dogrulaniyor...
"%CMAKE_BIN%\cmake.exe" --version
if errorlevel 1 (
    echo HATA: CMake bulunamadi!
    pause
    exit /b 1
)

echo.
echo [3/6] Build dizini hazirlaniyor...
if not exist "%BUILD_DIR%" (
    mkdir "%BUILD_DIR%"
    echo   OK: Build dizini olusturuldu: %BUILD_DIR%
)

REM 4. CMake configure
echo.
echo [4/6] CMake configure...
"%CMAKE_BIN%\cmake.exe" ^
    -S "%CODE_DIR%" ^
    -B "%BUILD_DIR%" ^
    -G "Visual Studio 18 2026" ^
    -A Win32 ^
    -DCANNONCRUISE_RW_PLATFORM=%PLATFORM%

if errorlevel 1 (
    echo HATA: CMake configure basarisiz!
    pause
    exit /b 1
)
echo   OK: CMake configure tamamlandi

REM 5. Build
echo.
echo [5/6] Derleme yapiliyor...
"%CMAKE_BIN%\cmake.exe" ^
    --build "%BUILD_DIR%" ^
    --config Release ^
    --parallel 4

if errorlevel 1 (
    echo HATA: Derleme basarisiz!
    pause
    exit /b 1
)
echo   OK: Derleme tamamlandi

REM 6. DLL kopyala
echo.
echo [6/6] DLL kopyalama ve tamamlama...
set DLL_SOURCE=%PROJECT_ROOT%\DLLs
set DLL_DEST=%BUILD_DIR%\Release
if exist "%DLL_SOURCE%" (
    if not exist "%DLL_DEST%" mkdir "%DLL_DEST%"
    copy "%DLL_SOURCE%\*" "%DLL_DEST%" /Y >nul
    echo   OK: DLL'ler kopyalandi
)

echo.
echo ======================================
echo BUILD BASARILI!
echo ======================================
echo.
echo Calistirmak icin:
echo   cd "%BUILD_DIR%\Release"
echo   CannonCruise.exe
echo.
echo Testleri calistirmak icin:
echo   "%CMAKE_BIN%\ctest.exe" --test-dir "%BUILD_DIR%" -C Release --output-on-failure
echo.

set /p RUN_TESTS="Testleri calistirmak ister misiniz? (E/H): "
if /i "%RUN_TESTS%"=="E" (
    echo Testler calistiriliyor...
    "%CMAKE_BIN%\ctest.exe" --test-dir "%BUILD_DIR%" -C Release --output-on-failure
)

echo.
echo Build script tamamlandi.
pause