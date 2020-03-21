#version 450 core

layout(location = 0) in vec2 texUV;
layout(location = 0) out vec4 outCol;

layout(set = 0, binding = 0) uniform sampler2D img;

layout(push_constant) uniform ParamsBlock {
    int times;
};


void main() {
    outCol = vec4((texture(img, texUV) / times).rgb, 1.0);
}
