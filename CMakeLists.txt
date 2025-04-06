cmake_minimum_required (VERSION 3.8)

project(dough LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SRCS 
src/dough.hpp
test/tests.cpp
)

add_executable ("${PROJECT_NAME}" ${SRCS})
add_compile_options(/utf-8)