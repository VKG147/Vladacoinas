cmake_minimum_required(VERSION 3.9)
project (vladacoinas)

find_package(OpenMP)

add_executable(main main.cpp vhasher.h vcoin.h)

if(OpenMP_CXX_FOUND)
    target_link_libraries(main PUBLIC OpenMP::OpenMP_CXX)
endif()

set(CMAKE_CXX_FLAGS "-O3")



