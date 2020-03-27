//
// Created by murmur wheel on 2020/3/22.
//

#ifndef GLSL_RAYTRACING_APP_H
#define GLSL_RAYTRACING_APP_H

#include "device.h"
#include "render.h"
#include <GLFW/glfw3.h>
#include <vector>

class App {
public:
    void startup(int width, int height);
    void cleanup();

    void run();

    void onMouseMove(double x, double y);
    void onWindowSize(int w, int h);

private:
    static void windowSizeProxy(GLFWwindow *window, int w, int h);
    static void mouseMoveProxy(GLFWwindow *window, double x, double y);

    GLFWwindow *window_ = nullptr;

    Device device_;
    SwapChain swap_chain_;
    Render render_;
};

#endif // GLSL_RAYTRACING_APP_H
