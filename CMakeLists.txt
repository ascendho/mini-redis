cmake_minimum_required(VERSION 4.0)
project(mini_redis)

set(CMAKE_CXX_STANDARD 20)

# 添加头文件搜索路径
include_directories(code/include)

# 添加可执行文件，包含所有源文件
add_executable(mini_redis 
    code/src/hashtable.cpp
    code/src/08_server.cpp
)
