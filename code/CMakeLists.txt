cmake_minimum_required(VERSION 3.16)
project(CannonCruise C CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# MSVC: conforming preprocessor (VS2017 15.8+)
# Traditional preprocessor breaks lambda syntax inside macros (C2059: '[')
# /Zc:preprocessor fixes this at root — RWS_REGISTER_CLASS uses lambda.
if(MSVC)
    add_compile_options(/Zc:preprocessor)
endif()

# ---------------------------------------------------------------------------
# Bullet Physics — portable physics engine instead of Havok (Phase 4.6).
# Binary parameters (sub_55E6D0): gravity -9.8, AABB ±500, friction 0.6,
# restitution 0.3, solver iter 20; fixed step 1/60 (sub_4B8710).
# Only required sub-libraries compiled (LinearMath/Collision/Dynamics).
# ---------------------------------------------------------------------------
include(FetchContent)
FetchContent_Declare(bullet3
    GIT_REPOSITORY https://github.com/bulletphysics/bullet3.git
    GIT_TAG        3.25
    GIT_SHALLOW    TRUE)
set(BUILD_UNIT_TESTS     OFF CACHE BOOL "" FORCE)
set(BUILD_EXTRAS         OFF CACHE BOOL "" FORCE)
set(BUILD_BULLET2_DEMOS  OFF CACHE BOOL "" FORCE)
set(BUILD_OPENGL3_DEMOS  OFF CACHE BOOL "" FORCE)
set(BUILD_CPU_DEMOS      OFF CACHE BOOL "" FORCE)
set(BUILD_PYBULLET       OFF CACHE BOOL "" FORCE)
set(BUILD_ENET           OFF CACHE BOOL "" FORCE)
set(BUILD_CLSOCKET       OFF CACHE BOOL "" FORCE)
set(USE_GLUT             OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS    OFF CACHE BOOL "" FORCE)
set(BULLET_REQUIRE_CXX_FLAGS OFF CACHE BOOL "" FORCE)
# Bullet 3.25 includes older cmake_minimum_required (CMake 4.x removes <3.5)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
FetchContent_MakeAvailable(bullet3)

# ---------------------------------------------------------------------------
# librw — RenderWare Graphics 3.x API reimplementation (MIT).
# Phase 4: real .RWS asset stream validation. Platform options:
#   NULL  — device-independent no-op (headless smoke test)
#   GL3   — GLFW + OpenGL 3.x (macOS/Linux build)
#   D3D9  — Direct3D 9 (Windows/MSVC build)
# ---------------------------------------------------------------------------
set(CANNONCRUISE_RW_PLATFORM "NULL" CACHE STRING "librw platform (NULL|GL3|D3D9)")
set_property(CACHE CANNONCRUISE_RW_PLATFORM PROPERTY STRINGS NULL GL3 D3D9)
FetchContent_Declare(librw
    GIT_REPOSITORY https://github.com/aap/librw.git
    # HEAD (2026-10): 18532c2 "build the ps2 platform entirely with freesce"
    GIT_TAG        18532c2e13efbc1aa43f0b00b4459831f9414de0
    GIT_SHALLOW    TRUE)
set(LIBRW_PLATFORM "${CANNONCRUISE_RW_PLATFORM}" CACHE STRING "" FORCE)
set(LIBRW_TOOLS    OFF   CACHE BOOL   "" FORCE)
set(LIBRW_INSTALL  OFF   CACHE BOOL   "" FORCE)
FetchContent_MakeAvailable(librw)

# Configuration by RenderWare platform selection:
#   NULL  — device-independent no-op (headless smoke test) — timeGetTime for WINMM
#   GL3   — GLFW + OpenGL 3.x (macOS/Linux build)
#   D3D9  — Direct3D 9 (Windows/MSVC build)
# ---------------------------------------------------------------------------
if(CANNONCRUISE_RW_PLATFORM STREQUAL "GL3")
    find_package(glfw3 REQUIRED)
elseif(CANNONCRUISE_RW_PLATFORM STREQUAL "D3D9")
    # Windows/MSVC for D3D9: no system package find in CMake;
    # -DD3D9 defined + d3d9.lib link sufficient (assumed in SDK).
endif()

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/framework
    ${CMAKE_CURRENT_SOURCE_DIR}/framework/core
    ${CMAKE_CURRENT_SOURCE_DIR}/framework/toolkits
)

# Source files (all game classes and framework sources)
file(GLOB_RECURSE SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
)

# Keep existing in-tree build artifacts (including CMake CompilerId sources)
# out of the source list. Use a fresh build directory outside code/; see BUILDING.md.
list(FILTER SOURCES EXCLUDE REGEX "/build.*")
list(FILTER SOURCES EXCLUDE REGEX "/main\\.cpp$")

# Exclude platform-specific win32 files when compiling on non-Windows platforms
if(NOT WIN32)
    list(FILTER SOURCES EXCLUDE REGEX ".*/framework/startup/win32/.*")
    list(FILTER SOURCES EXCLUDE REGEX ".*/framework/startup/startup\\.cpp")
    # Ocean_W32.cpp: Windows (NULL|D3D9); outside GL3 build.
    # NULL platform Im3D no-op; real D3D9 pipeline in D3D9/CannonCruise.
endif()

# Compile and archive sources; this does NOT validate external symbol resolution.
# Executable linking and runtime validation are separate, pending milestones.
add_library(CannonCruiseLib STATIC ${SOURCES})

# ---------------------------------------------------------------------------
# Phase 4.5 — Link verification target.
# main.cpp, statically proving all external symbols of CannonCruiseLib resolved
# via -force_load (forcing all archive objects included).
# ---------------------------------------------------------------------------
add_executable(CannonCruise main.cpp)
# force_load uses file path — define target dependency separately
add_dependencies(CannonCruise CannonCruiseLib)
# Forced inclusion: every object in archive linked → missing symbol link error.
# CMake 3.24+ portable path $<LINK_LIBRARY:WHOLE_ARCHIVE,...>:
#   Apple → -force_load, MSVC → /WHOLEARCHIVE:<lib>, GNU → --whole-archive
# NOT (why not bare /WHOLEARCHIVE): MSVC applies /WHOLEARCHIVE given
# without file to ALL subsequent libraries. Causes __NULL_IMPORT_DESCRIPTOR
# repetition for kernel32/user32/winmm/... and CannonCruiseLib not linked
# at all (Windows build error 2026-10-15).
target_link_libraries(CannonCruise PRIVATE
    "$<LINK_LIBRARY:WHOLE_ARCHIVE,CannonCruiseLib>")
# Platform-specific external libraries:
#   D3D9: Direct3D 9 SDK library
#   NULL: WINMM timing resolution (GetTime)
if(WIN32)
    if(CANNONCRUISE_RW_PLATFORM STREQUAL "D3D9")
        target_link_libraries(CannonCruise PRIVATE d3d9.lib)
    elseif(CANNONCRUISE_RW_PLATFORM STREQUAL "NULL")
        target_link_libraries(CannonCruise PRIVATE winmm.lib)
    endif()
endif()
# Bullet + librw physics/engine libraries — both static archive and link validation
# Note: Bullet/librw includes not propagated INTERFACE to targets → manually
target_include_directories(CannonCruiseLib PUBLIC ${bullet3_SOURCE_DIR}/src)
target_link_libraries(CannonCruiseLib PUBLIC
    BulletDynamics BulletCollision LinearMath librw::librw)
if(CANNONCRUISE_RW_PLATFORM STREQUAL "GL3")
    target_link_libraries(CannonCruiseLib PUBLIC glfw)
    target_include_directories(CannonCruise PRIVATE ${bullet3_SOURCE_DIR}/src)
    target_link_libraries(CannonCruise PRIVATE
        BulletDynamics BulletCollision LinearMath librw::librw)
endif()

# Platform-specific definitions
# ---------------------------------------------------------------------------
# Windows (MSVC) and RenderWare/D39 platform definitions
# ---------------------------------------------------------------------------
if(WIN32)
    add_definitions(-DWIN32 -D_WINDOWS)
    # RenderWare platform + DirectX 9 definitions
    if(CANNONCRUISE_RW_PLATFORM STREQUAL "D3D9")
        add_definitions(-DD3D9 -DDXROUTINE)
        target_link_libraries(CannonCruise PRIVATE d3d9.lib)
    elseif(CANNONCRUISE_RW_PLATFORM STREQUAL "NULL")
        target_link_libraries(CannonCruise PRIVATE winmm.lib)
    endif()
else()
    add_definitions(-D_POSIX_C_SOURCE=200809L)
endif()

# ---------------------------------------------------------------------------
# SDK-independent unit tests (roadmap: test coverage).
# ctest --test-dir build --output-on-failure
# ---------------------------------------------------------------------------
enable_testing()
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../tests ${CMAKE_CURRENT_BINARY_DIR}/tests)