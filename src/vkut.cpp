//
// Created by murmur.wheel@gmail.com on 2020/5/23.
//

#include "vkut.h"
#include <cassert>

VKUT *g_device_ptr = nullptr;

void VKUT::startup(GLFWwindow *window) {
    g_device_ptr = new VKUT();

    std::vector<const char *> instance_layers;
    instance_layers.push_back("VK_LAYER_LUNARG_standard_validation");

    std::vector<const char *> instance_extensions;
    uint32_t extension_count = 0;
    const char **extensions =
        glfwGetRequiredInstanceExtensions(&extension_count);
    for (uint32_t i = 0; i < extension_count; ++i) {
        instance_extensions.push_back(extensions[i]);
    }

    g_device_ptr->create_instance(instance_layers, instance_extensions);

    // create surface
    VKUT_THROW_IF_FAILED(glfwCreateWindowSurface(g_device_ptr->vk_instance_,
        window,
        nullptr,
        &g_device_ptr->vk_surface_));

    g_device_ptr->select_physical_device();
    g_device_ptr->create_logic_device();
    g_device_ptr->create_swap_chain_frame_independent_resources();
    g_device_ptr->create_swap_chain_frame_dependent_resources();
}

void VKUT::shutdown() {
    g_device_ptr->destroy_swap_chain_frame_dependent_resources();
    g_device_ptr->destroy_swap_chain_frame_independent_resources();
    for (auto p : g_device_ptr->transient_command_pools_) {
        vkDestroyCommandPool(g_device_ptr->vk_device_, p.second, nullptr);
    }
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

void VKUT::acquire() {
    VKUT_THROW_IF_FAILED(vkAcquireNextImageKHR(vk_device_,
        vk_swap_chain_,
        UINT64_MAX,
        VK_NULL_HANDLE,
        swap_chain_acquire_fence_,
        &back_image_index_));

    VKUT_THROW_IF_FAILED(vkWaitForFences(
        vk_device_, 1, &swap_chain_acquire_fence_, VK_FALSE, UINT64_MAX));
    VKUT_THROW_IF_FAILED(
        vkResetFences(vk_device_, 1, &swap_chain_acquire_fence_));
}

void VKUT::present() {
    VkQueue present_queue{ VK_NULL_HANDLE };
    vkGetDeviceQueue(
        vk_device_, present_queue_family_index_, 0, &present_queue);

    VkPresentInfoKHR present_info = {};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &vk_swap_chain_;
    present_info.pImageIndices = &back_image_index_;
    VKUT_THROW_IF_FAILED(vkQueuePresentKHR(present_queue, &present_info));
}

void VKUT::resize() {
    destroy_swap_chain_frame_dependent_resources();
    create_swap_chain_frame_dependent_resources();
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

void VKUT::create_buffer(VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags memory_flags,
    Buffer *buffer) {
    VkBufferCreateInfo buffer_create_info = {};
    buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_create_info.usage = usage;
    buffer_create_info.size = size;
    buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    buffer_create_info.queueFamilyIndexCount = 0;
    buffer_create_info.pQueueFamilyIndices = nullptr;
    VKUT_THROW_IF_FAILED(vkCreateBuffer(
        vk_device_, &buffer_create_info, nullptr, &buffer->vk_buffer));

    VkMemoryRequirements mem_req;
    vkGetBufferMemoryRequirements(vk_device_, buffer->vk_buffer, &mem_req);

    VkMemoryAllocateInfo memory_allocate_info = {};
    memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memory_allocate_info.memoryTypeIndex =
        get_memory_type(mem_req.memoryTypeBits, memory_flags);
    memory_allocate_info.allocationSize = mem_req.size;
    VKUT_THROW_IF_FAILED(vkAllocateMemory(
        vk_device_, &memory_allocate_info, nullptr, &buffer->vk_device_memory));

    VKUT_THROW_IF_FAILED(vkBindBufferMemory(
        vk_device_, buffer->vk_buffer, buffer->vk_device_memory, 0));
}

void VKUT::destroy_buffer(Buffer *buffer) {
    if (buffer->vk_buffer) {
        vkDestroyBuffer(vk_device_, buffer->vk_buffer, nullptr);
    }
    if (buffer->vk_device_memory) {
        vkFreeMemory(vk_device_, buffer->vk_device_memory, nullptr);
    }
}

VkCommandBuffer VKUT::begin_transient(uint32_t queue_family_index) {
    if (transient_command_pools_.find(queue_family_index) ==
        transient_command_pools_.end()) {
        VkCommandPoolCreateInfo command_pool_create_info = {};
        command_pool_create_info.sType =
            VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        command_pool_create_info.queueFamilyIndex = queue_family_index;
        VKUT_THROW_IF_FAILED(vkCreateCommandPool(vk_device_,
            &command_pool_create_info,
            nullptr,
            &transient_command_pools_[queue_family_index]));
    }

    VkCommandBuffer command_buffer = VK_NULL_HANDLE;
    VkCommandBufferAllocateInfo command_buffer_allocate_info = {};
    command_buffer_allocate_info.sType =
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocate_info.commandPool =
        transient_command_pools_[queue_family_index];
    command_buffer_allocate_info.commandBufferCount = 1;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    VKUT_THROW_IF_FAILED(vkAllocateCommandBuffers(
        vk_device_, &command_buffer_allocate_info, &command_buffer));

    // record command buffers
    transient_command_buffers_[command_buffer] = queue_family_index;

    VkCommandBufferBeginInfo command_buffer_begin_info = {};
    command_buffer_begin_info.sType =
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    VKUT_THROW_IF_FAILED(
        vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info));

    return command_buffer;
}

void VKUT::flush_transient(VkCommandBuffer command_buffer) {
    VKUT_THROW_IF_FAILED(vkEndCommandBuffer(command_buffer));

    VkQueue submit_queue{ VK_NULL_HANDLE };
    vkGetDeviceQueue(vk_device_, 0, 0, &submit_queue);

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer;
    VKUT_THROW_IF_FAILED(
        vkQueueSubmit(submit_queue, 1, &submit_info, VK_NULL_HANDLE));

    VKUT_THROW_IF_FAILED(vkQueueWaitIdle(submit_queue));

    uint32_t queue_family_index = transient_command_buffers_[command_buffer];
    vkFreeCommandBuffers(vk_device_,
        transient_command_pools_[queue_family_index],
        1,
        &command_buffer);

    transient_command_buffers_.erase(command_buffer);
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

    VKUT_THROW_IF_FAILED(
        vkCreateInstance(&instance_create_info, nullptr, &vk_instance_));
}

void VKUT::select_physical_device() {
    assert(vk_surface_ != VK_NULL_HANDLE);
    assert(vk_instance_ != VK_NULL_HANDLE);

    uint32_t physical_device_count = 0;
    VKUT_THROW_IF_FAILED(vkEnumeratePhysicalDevices(
        vk_instance_, &physical_device_count, nullptr));

    assert(physical_device_count > 0);
    physical_device_count = 1;

    VKUT_THROW_IF_FAILED(vkEnumeratePhysicalDevices(
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

    VKUT_THROW_IF_FAILED(vkCreateDevice(
        vk_physical_device_, &device_create_info, nullptr, &vk_device_));
}

void VKUT::create_swap_chain_frame_independent_resources() {
    VkFenceCreateInfo acquire_fence_create_info = {};
    acquire_fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    VKUT_THROW_IF_FAILED(vkCreateFence(vk_device_,
        &acquire_fence_create_info,
        nullptr,
        &swap_chain_acquire_fence_));

    // get surface present queue family
    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(
        vk_physical_device_, &queue_family_count, nullptr);

    present_queue_family_index_ = UINT32_MAX;
    for (uint32_t i = 0; i < queue_family_count; ++i) {
        VkBool32 supported = VK_FALSE;
        VKUT_THROW_IF_FAILED(vkGetPhysicalDeviceSurfaceSupportKHR(
            vk_physical_device_, i, vk_surface_, &supported));

        if (supported == VK_TRUE) {
            present_queue_family_index_ = i;
            break;
        }
    }

    assert(present_queue_family_index_ != UINT32_MAX);
}

void VKUT::destroy_swap_chain_frame_independent_resources() {
    if (swap_chain_acquire_fence_) {
        vkDestroyFence(vk_device_, swap_chain_acquire_fence_, nullptr);
    }
}

void VKUT::create_swap_chain_frame_dependent_resources() {
    compute_swap_chain_extent();
    select_image_format();
    select_present_mode();

    VkSwapchainCreateInfoKHR swap_chain_create_info = {};
    swap_chain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swap_chain_create_info.surface = vk_surface_;

    // image properties
    swap_chain_create_info.imageExtent = swap_chain_extent_;
    swap_chain_create_info.imageFormat = swap_chain_format_;
    swap_chain_create_info.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    swap_chain_create_info.minImageCount = 2;
    swap_chain_create_info.imageArrayLayers = 1;
    swap_chain_create_info.imageUsage =
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    // sharing mode
    swap_chain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swap_chain_create_info.queueFamilyIndexCount = 0;
    swap_chain_create_info.pQueueFamilyIndices = nullptr;

    // other properties
    swap_chain_create_info.presentMode = swap_chain_present_mode_;
    swap_chain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swap_chain_create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;

    VKUT_THROW_IF_FAILED(vkCreateSwapchainKHR(
        vk_device_, &swap_chain_create_info, nullptr, &vk_swap_chain_));

    // acquire swap chain images
    uint32_t swap_chain_image_count = 0;
    VKUT_THROW_IF_FAILED(vkGetSwapchainImagesKHR(
        vk_device_, vk_swap_chain_, &swap_chain_image_count, nullptr));

    swap_chain_images_.resize(swap_chain_image_count);
    VKUT_THROW_IF_FAILED(vkGetSwapchainImagesKHR(vk_device_,
        vk_swap_chain_,
        &swap_chain_image_count,
        swap_chain_images_.data()));
}

void VKUT::destroy_swap_chain_frame_dependent_resources() {
    if (vk_swap_chain_) {
        vkDestroySwapchainKHR(vk_device_, vk_swap_chain_, nullptr);
    }
}

void VKUT::compute_swap_chain_extent() {
    VkSurfaceCapabilitiesKHR caps = {};
    VKUT_THROW_IF_FAILED(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        vk_physical_device_, vk_surface_, &caps));

    assert(caps.currentExtent.width > 0 && caps.currentExtent.height > 0);

    swap_chain_extent_ = caps.currentExtent;
}

void VKUT::select_image_format() {
    std::vector<VkFormat> desired_formats_ = {
        VK_FORMAT_R8G8B8A8_UNORM,
        VK_FORMAT_B8G8R8A8_UNORM,
    };

    uint32_t surface_format_count = 0;
    VKUT_THROW_IF_FAILED(vkGetPhysicalDeviceSurfaceFormatsKHR(
        vk_physical_device_, vk_surface_, &surface_format_count, nullptr));

    std::vector<VkSurfaceFormatKHR> surface_formats(surface_format_count);
    VKUT_THROW_IF_FAILED(
        vkGetPhysicalDeviceSurfaceFormatsKHR(vk_physical_device_,
            vk_surface_,
            &surface_format_count,
            surface_formats.data()));

    bool found = false;
    for (auto &format : desired_formats_) {
        for (auto &surface_format : surface_formats) {
            if (surface_format.format == format) {
                found = true;
                swap_chain_format_ = format;
                break;
            }
        }

        if (found) {
            break;
        }
    }

    assert(found);
}

void VKUT::select_present_mode() {
    swap_chain_present_mode_ = VK_PRESENT_MODE_FIFO_KHR;
}

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
