cmake_minimum_required(VERSION 3.10)
project(glsl-raytracing)

set(CMAKE_CXX_STANDARD 17)

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

# shader compilation
set(SHADER_SRCS
        rt.vert
        rt.frag)
set(SHADER_SOURCE_DIR ${CMAKE_SOURCE_DIR}/shader)
set(SHADER_BINARY_DIR ${CMAKE_BINARY_DIR}/shader)
add_custom_command(TARGET glsl-raytracing
        PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADER_BINARY_DIR})
foreach (SRC ${SHADER_SRCS})
    set(SHADER_SOURCE ${SHADER_SOURCE_DIR}/${SRC}.glsl)
    set(SHADER_BINARY ${SHADER_BINARY_DIR}/${SRC}.spv)
    add_custom_command(TARGET glsl-raytracing
            PRE_BUILD
            COMMAND glslangValidator -V -o ${SHADER_BINARY} ${SHADER_SOURCE})
endforeach ()
