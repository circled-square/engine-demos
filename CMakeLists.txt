cmake_minimum_required(VERSION 3.25)
enable_testing()

project(engine-demo VERSION 0.1.0 LANGUAGES CXX C)
set(CMAKE_CXX_STANDARD 23)

add_subdirectory(deps/)
add_subdirectory(src/)

