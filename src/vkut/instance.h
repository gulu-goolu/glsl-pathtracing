//
// Created by murmur.wheel@gmail.com on 2020/6/12.
//

#ifndef VKUT_INSTANCE_H
#define VKUT_INSTANCE_H

#include <vulkan/vulkan.h>

#include <vector>

namespace vkut {
class Instance {
 public:
  Instance();
  ~Instance();

  [[nodiscard]] const VkInstance& vk_instance() const { return vk_instance_; }

 private:
  VkInstance vk_instance_{VK_NULL_HANDLE};
};
}  // namespace vkut

#endif  // VKUT_INSTANCE_H
