# Apparition (solution root)
cmake_minimum_required (VERSION 3.13)

project(base)

add_definitions(-pthread)

# see if we can use 20, else fall back to 17
#set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_COMPILER_ARCHITECTURE_ID "x64")

# look in /usr/local/lib/cmake
#find_package(wt REQUIRED)

add_subdirectory(src)

