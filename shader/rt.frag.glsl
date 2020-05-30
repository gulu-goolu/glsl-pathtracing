#version 450 core

layout(location = 0) out vec4 outColor;

// rsult descriptor set
layout(rgba32f, set = 0, binding = 0) uniform image2D resultImage;
layout(set = 0, binding = 1) uniform ResultUBO {
    uint epoch;// 已经累计的 epoch，至 max_epoch 后折半
    uint max_epoch;// 允许累计的最大 epoch
}result_ubo;

void result_commit(ivec2 grid, vec3 color) {
    // 使用 color 更新 resultImage 中的颜色值
    vec3 current_color = imageLoad(resultImage, grid).rgb;
    if (result_ubo.epoch == result_ubo.max_epoch) {
        current_color = current_color * 0.5f;
    }

    current_color += color;

    imageStore(resultImage, grid, vec4(current_color, 1.0f));
}


// 光追，将颜色写入 result image，并返回最终的颜色值
vec3 trace() {
    // uvec2 grid = gl_GlobalInvocation.xy;
    uvec2 grid = uvec2(0);
    result_commit(ivec2(grid), vec3(0.1f));
    return vec3(0.0f);
}

void main() {
    // outColor = vec4(trace(), 1.0f);
    outColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
}
