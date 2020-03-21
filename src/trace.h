//
// Created by murmur wheel on 2020/3/22.
//

#ifndef GLSL_RAYTRACING_TRACE_H
#define GLSL_RAYTRACING_TRACE_H

#include "device.h"
#include "scene.h"

class Trace {
public:
    Trace(Scene *scene);
    void updateCamera();
    void updateResolution();

    VkDescriptorSetLayout resLayout();
    VkDescriptorSet res();

private:
};

#endif // GLSL_RAYTRACING_TRACE_H
