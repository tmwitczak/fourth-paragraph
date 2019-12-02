cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
project(fourth-paragraph VERSION 1)

set(THIRDPARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty")

include(thirdparty/thirdparty.cmake)

add_subdirectory(src)

