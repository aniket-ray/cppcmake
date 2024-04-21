cmake_minimum_required(VERSION 3.27)
project(CppCmake)

set(CMAKE_CXX_STANDARD 20)

add_library(CppCmake SHARED library.cpp)
