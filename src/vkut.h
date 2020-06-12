//
// Created by murmur.wheel@gmail.com on 2020/5/23.
//

#ifndef VKUT_H
#define VKUT_H

#include <unordered_map>
#include <vector>
// clang-format off
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
// clang-format on

#include <memory>
#include <exception>

class Instance;
class Surface;
class PhysicalDevice;
class Device;
class SwapChain;
using InstancePtr = std::shared_ptr<Instance>;
using SurfacePtr = std::shared_ptr<Surface>;
using PhysicalDevicePtr = std::shared_ptr<PhysicalDevice>;
using DevicePtr = std::shared_ptr<Device>;
using SwapChainPtr = std::shared_ptr<SwapChain>;

class Instance : public std::enable_shared_from_this<Instance> {
 public:
  explicit Instance(uint32_t enabled_layer_count,
                    const char *const *enabled_layers,
                    uint32_t enabled_extension_count,
                    const char *const *enabled_extensions);
  ~Instance();

  // retrieve vulkan states
  [[nodiscard]] const VkInstance &vk_instance() const { return vk_instance_; }

  [[nodiscard]] std::vector<PhysicalDevicePtr> enumerate_physical_devices();

  static std::vector<VkLayerProperties> enumerate_instance_layers();

 private:
  VkInstance vk_instance_{VK_NULL_HANDLE};
};

class Surface {
 public:
  explicit Surface(GLFWwindow *window, InstancePtr instance);
  ~Surface();

  [[nodiscard]] const VkSurfaceKHR &vk_surface() const { return vk_surface_; }
  [[nodiscard]] const InstancePtr &instance() const { return instance_; }

 private:
  InstancePtr instance_;
  VkSurfaceKHR vk_surface_{VK_NULL_HANDLE};
};

class PhysicalDevice {
 public:
  explicit PhysicalDevice(InstancePtr instance,
                          VkPhysicalDevice physical_device);

  // retrieve states
  // clang-format off
  [[nodiscard]] const InstancePtr &instance() const { return instance_; }
  [[nodiscard]] const VkPhysicalDevice &vk_physical_device() const { return vk_physical_device_; }
  [[nodiscard]] uint32_t queue_family_count() const { return static_cast<uint32_t>(queue_family_properties_.size()); }
  [[nodiscard]] const std::vector<VkQueueFamilyProperties>& queue_family_properties() const { return queue_family_properties_; }
  // clang-format on

 private:
  InstancePtr instance_;
  VkPhysicalDevice vk_physical_device_{VK_NULL_HANDLE};
  std::vector<VkQueueFamilyProperties> queue_family_properties_;
};

class Device {
 public:
  explicit Device(PhysicalDevicePtr physical_device,
                  uint32_t enabled_extension_count,
                  const char *const *enabled_extensions);
  ~Device();

  // clang-format off
  [[nodiscard]] const PhysicalDevicePtr &physical_device() const { return physical_device_; }
  [[nodiscard]] const VkDevice &vk_device() const { return vk_device_; }
  // clang-format on

 private:
  PhysicalDevicePtr physical_device_;
  VkDevice vk_device_{VK_NULL_HANDLE};
};

class SwapChain {
 public:
  explicit SwapChain(DevicePtr device, SurfacePtr surface);
  ~SwapChain();

  // retrieve state
  // clang-format off
  [[nodiscard]] const SurfacePtr& surface() const { return surface_; }
  [[nodiscard]] const VkFormat& image_format() const { return image_format_; }
  [[nodiscard]] const VkExtent2D& image_extent() const { return image_extent_; }
  [[nodiscard]] const VkImage& image(uint32_t i) const { return images_[i]; }
  [[nodiscard]] const VkImageView& image_view(uint32_t i) const { return image_views_[i]; }
  [[nodiscard]] const uint32_t& back_image_index() const { return back_image_index_;}
  // clang-format on

  void acquire();
  void present();

 private:
  DevicePtr device_;
  SurfacePtr surface_;

  VkFence acquire_fence_{VK_NULL_HANDLE};

  // properties
  uint32_t present_queue_family_index_{UINT32_MAX};
  VkFormat image_format_{VK_FORMAT_UNDEFINED};
  VkExtent2D image_extent_{0, 0};
  VkPresentModeKHR present_mode_{VK_PRESENT_MODE_FIFO_KHR};
  VkSwapchainKHR vk_swapchain_{VK_NULL_HANDLE};
  std::vector<VkImage> images_;
  std::vector<VkImageView> image_views_;

  uint32_t back_image_index_{0};

  void find_present_queue_family();
  void create_swapchain();
  void destroy_swapchain();
};

void vkut_createSurfaceAndDevice(GLFWwindow *window, SurfacePtr *out_surface,
                                 DevicePtr *out_device);
const char *vkut_convertVkResultToString(VkResult result);

#define VKUT_THROW_IF_FAILED(EXPR)                           \
  do {                                                       \
    VkResult r = EXPR;                                       \
    if (r != VK_SUCCESS) {                                   \
      const char *err_msg = vkut_convertVkResultToString(r); \
      throw std::runtime_error(err_msg);                     \
    }                                                        \
  } while (false)

#endif  // VKUT_H
