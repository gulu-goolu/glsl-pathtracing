#version 450 core

layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 outCol;
layout(set = 0, binding = 0) uniform sampler2D img;

layout(set = 1, binding = 0) uniform timesUniformBuffer {
    uint times;
};

void main() {
    outCol = vec4((texture(img, inUV).rgb / float(times)).rgb, 1.0);
}
