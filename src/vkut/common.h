//
// Created by murmur.wheel@gmail.com on 2020/6/12.
//

#ifndef VKUT_COMMON_H
#define VKUT_COMMON_H

#include <vulkan/vulkan.h>

#include <exception>

const char* get_result_string(VkResult result);

#define VKUT_CHECK_RESULT(EXPR) \
  do {                          \
    VkResult result = EXPR;     \
    if (result != VK_SUCCESS) { \
      std::abort();             \
    }                           \
  } while (false)

#endif  // VKUT_COMMON_H
