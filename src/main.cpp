//
// Created by murmur.wheel@gmail.com on 2020/5/23.
//

#include "app.h"

void test_vulkan() {
  VkInstance instance;
  VkInstanceCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  if (vkCreateInstance(&create_info, nullptr, &instance) == VK_SUCCESS) {
    printf("init success!\n");
  } else {
    printf("init failed!\n");
  }

  vkDestroyInstance(instance, nullptr);
}

int main() {
  if (!glfwInit()) {
    return 1;
  }

  test_vulkan();

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  auto window = glfwCreateWindow(640, 480, "window", nullptr, nullptr);
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
  }

  glfwTerminate();
}