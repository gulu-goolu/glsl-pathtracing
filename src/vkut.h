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
#include <stdexcept>

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

class SwapchainNotifier {
 public:
  virtual void on_swapchain_created() = 0;
  virtual void on_swapchain_destroy() = 0;
};

class VKUT {
 public:
  static void startup(GLFWwindow *window, SwapchainNotifier *notifier);
  static void shutdown();
  static VKUT *get();

  VKUT(GLFWwindow *window, SwapchainNotifier *notifier);
  ~VKUT();

  void render();

 private:
  GLFWwindow *window_{nullptr};
  SwapchainNotifier *swapchain_notifier_{nullptr};

  VkInstance vk_instance_{VK_NULL_HANDLE};
  VkPhysicalDevice vk_physical_device_{VK_NULL_HANDLE};
  VkDevice vk_device_{VK_NULL_HANDLE};

  void create_instance();
  void select_physical_device();
  void create_logic_device();
};

#endif  // VKUT_H
