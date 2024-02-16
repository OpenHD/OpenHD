#!/bin/bash

# Create a directory where we can build with ninja & generate the list of files for clang tidy
cmake -G Ninja -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
clang-tidy -format-style=file -header-filter=. -p build
