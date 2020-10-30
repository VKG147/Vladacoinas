cmake_minimum_required(VERSION 3.0)
project (vladacoinas)
add_executable(main main.cpp vladahasher.cpp vladahasher.h)

set(CMAKE_CXX_FLAGS "-O3")