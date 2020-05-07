//
// Created by murmur.wheel@gmail.com on 2020/3/22.
//

#ifndef GLSL_RAYTRACING_RENDER_H
#define GLSL_RAYTRACING_RENDER_H

#include "glm/glm.hpp"
#include "scene.h"

struct CameraData {
    glm::vec4 from;
    glm::vec4 to;
    glm::vec4 up;
    float fov_angle;
    float aspect;

    // for align
    float reserved1;
    float reserved2;

    glm::vec4 top_left_corner;
    glm::vec4 vertical;
    glm::vec4 horizontal;
};

#endif // GLSL_RAYTRACING_RENDER_H
