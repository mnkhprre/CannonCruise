cmake_minimum_required(VERSION 3.16)
project(CannonCruise C CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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

# Exclude platform-specific win32 files when compiling on non-Windows platforms
if(NOT WIN32)
    list(FILTER SOURCES EXCLUDE REGEX ".*/framework/startup/win32/.*")
    list(FILTER SOURCES EXCLUDE REGEX ".*/framework/startup/startup\\.cpp")
    list(FILTER SOURCES EXCLUDE REGEX ".*/Ocean_W32\\.cpp")
endif()

# Build as a static library for syntax and linking validation
add_library(CannonCruiseLib STATIC ${SOURCES})

# Platform-specific definitions
if(WIN32)
    add_definitions(-DWIN32 -D_WINDOWS)
else()
    add_definitions(-D_POSIX_C_SOURCE=200809L)
endif()
