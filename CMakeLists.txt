cmake_minimum_required(VERSION 3.10)

project(glsl-raytracing)

set(SRCS
        src/device.cpp
        src/main.cpp)
add_executable(demo ${SRCS})

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    target_include_directories(demo
            PRIVATE
            $ENV{VK_SDK_PATH}/Include)
    target_link_directories(demo
            PRIVATE
            $ENV{VK_SDK_PATH}/Lib)
    target_link_libraries(demo vulkan-1)
    target_compile_definitions(demo
            PRIVATE
            -DVK_USE_PLATFORM_WIN32_KHR)
endif()

# compile shaders
add_custom_target(shader
        COMMAND glslangValidator -V ${CMAKE_CURRENT_SOURCE_DIR}/shaders/trace.comp.glsl -o ${CMAKE_CURRENT_BINARY_DIR}/trace.comp.spv
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/shaders/trace.comp.glsl
        COMMENT "compile shader")

# demo
add_dependencies(demo shader)