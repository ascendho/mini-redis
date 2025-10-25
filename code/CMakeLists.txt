cmake_minimum_required(VERSION 4.0)
project(code)

set(CMAKE_CXX_STANDARD 20)

add_executable(code
        include/hashtable.hpp
        src/hashtable.cpp
        src/server.cpp
        include/avl.hpp
        src/avl.cpp)
