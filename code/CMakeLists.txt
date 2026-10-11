cmake_minimum_required(VERSION 3.16)
project(CannonCruise C CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---------------------------------------------------------------------------
# Bullet Physics — Havok yerine taşınabilir fizik motoru (Faz 4.6).
# Binary parametreleri (sub_55E6D0): gravity -9.8, AABB ±500, friction 0.6,
# restitution 0.3, solver iter 20; sabit adım 1/60 (sub_4B8710).
# Sadece gerekli alt kütüphaneler derlenir (LinearMath/Collision/Dynamics).
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
# Bullet 3.25 eski cmake_minimum_required içerir (CMake 4.x <3.5 kaldırır)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
FetchContent_MakeAvailable(bullet3)

# ---------------------------------------------------------------------------
# librw — RenderWare Graphics 3.x API yeniden implementasyonu (MIT).
# Kapı 4: gerçek .RWS asset akışı doğrulaması. NULL platform ile başlanır
# (cihaz bağımsız — stream/clump parse); render platformu (GL3/D3D9) sonraki
# iş kalemi. rwstubs.cpp'nin engine/stream sembolleri rw_librw.cpp'ye taşındı.
# Faz 4.6 (görsel deneme): -DCANNONCRUISE_RW_PLATFORM=GL3 → GLFW penceresi,
# gerçek piksel (ayrı build dizini: build-gl3/).
# ---------------------------------------------------------------------------
set(CANNONCRUISE_RW_PLATFORM "NULL" CACHE STRING "librw platform (NULL|GL3)")
set_property(CACHE CANNONCRUISE_RW_PLATFORM PROPERTY STRINGS NULL GL3)
FetchContent_Declare(librw
    GIT_REPOSITORY https://github.com/aap/librw.git
    # HEAD (2026-10): 18532c2 "build the ps2 platform entirely with freesce"
    GIT_TAG        18532c2e13efbc1aa43f0b00b4459831f9414de0
    GIT_SHALLOW    TRUE)
set(LIBRW_PLATFORM "${CANNONCRUISE_RW_PLATFORM}" CACHE STRING "" FORCE)
set(LIBRW_TOOLS    OFF   CACHE BOOL   "" FORCE)
set(LIBRW_INSTALL  OFF   CACHE BOOL   "" FORCE)
FetchContent_MakeAvailable(librw)

# GL3 platform — librw gl3-glfw cihazı pencereyi GLFW ile açar (brew:
# glfw). CannonCruiseLib'e PUBLIC: rw_librw/RenderPlatform glfw sembollerini
# kullanır; null build'de bu blok çalışmaz.
if(CANNONCRUISE_RW_PLATFORM STREQUAL "GL3")
    find_package(glfw3 REQUIRED)
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
    # Ocean_W32.cpp artık tüm platformlarda derlenir (Im3D stub'ları eklendi)
endif()

# Compile and archive sources; this does NOT validate external symbol resolution.
# Executable linking and runtime validation are separate, pending milestones.
add_library(CannonCruiseLib STATIC ${SOURCES})

# ---------------------------------------------------------------------------
# Faz 4.5 — Link doğrulama hedefi.
# main.cpp, statik kütüphanenin TÜM dış sembollerini çözdüğünü kanıtlamak
# için -force_load (tüm arşiv nesnelerini zorla dahil et) ile bağlanır.
# ---------------------------------------------------------------------------
add_executable(CannonCruise main.cpp)
# force_load dosya yolu kullandığı için hedef bağımlılığını ayrıca tanımla
add_dependencies(CannonCruise CannonCruiseLib)
if(APPLE)
    # -force_load: arşivdeki her nesne bağlanır → eksik sembol varsa hata verir
    target_link_options(CannonCruise PRIVATE
        "-Wl,-force_load,${CMAKE_CURRENT_BINARY_DIR}/libCannonCruiseLib.a")
else()
    # Not: diğer platformlarda doğrulama Windows (4.4b) / Linux adımlarında
    # --whole-archive veya /WHOLEARCHIVE ile tekrarlanacak.
    target_link_libraries(CannonCruise CannonCruiseLib)
endif()
# Bullet + librw fizik/motor kütüphaneleri — hem statik arşiv hem link doğrulama
# Not: Bullet/librw include'ları hedeflere INTERFACE olarak yayılmaz → elle
target_include_directories(CannonCruiseLib PUBLIC ${bullet3_SOURCE_DIR}/src)
target_link_libraries(CannonCruiseLib PUBLIC
    BulletDynamics BulletCollision LinearMath librw::librw)
if(CANNONCRUISE_RW_PLATFORM STREQUAL "GL3")
    target_link_libraries(CannonCruiseLib PUBLIC glfw)
endif()
target_include_directories(CannonCruise PRIVATE ${bullet3_SOURCE_DIR}/src)
target_link_libraries(CannonCruise PRIVATE
    BulletDynamics BulletCollision LinearMath librw::librw)

# Platform-specific definitions
if(WIN32)
    add_definitions(-DWIN32 -D_WINDOWS)
else()
    add_definitions(-D_POSIX_C_SOURCE=200809L)
endif()

# ---------------------------------------------------------------------------
# SDK bağımsız birim testleri (roadmap: test kapsamı).
# ctest --test-dir build --output-on-failure
# ---------------------------------------------------------------------------
enable_testing()
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../tests ${CMAKE_CURRENT_BINARY_DIR}/tests)
