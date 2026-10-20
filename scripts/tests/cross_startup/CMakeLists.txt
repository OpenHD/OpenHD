cmake_minimum_required(VERSION 3.16)
project(cross_startup LANGUAGES CXX)
add_executable(cross_startup main.cpp)
target_compile_features(cross_startup PRIVATE cxx_std_17)
