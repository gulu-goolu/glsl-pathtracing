//
// Created by murmur.wheel@gmail.com on 2020/5/23.
//

#include "vkut.h"

Instance::Instance(uint32_t enabled_layer_count,
                   const char* const* enabled_layers,
                   uint32_t enabled_extension_count,
                   const char* const* enabled_extensions) {
  VkInstanceCreateInfo instance_create_info = {};
  instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instance_create_info.enabledLayerCount = enabled_layer_count;
  instance_create_info.ppEnabledLayerNames = enabled_layers;
  instance_create_info.enabledExtensionCount = enabled_extension_count;
  instance_create_info.ppEnabledExtensionNames = enabled_extensions;
  VKUT_THROW_IF_FAILED(
      vkCreateInstance(&instance_create_info, nullptr, &vk_instance_));
}

Instance::~Instance() {
  if (vk_instance_) {
    vkDestroyInstance(vk_instance_, nullptr);
  }
}

std::vector<PhysicalDevicePtr> Instance::enumerate_physical_devices() {
  uint32_t physical_device_count = 0;
  VKUT_THROW_IF_FAILED(vkEnumeratePhysicalDevices(
      vk_instance_, &physical_device_count, nullptr));

  std::vector<VkPhysicalDevice> temp(physical_device_count);
  VKUT_THROW_IF_FAILED(vkEnumeratePhysicalDevices(
      vk_instance_, &physical_device_count, temp.data()));

  std::vector<PhysicalDevicePtr> physical_devices;
  for (const auto& t : temp) {
    auto p = std::make_shared<PhysicalDevice>(shared_from_this(), t);
    physical_devices.push_back(p);
  }

  return physical_devices;
}

Surface::Surface(GLFWwindow* window, InstancePtr instance)
    : instance_(std::move(instance)) {
  VKUT_THROW_IF_FAILED(glfwCreateWindowSurface(instance_->vk_instance(), window,
                                               nullptr, &vk_surface_));
}

Surface::~Surface() {
  if (vk_surface_) {
    vkDestroySurfaceKHR(instance_->vk_instance(), vk_surface_, nullptr);
  }
}

PhysicalDevice::PhysicalDevice(InstancePtr instance,
                               VkPhysicalDevice physical_device)
    : instance_(std::move(instance)), vk_physical_device_(physical_device) {
  uint32_t queue_family_count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(vk_physical_device_,
                                           &queue_family_count, nullptr);

  queue_family_properties_.resize(queue_family_count);
  vkGetPhysicalDeviceQueueFamilyProperties(vk_physical_device_,
                                           &queue_family_count,
                                           queue_family_properties_.data());
}

Device::Device(PhysicalDevicePtr physical_device,
               uint32_t enabled_extension_count,
               const char* const* enabled_extensions)
    : physical_device_(std::move(physical_device)) {
  const float queue_priorities[] = {1};
  std::vector<VkDeviceQueueCreateInfo> device_queue_create_infos = {};
  for (uint32_t i = 0; i < physical_device_->queue_family_count(); ++i) {
    VkDeviceQueueCreateInfo queue_create_info = {};
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.queueCount = 1;
    queue_create_info.queueFamilyIndex = i;
    queue_create_info.pQueuePriorities = queue_priorities;
    device_queue_create_infos.push_back(queue_create_info);
  }

  VkDeviceCreateInfo device_create_info = {};
  // clang-format off
  device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  device_create_info.queueCreateInfoCount = static_cast<uint32_t>(device_queue_create_infos.size());
  device_create_info.pQueueCreateInfos = device_queue_create_infos.data();
  device_create_info.enabledExtensionCount = enabled_extension_count;
  device_create_info.ppEnabledExtensionNames = enabled_extensions;
  // clang-format on
  VKUT_THROW_IF_FAILED(vkCreateDevice(physical_device_->vk_physical_device(),
                                      &device_create_info, nullptr,
                                      &vk_device_));
}

Device::~Device() {
  if (vk_device_) {
    vkDestroyDevice(vk_device_, nullptr);
  }
}

SwapChain::SwapChain(DevicePtr device, SurfacePtr surface)
    : device_(std::move(device)), surface_(std::move(surface)) {
  find_present_queue_family();

  // create a fence for acquire back image
  VkFenceCreateInfo acquire_fence_create_info = {};
  acquire_fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  VKUT_THROW_IF_FAILED(vkCreateFence(device_->vk_device(),
                                     &acquire_fence_create_info, nullptr,
                                     &acquire_fence_));

  create_swapchain();
}

SwapChain::~SwapChain() {
  destroy_swapchain();

  if (acquire_fence_) {
    vkDestroyFence(device_->vk_device(), acquire_fence_, nullptr);
  }
}

void SwapChain::acquire() {
  VKUT_THROW_IF_FAILED(vkAcquireNextImageKHR(
      device_->vk_device(), vk_swapchain_, UINT64_MAX, VK_NULL_HANDLE,
      acquire_fence_, &back_image_index_));

  VKUT_THROW_IF_FAILED(vkWaitForFences(device_->vk_device(), 1, &acquire_fence_,
                                       VK_FALSE, UINT64_MAX));
  VKUT_THROW_IF_FAILED(vkResetFences(device_->vk_device(), 1, &acquire_fence_));
}

void SwapChain::present() {
  VkQueue q = VK_NULL_HANDLE;
  vkGetDeviceQueue(device_->vk_device(), present_queue_family_index_, 0, &q);

  VkPresentInfoKHR present_info = {};
  present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  present_info.swapchainCount = 1;
  present_info.pSwapchains = &vk_swapchain_;
  present_info.pImageIndices = &back_image_index_;
  VKUT_THROW_IF_FAILED(vkQueuePresentKHR(q, &present_info));
}

void SwapChain::find_present_queue_family() {
  auto physical_device = device_->physical_device();

  present_queue_family_index_ = UINT32_MAX;
  for (uint32_t i = 0; i < physical_device->queue_family_count(); ++i) {
    VkBool32 supported = VK_FALSE;
    VKUT_THROW_IF_FAILED(vkGetPhysicalDeviceSurfaceSupportKHR(
        physical_device->vk_physical_device(), i, surface_->vk_surface(),
        &supported));

    if (supported == VK_TRUE) {
      present_queue_family_index_ = i;
      break;
    }
  }
}

void SwapChain::create_swapchain() {
  const auto compute_image_extent = [this] {
    VkSurfaceCapabilitiesKHR caps;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        device_->physical_device()->vk_physical_device(), surface_->vk_surface(),
        &caps);

    image_extent_ = caps.currentExtent;
  };
  const auto select_image_format = [this] {
    auto& physical_device = device_->physical_device();

    uint32_t format_count = 0;
    VKUT_THROW_IF_FAILED(vkGetPhysicalDeviceSurfaceFormatsKHR(
        physical_device->vk_physical_device(), surface_->vk_surface(),
        &format_count, nullptr));

    std::vector<VkSurfaceFormatKHR> surface_formats(format_count);
    VKUT_THROW_IF_FAILED(vkGetPhysicalDeviceSurfaceFormatsKHR(
        physical_device->vk_physical_device(), surface_->vk_surface(),
        &format_count, surface_formats.data()));

    for (auto& surface_format : surface_formats) {
      if (surface_format.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR &&
          surface_format.format == VK_FORMAT_R8G8B8A8_UNORM) {
        image_format_ = VK_FORMAT_R8G8B8A8_UNORM;
        break;
      }
    }
  };
  const auto select_present_mode = [this] {
    present_mode_ = VK_PRESENT_MODE_FIFO_KHR;
  };

  compute_image_extent();
  select_image_format();
  select_present_mode();

  VkSwapchainCreateInfoKHR swapchain_create_info = {};
  swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  swapchain_create_info.surface = surface_->vk_surface();

  // image properties
  swapchain_create_info.minImageCount = 2;
  swapchain_create_info.imageUsage =
      VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
  swapchain_create_info.imageFormat = image_format_;
  swapchain_create_info.imageExtent = image_extent_;
  swapchain_create_info.imageArrayLayers = 1;
  swapchain_create_info.imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;

  // sharing mode
  swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  swapchain_create_info.queueFamilyIndexCount = 0;
  swapchain_create_info.pQueueFamilyIndices = nullptr;

  // other properties
  swapchain_create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
  swapchain_create_info.presentMode = present_mode_;
  swapchain_create_info.clipped = VK_TRUE;
  swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  VKUT_THROW_IF_FAILED(vkCreateSwapchainKHR(
      device_->vk_device(), &swapchain_create_info, nullptr, &vk_swapchain_));

  // retrieve swapchain images
  uint32_t image_count = 0;
  VKUT_THROW_IF_FAILED(vkGetSwapchainImagesKHR(
      device_->vk_device(), vk_swapchain_, &image_count, nullptr));

  images_.resize(image_count);
  VKUT_THROW_IF_FAILED(vkGetSwapchainImagesKHR(
      device_->vk_device(), vk_swapchain_, &image_count, images_.data()));

  // create associate image view
  for (auto& image : images_) {
    VkImageViewCreateInfo image_view_create_info = {};
    image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    image_view_create_info.image = image;
    image_view_create_info.format = image_format_;
    image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_create_info.components = {
        VK_COMPONENT_SWIZZLE_IDENTITY,
        VK_COMPONENT_SWIZZLE_IDENTITY,
        VK_COMPONENT_SWIZZLE_IDENTITY,
        VK_COMPONENT_SWIZZLE_IDENTITY,
    };
    image_view_create_info.subresourceRange = {
        VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1,
    };
    VkImageView view = VK_NULL_HANDLE;
    VKUT_THROW_IF_FAILED(vkCreateImageView(
        device_->vk_device(), &image_view_create_info, nullptr, &view));

    image_views_.push_back(view);
  }
}

void SwapChain::destroy_swapchain() {
  if (vk_swapchain_) {
    vkDestroySwapchainKHR(device_->vk_device(), vk_swapchain_, nullptr);
  }
  for (auto& view : image_views_) {
    vkDestroyImageView(device_->vk_device(), view, nullptr);
  }
}

void vkut_createInstanceAndSurface(GLFWwindow* window,
                                   InstancePtr* out_instance,
                                   SurfacePtr* out_surface) {
  uint32_t extension_count = 0;
  const auto extensions = glfwGetRequiredInstanceExtensions(&extension_count);

  *out_instance =
      std::make_shared<Instance>(0, nullptr, extension_count, extensions);
  *out_surface = std::make_shared<Surface>(window, *out_instance);
}

void vkut_createSurfaceAndDevice(GLFWwindow* window, SurfacePtr* out_surface,
                                 DevicePtr* out_device) {
  // create instance
  std::vector<const char*> instance_layers = {
      "VK_LAYER_LUNARG_standard_validation",
  };

  uint32_t extension_count = 0;
  const auto extensions = glfwGetRequiredInstanceExtensions(&extension_count);

  auto instance = std::make_shared<Instance>(uint32_t(instance_layers.size()),
                                             instance_layers.data(),
                                             extension_count, extensions);

  // create surface
  *out_surface = std::make_shared<Surface>(window, instance);

  // create device
  std::vector<const char*> device_extensions = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME,
  };
  auto physical_device = instance->enumerate_physical_devices()[0];
  *out_device = std::make_shared<Device>(physical_device,
                                         uint32_t(device_extensions.size()),
                                         device_extensions.data());
}

const char* vkut_convertVkResultToString(VkResult result) {
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
