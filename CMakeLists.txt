cmake_minimum_required(VERSION 3.10)
project(glsl-raytracing)

add_subdirectory(3rdparty)

include(cmake/vulkan.cmake)
include(cmake/shader.cmake)

set(SRCS
        # headers
        src/util.h
        src/vkut.h
        src/camera.h
        src/app.h
        src/scene.h
        src/render.h
        src/bvh.h

        # sources
        src/util.cpp
        src/vkut.cpp
        src/camera.cpp
        src/app.cpp
        src/scene.cpp
        src/render.cpp
        src/bvh.cpp

        # entry point
        src/main.cpp)
add_executable(glsl-raytracing ${SRCS})
target_link_libraries(glsl-raytracing glfw)
add_vulkan_support(glsl-raytracing)
target_add_shader(glsl-raytracing compile.vert)
target_add_shader(glsl-raytracing compile.frag)