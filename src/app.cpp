//
// Created by murmur wheel on 2020/3/22.
//

#include "app.h"
#include <cstdlib>

void App::startup(int width, int height) {
    if (!glfwInit()) {
        exit(1);
    }

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window_ =
        glfwCreateWindow(width, height, "glsl raytracing", nullptr, nullptr);

    // set callbacks
    glfwSetWindowUserPointer(window_, this);
    glfwSetWindowSizeCallback(window_, windowSizeProxy);
    glfwSetCursorPosCallback(window_, mouseMoveProxy);
}

void App::cleanup() {
    glfwTerminate();
}

void App::run() {
    while (!glfwWindowShouldClose(window_)) {
        glfwPollEvents();
    }
}

void App::onWindowSize(int w, int h) {}

void App::onMouseMove(double x, double y) {
    printf("x = %f, y = %f\n", x, y);
}

void App::windowSizeProxy(GLFWwindow *window, int w, int h) {
    auto p = glfwGetWindowUserPointer(window);
    auto app = reinterpret_cast<App *>(p);
    app->onWindowSize(w, h);
}

void App::mouseMoveProxy(GLFWwindow *window, double x, double y) {
    auto p = glfwGetWindowUserPointer(window);
    auto app = reinterpret_cast<App *>(p);
    app->onMouseMove(x, y);
}
