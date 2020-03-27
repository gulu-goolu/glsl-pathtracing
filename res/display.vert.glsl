#version 450 core

vec2 vertices[] = {
    vec2(0, -0.707),
    vec2(0.707, 0.707),
    vec2(-0.707, 0.707),
};

void main() {
    gl_Position = vec4(vertices[gl_VertexIndex], 0, 1);
}
