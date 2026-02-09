cmake_minimum_required(VERSION 3.22)
project(assignments C CXX)

# Standards festlegen
set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

# Alle C- und C++-Dateien in Unterordnern finden
file(GLOB_RECURSE SRC_C "${CMAKE_SOURCE_DIR}/*.c")
file(GLOB_RECURSE SRC_CPP "${CMAKE_SOURCE_DIR}/*.cpp")

# Dummy-Target nur für Codeanalyse in CLion
add_executable(dummy_target ${SRC_C} ${SRC_CPP})
