#version 450 core

layout(location = 0) out vec2 outUV;

vec2 vertices[] = {
vec2(-1, -1), vec2(1, -1), vec2(1, 1),

vec2(1, 1), vec2(-1, 1), vec2(-1, -1),
};

vec2 calcUV(vec4 pos) {
    return vec2(pos.xy + 1) * 0.5f;
}

void main() {
    vec4 pos = vec4(vertices[gl_VertexIndex], 0, 1);
    gl_Position = pos;
    outUV = calcUV(pos);
}
