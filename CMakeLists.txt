cmake_minimum_required(VERSION 3.10)
project(glsl-raytracing)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(3rdparty)

# compile res
add_custom_target(shader
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/res
        COMMAND glslangValidator -V ${CMAKE_CURRENT_SOURCE_DIR}/res/trace.comp.glsl -o ${CMAKE_CURRENT_BINARY_DIR}/res/trace.comp.spv
        COMMAND glslangValidator -V ${CMAKE_CURRENT_SOURCE_DIR}/res/display.vert.glsl -o ${CMAKE_CURRENT_BINARY_DIR}/res/display.vert.spv
        COMMAND glslangValidator -V ${CMAKE_CURRENT_SOURCE_DIR}/res/display.frag.glsl -o ${CMAKE_CURRENT_BINARY_DIR}/res/display.frag.spv
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/res/trace.comp.glsl ${CMAKE_CURRENT_SOURCE_DIR}/res/display.frag.glsl ${CMAKE_CURRENT_SOURCE_DIR}/res/display.vert.glsl
        COMMENT "compile shader")

set(SRCS
        src/util.cpp
        src/scene.cpp
        src/render.cpp
        src/app.cpp
        src/main.cpp
        src/camera.cpp
        src/bvh.cpp)
add_executable(glsl-raytracing ${SRCS})

target_include_directories(glsl-raytracing
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/glm/)

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    target_include_directories(glsl-raytracing
            PRIVATE
            $ENV{VK_SDK_PATH}/Include)
    target_link_directories(glsl-raytracing
            PRIVATE
            $ENV{VK_SDK_PATH}/Lib)
    target_link_libraries(glsl-raytracing vulkan-1)
    target_compile_definitions(glsl-raytracing
            PRIVATE
            -DDEBUG
            -DVK_USE_PLATFORM_WIN32_KHR)
endif ()

# demo
add_dependencies(glsl-raytracing shader)
target_link_libraries(glsl-raytracing glfw vkut)