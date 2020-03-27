#version 450 core

layout(location = 0) out vec4 outCol;
/*
layout(set = 0, binding = 0) uniform ParamsBlock {
    int times;
};
*/
// layout(set = 0, binding = 1) uniform sampler2D img;

void main() {
    // outCol = vec4((texture(img, texUV) / times).rgb, 1.0);
    outCol = vec4(1, 1, 0, 1);
}
