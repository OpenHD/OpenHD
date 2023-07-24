# bin/bash

# convenient script to build this project with cmake and debugging enabled

cmake -S . -B build_debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build_debug
