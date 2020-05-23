//
// Created by murmur.wheel@gmail.com on 2020/5/23.
//

#include "app.h"
#include <GLFW/glfw3.h>

int main() {
    if (!glfwInit()) {
        return 1;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    auto window = glfwCreateWindow(1920, 1080, "window", nullptr, nullptr);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }

    glfwTerminate();
}