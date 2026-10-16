# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "C:/Users/mnkhprre/Documents/GitHub/CannonCruise/code/build_win32_v180/_deps/librw-src")
  file(MAKE_DIRECTORY "C:/Users/mnkhprre/Documents/GitHub/CannonCruise/code/build_win32_v180/_deps/librw-src")
endif()
file(MAKE_DIRECTORY
  "C:/Users/mnkhprre/Documents/GitHub/CannonCruise/code/build_win32_v180/_deps/librw-build"
  "C:/Users/mnkhprre/Documents/GitHub/CannonCruise/code/build_win32_v180/_deps/librw-subbuild/librw-populate-prefix"
  "C:/Users/mnkhprre/Documents/GitHub/CannonCruise/code/build_win32_v180/_deps/librw-subbuild/librw-populate-prefix/tmp"
  "C:/Users/mnkhprre/Documents/GitHub/CannonCruise/code/build_win32_v180/_deps/librw-subbuild/librw-populate-prefix/src/librw-populate-stamp"
  "C:/Users/mnkhprre/Documents/GitHub/CannonCruise/code/build_win32_v180/_deps/librw-subbuild/librw-populate-prefix/src"
  "C:/Users/mnkhprre/Documents/GitHub/CannonCruise/code/build_win32_v180/_deps/librw-subbuild/librw-populate-prefix/src/librw-populate-stamp"
)

set(configSubDirs Debug)
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/Users/mnkhprre/Documents/GitHub/CannonCruise/code/build_win32_v180/_deps/librw-subbuild/librw-populate-prefix/src/librw-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/Users/mnkhprre/Documents/GitHub/CannonCruise/code/build_win32_v180/_deps/librw-subbuild/librw-populate-prefix/src/librw-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
