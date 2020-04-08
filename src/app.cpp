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

    // initialize device resources
    device_.initialize(window_);
    swap_chain_.initialize(&device_);
    render_.initialize(&device_, &swap_chain_, nullptr, {});
}

void App::cleanup() {
    render_.finalize();
    swap_chain_.finalize();
    device_.finalize();

    glfwTerminate();
}

void App::run() {
    while (!glfwWindowShouldClose(window_)) {
        glfwPollEvents();

        swap_chain_.acquire();
        render_.drawFrame(swap_chain_.currentImageIndex);
        swap_chain_.present();
    }
}

void App::onWindowSize(int w, int h) {
    render_.finalize();
    swap_chain_.resize();
    render_.initialize(&device_, &swap_chain_, nullptr, {});
}

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
