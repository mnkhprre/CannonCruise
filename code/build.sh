#!/bin/sh
# CannonCruise altyapı derleme betiği — stale-binary tuzağına karşı:
# link öncesi eski exe'ler silinir (| head SIGPIPE + eksik relink).
set -e
cd "$(dirname "$0")"
rm -f build/CannonCruise build-gl3/CannonCruise
cmake --build build -j8
cmake --build build-gl3 -j8
ctest --test-dir build
