cmake_minimum_required(VERSION 3.26)
project(ecc)

set(CMAKE_CXX_STANDARD 17)

set(SOURCE_FILES main.cpp)
add_executable(ecc_run ${SOURCE_FILES})

include_directories(ecc_lib)
add_subdirectory(ecc_lib)

target_link_libraries(ecc_run ecc_lib)

add_subdirectory(gtest)