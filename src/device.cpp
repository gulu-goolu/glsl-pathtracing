//
// Created by murmur.wheel@gmail.com on 2020/5/23.
//

#include "device.h"

void SwapChain::acquire() {}

void SwapChain::present() {}

const char *vk_result_string(VkResult result) {
#define CASE(E) \
  case E:       \
    return #E;
  switch (result) {
    CASE(VK_ERROR_DEVICE_LOST)
    CASE(VK_ERROR_FORMAT_NOT_SUPPORTED)
    default:
      return "undefined";
  }
#undef CASE
}
