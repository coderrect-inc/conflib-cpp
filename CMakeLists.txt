cmake_minimum_required(VERSION 3.10)

project(conflib_cpp)

set(CMAKE_CXX_STANDARD 17)


include_directories(include third-party/jsoncons)

file(GLOB SOURCES "examples/*.cpp")

add_executable(testconflib ${SOURCES})