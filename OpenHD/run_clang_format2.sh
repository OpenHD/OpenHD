mkdir clang_tidy_tmp_build

cd clang_tidy_tmp_build

cmake -G Ninja -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
clang-tidy-11 -format-style=file -header-filter=. -p build