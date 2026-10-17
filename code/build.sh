#!/bin/sh
# CannonCruise infrastructure build script — guards against stale-binary trap:
# old exe files are deleted before linking (| head SIGPIPE + missing relink).
set -e
cd "$(dirname "$0")"
rm -f build/CannonCruise build-gl3/CannonCruise
cmake --build build -j8
cmake --build build-gl3 -j8
ctest --test-dir build
