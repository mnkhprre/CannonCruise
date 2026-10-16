# CannonCruise PowerShell Build Script
# Visual Studio 18 Community (MSVC v145) + CMake + Ninja
# Created: October 15, 2026

$ErrorActionPreference = "Stop"

# Path definitions
$VS_ROOT = "C:\Program Files\Microsoft Visual Studio\18\Community"
$MSVC_VERSION = "14.51.36231"
$MSVC_BIN = "$VS_ROOT\VC\Tools\MSVC\$MSVC_VERSION\bin\Hostx86\x86"
$CMAKE_BIN = "$VS_ROOT\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin"
$PROJECT_ROOT = "C:\Users\mnkhprre\Documents\GitHub\CannonCruise"
$CODE_DIR = "$PROJECT_ROOT\code"
$BUILD_DIR = "$CODE_DIR\build_win32_v180"
$PLATFORM = "NULL"

Write-Host "======================================"
Write-Host "CannonCruise Build Script"
Write-Host "======================================"
Write-Host ""
Write-Host "Project root: $PROJECT_ROOT"
Write-Host "Build dir: $BUILD_DIR"
Write-Host "Platform: $PLATFORM"
Write-Host ""

# 1. Setup environment
Write-Host "[1/6] Setting up environment..."
$env:PATH = "$MSVC_BIN;$CMAKE_BIN;$env:PATH"

# 2. Verify tools
Write-Host "[2/6] Verifying tools..."
$cmakePath = "$CMAKE_BIN\cmake.exe"
$clPath = "$MSVC_BIN\cl.exe"

if (Test-Path $cmakePath) {
    Write-Host "  CMake found: $cmakePath"
} else {
    Write-Host "  ERROR: CMake not found!"
    exit 1
}

if (Test-Path $clPath) {
    Write-Host "  cl.exe found: $clPath"
} else {
    Write-Host "  ERROR: cl.exe not found!"
    exit 1
}

# 3. Prepare build directory
Write-Host "[3/6] Preparing build directory..."
if (Test-Path $BUILD_DIR) {
    Write-Host "  Removing old build directory..."
    Remove-Item -Recurse -Force $BUILD_DIR
}
New-Item -ItemType Directory -Path $BUILD_DIR | Out-Null
Write-Host "  Created: $BUILD_DIR"

# 4. CMake configure
Write-Host "[4/6] CMake configure..."
$cmakeArgs = @(
    "-S", $CODE_DIR,
    "-B", $BUILD_DIR,
    "-G", "Visual Studio 18 2026",
    "-A", "Win32",
    "-DCANNONCRUISE_RW_PLATFORM=$PLATFORM"
)

Write-Host "  Running: cmake configure"
& $cmakePath @cmakeArgs

if ($LASTEXITCODE -ne 0) {
    Write-Host "  ERROR: CMake configure failed!"
    exit 1
}
Write-Host "  OK: Configure complete"

# 5. Build
Write-Host "[5/6] Building..."
$buildArgs = @(
    "--build", $BUILD_DIR,
    "--config", "Release",
    "--parallel", "4"
)

Write-Host "  Running: cmake build"
& $cmakePath @buildArgs

if ($LASTEXITCODE -ne 0) {
    Write-Host "  ERROR: Build failed!"
    exit 1
}
Write-Host "  OK: Build complete"

# 6. Copy DLLs
Write-Host "[6/6] Copying DLLs..."
$dllSource = "$PROJECT_ROOT\DLLs"
$dllDest = "$BUILD_DIR\Release"
if (Test-Path $dllSource) {
    Copy-Item "$dllSource\*" $dllDest -Force
    Write-Host "  OK: DLLs copied"
}

Write-Host ""
Write-Host "======================================"
Write-Host "BUILD SUCCESSFUL!"
Write-Host "======================================"
Write-Host ""
Write-Host "To run:"
Write-Host "  cd $BUILD_DIR\Release"
Write-Host "  CannonCruise.exe"
Write-Host ""
Write-Host "To run tests:"
Write-Host "  cd $BUILD_DIR"
Write-Host "  ctest --output-on-failure"
Write-Host ""
