project(demo)
cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -std=c++17")

set(CMAKE_BUILD_TYPE Debug)

include_directories(./hdr)

add_executable(demo
	src/example.cpp
)
