#version 450 core

#define SHAPE_BOUNDS 0
#define SHAPE_ENITTY 1

struct Node {
    uint shape_type;
    uint shape;

    uint children;
    uint count;
};

const uint BOUNDS = 0;
const uint SHAPES = 1;

layout(rgba32f, set = 0, binding = 0) uniform image2D resultImage;
/*
layout(set = 1, binding = 0) readonly buffer HierarchyBuffer {
    Node nodes[];
};
layout(set = 1, binding = 1) readonly buffer IntegersBuffer {
    uint integers[];
};
layout(set = 1, binding = 2) readonly buffer FloatsBuffer {
    vec4 floats[];
};
layout(set = 1, binding = 3) readonly buffer LightUniformBuffer {
    uint numLight;
    vec4 lights[];
};

layout(set = 2, binding = 0) uniform CameraUniformBuffer {
    vec3 eye_pos;
    vec3 up_dir;
    vec3 from;

    vec3 top_left_corner;
    vec3 vertical;
    vec3 horizontal;
};

struct Ray {
    vec3 origin;
    vec3 direction;
};

struct Sphere {
    vec3 center;
    float radius;
};

struct Hit {
    uvec2 material;
    vec3 position;
    vec3 normal;
    vec2 pt;
};

bool is_valid(uvec2 p) {
    return p != uvec2(65535);
}

Hit hit_scene(Ray ray)  {
    Hit hit;
    return hit;
}
*/

void main() {
    ivec2 pos = ivec2(gl_GlobalInvocationID.xy);
    vec3 col = imageLoad(resultImage, pos).xyz + vec3(0.01);
    imageStore(resultImage, pos, vec4(col, 1));
}
