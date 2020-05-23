cmake_minimum_required(VERSION 3.10)
project(glsl-raytracing)

add_subdirectory(3rdparty)

include(cmake/vulkan.cmake)
include(cmake/shader.cmake)

set(SRCS
        src/main.cpp
        src/util.h
        src/util.cpp
        src/device.h
        src/device.cpp
        src/camera.h
        src/camera.cpp
        src/app.h
        src/app.cpp
        src/scene.h
        src/scene.cpp
        src/render.h
        src/render.cpp
        src/bvh.h
        src/bvh.cpp)
add_executable(glsl-raytracing ${SRCS})
target_link_libraries(glsl-raytracing glfw)
add_vulkan_support(glsl-raytracing)
target_add_shader(glsl-raytracing compile.vert)
target_add_shader(glsl-raytracing compile.frag)