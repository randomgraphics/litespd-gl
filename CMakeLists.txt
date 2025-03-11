# litespd-gl is a header-only library. This cmake scripts is only to build samples and tests
cmake_minimum_required(VERSION 3.16)
project(litespd-gl)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
add_subdirectory(dev)
