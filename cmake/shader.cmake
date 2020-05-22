add_custom_target(compile_shader ALL)

macro(target_add_shader target shader)
    file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/res/)

    set(shader_source ${PROJECT_SOURCE_DIR}/res/${shader}.glsl)
    set(shader_binary ${PROJECT_BINARY_DIR}/res/${shader}.spv)

    add_custom_command(TARGET compile_shader
            PRE_BUILD
            COMMAND glslangValidator -V -o ${shader_binary} ${shader_source}
            BYPRODUCTS ${shader_source})
endmacro()