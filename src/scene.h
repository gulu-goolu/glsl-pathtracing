//
// Created by murmur wheel on 2020/3/22.
//

#ifndef GLSL_RAYTRACING_SCENE_H
#define GLSL_RAYTRACING_SCENE_H

#include <vector>

struct Vec4 {
    float x, y, z, w;
};

struct Quad {};
struct Sphere {};
struct Triangle {};
struct Material {};

class Scene {
public:
};

struct SceneData {
    std::vector<uint32_t> node;
    std::vector<int32_t> int_literals;
    std::vector<Vec4> float_literals;
};

#endif // GLSL_RAYTRACING_SCENE_H
