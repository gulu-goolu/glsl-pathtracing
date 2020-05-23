//
// Created by murmur.wheel@gmail.com on 2020/5/23.
//

#include "device.h"
#include <cassert>

Device *g_device_ptr = nullptr;

#ifdef VK_USE_PLATFORM_WIN32_KHR
void Device::startup_win32(HINSTANCE hinstance, HWND hwnd) {
    g_device_ptr = new Device();

    std::vector<const char *> instance_layers;
    std::vector<const char *> instance_extensions;
    instance_extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
    instance_extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
    g_device_ptr->create_instance(instance_layers, instance_extensions);

    // create surface
    VkWin32SurfaceCreateInfoKHR win32_surface_info = {};
    win32_surface_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    win32_surface_info.hinstance = hinstance;
    win32_surface_info.hwnd = hwnd;
    VK_THROW_IF_FAILED(vkCreateWin32SurfaceKHR(g_device_ptr->vk_instance_,
        &win32_surface_info,
        nullptr,
        &g_device_ptr->vk_surface_));

    g_device_ptr->select_physical_device();
    g_device_ptr->create_logic_device();
    g_device_ptr->create_swap_chain();
}
#endif // !VK_USE_PLATFORM_WIN32_KHR

void Device::shutdown() {
    if (g_device_ptr->vk_device_) {
        vkDestroyDevice(g_device_ptr->vk_device_, nullptr);
    }
    if (g_device_ptr->vk_instance_) {
        vkDestroyInstance(g_device_ptr->vk_instance_, nullptr);
    }
    delete g_device_ptr;
    g_device_ptr = nullptr;
}

Device *Device::get() {
    assert(g_device_ptr != nullptr);
    return g_device_ptr;
}

uint32_t Device::get_memory_type(uint32_t type_bits,
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

void Device::create_instance(const std::vector<const char *> &enabled_layers,
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

void Device::select_physical_device() {
    assert(vk_surface_ != VK_NULL_HANDLE);
    assert(vk_instance_ != VK_NULL_HANDLE);
}

void Device::create_logic_device() {
    std::vector<const char *> device_extensions;
}

void Device::create_swap_chain() {}

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
