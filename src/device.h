//
// Created by murmur.wheel@gmail.com on 2020/5/23.
//

#ifndef GLSL_RAYTRACING_DEVICE_H
#define GLSL_RAYTRACING_DEVICE_H

#include <unordered_map>
#include <vector>
// clang-format off
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
// clang-format on

class Device {
 public:
};

class SwapChain {
 public:
  void acquire();
  void present();
};

const char *vk_result_string(VkResult result);

#define VKUT_THROW_IF_FAILED(EXPR)               \
  do {                                           \
    VkResult r = EXPR;                           \
    if (r != VK_SUCCESS) {                       \
      throw std::exception(vk_result_string(r)); \
    }                                            \
  } while (false)

#endif  // GLSL_RAYTRACING_DEVICE_H
