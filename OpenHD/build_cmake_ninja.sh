# bin/bash

cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DCMAKE_GENERATOR:INTERNAL=Ninja
cmake --build build
