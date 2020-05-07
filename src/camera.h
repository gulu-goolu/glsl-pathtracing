//
// Created by murmur.wheel@gmail.com on 2020/5/7.
//

#ifndef GLSL_RAYTRACING_CAMERA_H
#define GLSL_RAYTRACING_CAMERA_H

#include <glm/glm.hpp>

struct CameraData {};

class CameraBase {
public:
    virtual void get_data(CameraData *data, uint32_t *version) const = 0;
};

class ModelViewCamera : public CameraBase {};

class FirstPersonCamera : public CameraBase {};

#endif // GLSL_RAYTRACING_CAMERA_H
