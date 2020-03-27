//
// Created by murmur wheel on 2020/3/22.
//

#ifndef GLSL_RAYTRACING_SCENE_H
#define GLSL_RAYTRACING_SCENE_H

#include "util.h"
#include <glm/glm.hpp>
#include <vector>

const int BOUNDS = 0;
const int SPHERE = 1;
const int TRIANGLE = 2;
const int MESH = 3;
const int METALLIC_ROUGHNESS = 4;

struct Ray {
    glm::vec3 origin;
    glm::vec3 direction;
};

struct Bounds {
    glm::vec3 min_pt;
    glm::vec3 max_pt;

    // initial as a empty bounds
    Bounds() : min_pt(glm::vec3(FLT_MAX)), max_pt(glm::vec3(FLT_MIN)) {}

    explicit Bounds(const glm::vec3 &min_pt, const glm::vec3 &max_pt) :
      min_pt(min_pt), max_pt(max_pt) {}
    [[nodiscard]] bool contains(const glm::vec3 &pt) const;
    [[nodiscard]] bool intersects(const Ray &r, float t_min, float t_max) const;
    static Bounds merged(const Bounds &b1, const Bounds &b2);
};

class Material {
public:
    virtual void decodeTo(std::vector<uint32_t> &integers,
        std::vector<glm::vec4> &floats) = 0;
};

class Shape {
public:
    [[nodiscard]] virtual Bounds bounds() const = 0;
    virtual bool hit(const Ray &ray) const = 0;

    [[nodiscard]] virtual Material *material() const = 0;

    virtual void decodeTo(uint32_t material_index,
        std::vector<uint32_t> &integers,
        std::vector<glm::vec4> &floats) const = 0;
};

class Sphere : public Shape {
public:
    explicit Sphere(const glm::vec3 &center, float radius) :
      center(center), radius(radius) {}

    bool hit(const Ray &ray) const override;

    glm::vec3 center;
    float radius;
};

class Triangle : public Shape {};

struct Node {
    uint32_t shape_type = 0;
    Shape *shape = nullptr;

    Node *left = nullptr;
    Node *right = nullptr;
};

struct Scene {
    Node *root = nullptr;
};

Scene *cornellBox();

#endif // GLSL_RAYTRACING_SCENE_H
