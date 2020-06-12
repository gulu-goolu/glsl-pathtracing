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

  uint32_t physical_device_count = 0;
  vkEnumeratePhysicalDevices(instance, &physical_device_count, nullptr);
  printf("physical device count: %d\n", physical_device_count);

  vkDestroyInstance(instance, nullptr);
}

int main() {
  App app;
  app.startup(640, 480);

  app.run();

  app.shutdown();
}