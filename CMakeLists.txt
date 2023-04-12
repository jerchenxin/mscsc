cmake_minimum_required(VERSION 3.10)
project(DCCM)

#set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "-O3")
# set(CMAKE_CXX_FLAGS "-g")

# include_directories(/usr/include)

add_executable(DCCM test.cpp graph.cpp tarjan.cpp ReducedGraph.cpp timer.cpp)

