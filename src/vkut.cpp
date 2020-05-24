//
// Created by murmur.wheel@gmail.com on 2020/5/23.
//

#include "vkut.h"
#include <cassert>

VKUT *g_device_ptr = nullptr;

void VKUT::startup(GLFWwindow *window) {
    g_device_ptr = new VKUT();

    std::vector<const char *> instance_layers;
    std::vector<const char *> instance_extensions;
    uint32_t extension_count = 0;
    const char **extensions =
        glfwGetRequiredInstanceExtensions(&extension_count);
    for (uint32_t i = 0; i < extension_count; ++i) {
        instance_extensions.push_back(extensions[i]);
    }

    g_device_ptr->create_instance(instance_layers, instance_extensions);

    // create surface
    VK_THROW_IF_FAILED(glfwCreateWindowSurface(g_device_ptr->vk_instance_,
        window,
        nullptr,
        &g_device_ptr->vk_surface_));

    g_device_ptr->select_physical_device();
    g_device_ptr->create_logic_device();
    g_device_ptr->create_swap_chain();
}

void VKUT::shutdown() {
    if (g_device_ptr->vk_device_) {
        vkDestroyDevice(g_device_ptr->vk_device_, nullptr);
    }
    if (g_device_ptr->vk_surface_) {
        vkDestroySurfaceKHR(
            g_device_ptr->vk_instance_, g_device_ptr->vk_surface_, nullptr);
    }
    if (g_device_ptr->vk_instance_) {
        vkDestroyInstance(g_device_ptr->vk_instance_, nullptr);
    }
    delete g_device_ptr;
    g_device_ptr = nullptr;
}

VKUT *VKUT::get() {
    assert(g_device_ptr != nullptr);
    return g_device_ptr;
}

uint32_t VKUT::get_memory_type(uint32_t type_bits,
    VkMemoryPropertyFlags memory_flags) const {
    VkPhysicalDeviceMemoryProperties properties;
    vkGetPhysicalDeviceMemoryProperties(vk_physical_device_, &properties);

    for (uint32_t i = 0; i < properties.memoryTypeCount; ++i) {
        if (type_bits & (0x1U << i) &&
            ((memory_flags & properties.memoryTypes[i].propertyFlags) ==
                memory_flags)) {
            return i;
        }
    }
    return UINT32_MAX;
}

void VKUT::create_instance(const std::vector<const char *> &enabled_layers,
    const std::vector<const char *> &enabled_extensions) {
    VkApplicationInfo application_info = {};
    application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    application_info.pApplicationName = "glsl raytracing";
    application_info.pEngineName = "glsl raytracing";
    application_info.apiVersion = VK_API_VERSION_1_2;

    VkInstanceCreateInfo instance_create_info = {};
    instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_create_info.pApplicationInfo = &application_info;
    instance_create_info.enabledLayerCount = uint32_t(enabled_layers.size());
    instance_create_info.ppEnabledLayerNames = enabled_layers.data();
    instance_create_info.enabledExtensionCount =
        uint32_t(enabled_extensions.size());
    instance_create_info.ppEnabledExtensionNames = enabled_extensions.data();

    VK_THROW_IF_FAILED(
        vkCreateInstance(&instance_create_info, nullptr, &vk_instance_));
}

void VKUT::select_physical_device() {
    assert(vk_surface_ != VK_NULL_HANDLE);
    assert(vk_instance_ != VK_NULL_HANDLE);

    uint32_t physical_device_count = 0;
    VK_THROW_IF_FAILED(vkEnumeratePhysicalDevices(
        vk_instance_, &physical_device_count, nullptr));

    assert(physical_device_count > 0);
    physical_device_count = 1;

    VK_THROW_IF_FAILED(vkEnumeratePhysicalDevices(
        vk_instance_, &physical_device_count, &vk_physical_device_));
}

void VKUT::create_logic_device() {
    std::vector<const char *> device_extensions;
    device_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(
        vk_physical_device_, &queue_family_count, nullptr);
    const float queue_priorities[] = { 1 };
    std::vector<VkDeviceQueueCreateInfo> queue_create_infos = {};
    for (uint32_t i = 0; i < queue_family_count; ++i) {
        VkDeviceQueueCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        info.pQueuePriorities = queue_priorities;
        info.queueCount = 1;
        info.queueFamilyIndex = i;

        queue_create_infos.push_back(info);
    }

    VkDeviceCreateInfo device_create_info = {};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.enabledExtensionCount =
        static_cast<uint32_t>(device_extensions.size());
    device_create_info.ppEnabledExtensionNames = device_extensions.data();
    device_create_info.queueCreateInfoCount =
        static_cast<uint32_t>(queue_create_infos.size());
    device_create_info.pQueueCreateInfos = queue_create_infos.data();

    VK_THROW_IF_FAILED(vkCreateDevice(
        vk_physical_device_, &device_create_info, nullptr, &vk_device_));
}

void VKUT::create_swap_chain() {}

void SwapChain::acquire() {}

void SwapChain::present() {}

const char *vk_result_string(VkResult result) {
#define CASE(E) \
    case E: return #E;
    switch (result) {
        CASE(VK_ERROR_DEVICE_LOST)
        CASE(VK_ERROR_FORMAT_NOT_SUPPORTED)
        default: return "undefined";
    }
#undef CASE
}
