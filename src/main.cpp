//
// Created by murmur.wheel@gmail.com on 2020/5/23.
//

#include "vkut.h"

int main() {
    if (!glfwInit()) {
        return 1;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    auto window = glfwCreateWindow(1920, 1080, "window", nullptr, nullptr);
    VKUT::startup(window);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        // VKUT::get()->swap_chain()->acquire();

        // VKUT::get()->swap_chain()->present();
    }

    VKUT::shutdown();
    glfwTerminate();
}