cmake_minimum_required(VERSION 3.10)
project(glsl-raytracing)

include(cmake/vulkan.cmake)
include(cmake/shader.cmake)

add_executable(glsl-raytracing src/main.cpp res/compile.vert.glsl)
add_vulkan_support(glsl-raytracing)
target_add_shader(glsl-raytracing compile.vert)
target_add_shader(glsl-raytracing compile.frag)