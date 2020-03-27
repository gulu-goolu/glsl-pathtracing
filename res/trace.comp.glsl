#version 450 core

struct Node {
    uint type;
    uint object;

    uint children;
    uint count;
};

const uint BOUNDS = 0;
const uint SHAPES = 1;

layout(set = 0, binding = 0) readonly buffer Hierarchy {
    Node nodes[];
};
layout(set = 0, binding = 1) readonly buffer Integers {
    uint integers[];
};
layout(set = 0, binding = 2) readonly buffer Floats {
    vec4 floats[];
};
layout(set = 0, binding = 3) uniform Light {
    uint numLight;
    uvec2 lights[];
};

layout(set = 1, binding = 0) uniform writeonly image2D color;// read write

layout(set = 2, binding = 0) uniform Camera {
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

void main() { }
