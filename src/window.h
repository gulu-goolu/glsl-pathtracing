//
// Created by murmur wheel on 2020/3/22.
//

#ifndef GLSL_RAYTRACING_WINDOW_H
#define GLSL_RAYTRACING_WINDOW_H

#include "trace.h"

// just used for show result
class Window {
public:
    explicit Window(Trace *trace, int width, int height, const char *title);

    virtual void onWheel(float x, float y);       // set fov angle
    virtual void onMouseMove(float dx, float dy); // drag model
    virtual void onSize(int width, int height);   // window resize

    void run();

private:
    Trace *trace_ = nullptr;
};

#endif // GLSL_RAYTRACING_WINDOW_H
