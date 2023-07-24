# bin/bash

cmake -S . -B build_debug -DCMAKE_BUILD_TYPE=Release
cmake --build build_release
