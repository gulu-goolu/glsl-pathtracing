cmake_minimum_required(VERSION 3.10)
project(glsl-raytracing)

include(cmake/vulkan.cmake)

add_executable(glsl-raytracing src/main.cpp)
add_vulkan_support(glsl-raytracing)
