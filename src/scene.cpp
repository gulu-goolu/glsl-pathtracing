//
// Created by murmur wheel on 2020/3/22.
//

#include "scene.h"

bool Bounds::contains(const glm::vec3 &pt) const {
    return (pt.x >= min_pt.x && pt.x <= max_pt.x) &&
           (pt.y >= min_pt.y && pt.y <= max_pt.y) &&
           (pt.z >= min_pt.z && pt.z <= max_pt.z);
}

bool Bounds::intersects(const Ray &r, float t_min, float t_max) const {
    for (int axis = 0; axis < 3; ++axis) {
        if (std::abs(r.direction[axis]) < 0.0001f) {
            continue;
        }

        const int other1 = (axis + 1) % 3;
        const int other2 = (axis + 2) % 3;
        float t;
        if (r.direction[axis] > 0) {
            t = (min_pt[axis] - r.origin[axis]) / r.direction[axis];
        } else {
            t = (max_pt[axis] - r.origin[axis]) / r.direction[axis];
        }

        if (t > t_min && t < t_max) {
            auto pt = r.origin + r.direction * t;
            if (min_pt[other1] < pt[other1] && min_pt[other2] < pt[other2] &&
                max_pt[other1] > pt[other1] && max_pt[other2] > pt[other2]) {
                return true;
            }
        }
    }
    return false;
}

Bounds Bounds::merged(const Bounds &b1, const Bounds &b2) {
    auto min_pt = glm::min(b1.min_pt, b2.min_pt);
    auto max_pt = glm::max(b1.min_pt, b2.max_pt);
    return Bounds(min_pt, max_pt);
}

Scene *cornellBox() {
    return nullptr;
}
