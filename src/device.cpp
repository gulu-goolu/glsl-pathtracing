//
// Created by murmur wheel on 2020/3/22.
//

#include "device.h"
#include "util.h"
#include <unordered_set>

static VkComponentMapping identityComponentMapping() {
    return VkComponentMapping{
        VK_COMPONENT_SWIZZLE_IDENTITY,
        VK_COMPONENT_SWIZZLE_IDENTITY,
        VK_COMPONENT_SWIZZLE_IDENTITY,
        VK_COMPONENT_SWIZZLE_IDENTITY,
    };
}

static VkImageSubresourceRange imageSubresourceRange(uint32_t aspectMask,
    uint32_t baseMiplevel,
    uint32_t levelCount,
    uint32_t baseArrayLayer,
    uint32_t layerCount) {
    return VkImageSubresourceRange{
        aspectMask, baseMiplevel, levelCount, baseArrayLayer, layerCount
    };
}

void Device::initialize(GLFWwindow *window) {
    createInstance();
    MUST_SUCCESS(
        glfwCreateWindowSurface(vk_instance, window, nullptr, &vk_surface));

    selectPhysicalDevice();
    createLogicDevice();

    VkFenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    MUST_SUCCESS(
        vkCreateFence(vkDevice, &fenceCreateInfo, nullptr, &transient_fence));
}

void Device::finalize() {
    for (auto &p : transient_command_pools) {
        vkDestroyCommandPool(vkDevice, p.second, nullptr);
    }

    if (transient_fence) {
        vkDestroyFence(vkDevice, transient_fence, nullptr);
    }

    if (vkDevice) {
        vkDestroyDevice(vkDevice, nullptr);
    }
    if (vk_surface) {
        vkDestroySurfaceKHR(vk_instance, vk_surface, nullptr);
    }
    if (vk_instance) {
        vkDestroyInstance(vk_instance, nullptr);
    }
}

uint32_t Device::getMemoryType(uint32_t type_bits,
    uint32_t property_flags) const {
    VkPhysicalDeviceMemoryProperties memoryProperties = {};
    vkGetPhysicalDeviceMemoryProperties(vk_physical_device, &memoryProperties);

    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
        if ((type_bits & 1U) == 1) {
            // Type is available, does it match user properties?
            if ((memoryProperties.memoryTypes[i].propertyFlags &
                    property_flags) == property_flags) {
                return i;
            }
        }
        type_bits >>= 1U;
    }
    return UINT32_MAX;
}

void Device::createBuffer(VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags memoryFlags,
    Buffer *buffer) {
    buffer->memoryFlags = memoryFlags;
    if (buffer->memoryFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
        buffer->memoryFlags |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    }

    VkBufferCreateInfo bufferCreateInfo = {};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferCreateInfo.usage = usage;
    bufferCreateInfo.size = size;
    vkCreateBuffer(vkDevice, &bufferCreateInfo, nullptr, &buffer->vk_buffer);

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(
        vkDevice, buffer->vk_buffer, &memoryRequirements);

    VkMemoryAllocateInfo memoryAllocateInfo = {};
    memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.allocationSize = memoryRequirements.size;
    memoryAllocateInfo.memoryTypeIndex =
        getMemoryType(memoryRequirements.memoryTypeBits, memoryFlags);
    MUST_SUCCESS(vkAllocateMemory(
        vkDevice, &memoryAllocateInfo, nullptr, &buffer->vk_deviceMemory));

    MUST_SUCCESS(vkBindBufferMemory(
        vkDevice, buffer->vk_buffer, buffer->vk_deviceMemory, 0));
}

void Device::destroyBuffer(Buffer *buffer) {
    if (!buffer) {
        return;
    }

    if (buffer->vk_buffer) {
        vkDestroyBuffer(vkDevice, buffer->vk_buffer, nullptr);
    }
    if (buffer->vk_deviceMemory) {
        vkFreeMemory(vkDevice, buffer->vk_deviceMemory, nullptr);
    }
}

void Device::updateBuffer(Buffer *buffer,
    size_t offset,
    size_t size,
    VkPipelineStageFlags srcPipelineStage,
    VkPipelineStageFlags dstPipelineStage,
    VkAccessFlags srcAccessMask,
    VkAccessFlags dstAccessMask,
    const void *data) {
    // create staging resources
    Buffer staging{};
    if (buffer->memoryFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
        createBuffer(size,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
            &staging);
        updateBuffer(&staging,
            0,
            size,
            VK_PIPELINE_STAGE_HOST_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_ACCESS_HOST_WRITE_BIT,
            VK_ACCESS_TRANSFER_READ_BIT,
            data);
    }

    // copy buffer
    VkCommandBuffer cmd = beginTransientCommandBuffer(graphicsQueueIndex);

    const auto set_barrier = [&cmd, &buffer, &offset, &size](
                                 VkPipelineStageFlags srcStage,
                                 VkPipelineStageFlags dstStage,
                                 VkAccessFlags srcMask,
                                 VkAccessFlags dstMask) {
        VkBufferMemoryBarrier barrier = {};
        barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.srcAccessMask = srcMask;
        barrier.dstAccessMask = dstMask;
        barrier.buffer = buffer->vk_buffer;
        barrier.offset = offset;
        barrier.size = size;
        vkCmdPipelineBarrier(
            cmd, srcStage, dstStage, 0, 0, nullptr, 1, &barrier, 0, nullptr);
    };

    VkPipelineStageFlags currentStage = 0;
    VkAccessFlags currentAccessMask = 0;

    if (buffer->memoryFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
        // set transfer barrier
        currentAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        currentStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        set_barrier(
            srcPipelineStage, currentStage, srcAccessMask, currentAccessMask);

        VkBufferCopy copy = {};
        copy.srcOffset = 0;
        copy.dstOffset = offset;
        copy.size = size;
        vkCmdCopyBuffer(cmd, staging.vk_buffer, buffer->vk_buffer, 1, &copy);
    } else {
        // update barrier
        currentStage = VK_PIPELINE_STAGE_HOST_BIT;
        currentAccessMask = VK_ACCESS_HOST_WRITE_BIT;
        set_barrier(
            srcPipelineStage, currentStage, srcAccessMask, currentAccessMask);

        // copy data
        void *mapped = nullptr;
        MUST_SUCCESS(vkMapMemory(
            vkDevice, buffer->vk_deviceMemory, offset, size, 0, &mapped));
        memcpy(mapped, data, size);
        vkUnmapMemory(vkDevice, buffer->vk_deviceMemory);
    }

    // update barrier
    set_barrier(
        currentStage, dstPipelineStage, currentAccessMask, dstAccessMask);

    flushTransientCommandBuffer(cmd);

    // destroy staging resources
    if (buffer->memoryFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
        destroyBuffer(&staging);
    }
}

void Device::createImage2D(VkFormat format,
    VkExtent2D extent,
    VkImageUsageFlags usageFlags,
    VkMemoryPropertyFlags memoryFlags,
    VkImageAspectFlags aspectFlags,
    Image2D *texture) const {
    VkImageCreateInfo image_create_info = {};
    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.extent = { extent.width, extent.height, 1 };
    image_create_info.format = format;
    image_create_info.usage = usageFlags;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    image_create_info.arrayLayers = 1;
    image_create_info.mipLevels = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    MUST_SUCCESS(vkCreateImage(
        vkDevice, &image_create_info, nullptr, &texture->vk_image));

    VkMemoryRequirements image_memory_requirements = {};
    vkGetImageMemoryRequirements(
        vkDevice, texture->vk_image, &image_memory_requirements);

    VkMemoryAllocateInfo memory_allocate_info = {};
    memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memory_allocate_info.memoryTypeIndex =
        getMemoryType(image_memory_requirements.memoryTypeBits, memoryFlags);
    memory_allocate_info.allocationSize = image_memory_requirements.size;
    MUST_SUCCESS(vkAllocateMemory(
        vkDevice, &memory_allocate_info, nullptr, &texture->vk_device_memory));
    MUST_SUCCESS(vkBindImageMemory(
        vkDevice, texture->vk_image, texture->vk_device_memory, 0));

    VkImageViewCreateInfo image_view_create_info = {};
    image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    image_view_create_info.format = format;
    image_view_create_info.image = texture->vk_image;
    image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_create_info.components = identityComponentMapping();
    image_view_create_info.subresourceRange =
        imageSubresourceRange(aspectFlags, 0, 1, 0, 1);
    MUST_SUCCESS(vkCreateImageView(
        vkDevice, &image_view_create_info, nullptr, &texture->vk_image_view));
}

void Device::destroyImage2D(Image2D *t) const {
    if (!t) {
        return;
    }

    if (t->vk_image) {
        vkDestroyImage(vkDevice, t->vk_image, nullptr);
    }
    if (t->vk_device_memory) {
        vkFreeMemory(vkDevice, t->vk_device_memory, nullptr);
    }
    if (t->vk_image_view) {
        vkDestroyImageView(vkDevice, t->vk_image_view, nullptr);
    }
    *t = {};
}

VkCommandBuffer Device::beginTransientCommandBuffer(uint32_t queueFamilyIndex) {
    if (transient_command_pools[queueFamilyIndex] == VK_NULL_HANDLE) {
        VkCommandPoolCreateInfo commandPoolCreateInfo = {};
        commandPoolCreateInfo.sType =
            VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndex;
        commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
        MUST_SUCCESS(vkCreateCommandPool(vkDevice,
            &commandPoolCreateInfo,
            nullptr,
            &transient_command_pools[queueFamilyIndex]));
    }

    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
    commandBufferAllocateInfo.sType =
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.commandBufferCount = 1;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandPool =
        transient_command_pools[queueFamilyIndex];
    MUST_SUCCESS(vkAllocateCommandBuffers(
        vkDevice, &commandBufferAllocateInfo, &commandBuffer));

    // record command buffer's queue family index
    transient_command_buffers[commandBuffer] = queueFamilyIndex;

    VkCommandBufferBeginInfo commandBufferBeginInfo = {};
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    MUST_SUCCESS(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo));

    return commandBuffer;
}

void Device::flushTransientCommandBuffer(VkCommandBuffer commandBuffer) {
    MUST_SUCCESS(vkEndCommandBuffer(commandBuffer));

    auto queueFamilyIndex = transient_command_buffers[commandBuffer];

    VkQueue q;
    vkGetDeviceQueue(vkDevice, queueFamilyIndex, 0, &q);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    MUST_SUCCESS(vkQueueSubmit(q, 1, &submitInfo, transient_fence));
    MUST_SUCCESS(
        vkWaitForFences(vkDevice, 1, &transient_fence, VK_FALSE, UINT64_MAX));
    MUST_SUCCESS(vkResetFences(vkDevice, 1, &transient_fence));

    MUST_SUCCESS(vkQueueWaitIdle(q));
}

VkDescriptorSet Device::allocateSingleDescriptorSet(VkDescriptorPool pool,
    VkDescriptorSetLayout layout) const {
    VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
    descriptorSetAllocateInfo.sType =
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocateInfo.descriptorPool = pool;
    descriptorSetAllocateInfo.descriptorSetCount = 1;
    descriptorSetAllocateInfo.pSetLayouts = &layout;
    MUST_SUCCESS(vkAllocateDescriptorSets(
        vkDevice, &descriptorSetAllocateInfo, &descriptorSet));

    return descriptorSet;
}

VkShaderModule Device::loadShaderModule(const char *filename) const {
    auto blob = readFile(filename);
    if (blob.empty()) {
        return VK_NULL_HANDLE;
    }

    VkShaderModule shaderModule = VK_NULL_HANDLE;
    VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
    shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModuleCreateInfo.codeSize = static_cast<uint32_t>(blob.size());
    shaderModuleCreateInfo.pCode =
        reinterpret_cast<const uint32_t *>(blob.data());
    MUST_SUCCESS(vkCreateShaderModule(
        vkDevice, &shaderModuleCreateInfo, nullptr, &shaderModule));

    return shaderModule;
}

void Device::createInstance() {
    std::vector<const char *> extensions = {
        VK_KHR_SURFACE_EXTENSION_NAME,
#if defined(VK_USE_PLATFORM_WIN32_KHR)
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#endif
    };

    std::vector<const char *> layers = {
#if defined(DEBUG)
        "VK_LAYER_LUNARG_standard_validation"
#endif
    };

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
    createInfo.ppEnabledLayerNames = layers.data();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();
    MUST_SUCCESS(vkCreateInstance(&createInfo, nullptr, &vk_instance));
}

void Device::selectPhysicalDevice() {
    uint32_t cnt = 0;
    MUST_SUCCESS(vkEnumeratePhysicalDevices(vk_instance, &cnt, nullptr));

    std::vector<VkPhysicalDevice> physical_devices(cnt);
    MUST_SUCCESS(
        vkEnumeratePhysicalDevices(vk_instance, &cnt, physical_devices.data()));

    const auto make_invalid = [=] {
        graphicsQueueIndex = UINT32_MAX;
        compute_queue_index = UINT32_MAX;
        present_queue_index = UINT32_MAX;
    };

    const auto is_valid = [=] {
        return graphicsQueueIndex != UINT32_MAX &&
               compute_queue_index != UINT32_MAX &&
               present_queue_index != UINT32_MAX;
    };

    for (auto &dev : physical_devices) {
        make_invalid();

        uint32_t q_cnt = 0; // queue family count
        vkGetPhysicalDeviceQueueFamilyProperties(dev, &q_cnt, nullptr);

        std::vector<VkQueueFamilyProperties> q_props(q_cnt);
        vkGetPhysicalDeviceQueueFamilyProperties(dev, &q_cnt, q_props.data());

        const auto find_queue_index = [&q_props, &q_cnt](uint32_t bits) {
            for (uint32_t i = 0; i < q_cnt; ++i) {
                if ((q_props[i].queueFlags & bits) == bits) {
                    return i;
                }
            }
            return UINT32_MAX; // invalid
        };
        graphicsQueueIndex = find_queue_index(VK_QUEUE_GRAPHICS_BIT);
        compute_queue_index = find_queue_index(VK_QUEUE_COMPUTE_BIT);

        for (uint32_t i = 0; i < q_cnt; ++i) {
            VkBool32 supported = VK_FALSE;
            MUST_SUCCESS(vkGetPhysicalDeviceSurfaceSupportKHR(
                dev, i, vk_surface, &supported));

            if (supported) {
                present_queue_index = i;
                break;
            }
        }

        if (is_valid()) {
            vk_physical_device = dev;
            break;
        }
    }
}

void Device::createLogicDevice() {
    std::unordered_set<uint32_t> queue_indices;
    queue_indices.insert(graphicsQueueIndex);
    queue_indices.insert(compute_queue_index);
    queue_indices.insert(present_queue_index);

    std::vector<VkDeviceQueueCreateInfo> queueInfos = {};
    const float queue_priorities[] = { 1 };
    for (auto &i : queue_indices) {
        VkDeviceQueueCreateInfo queueInfo = {};
        queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueInfo.pQueuePriorities = queue_priorities;
        queueInfo.queueCount = 1;
        queueInfo.queueFamilyIndex = i;

        queueInfos.push_back(queueInfo);
    }

    std::vector<const char *> extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueInfos.size());
    createInfo.pQueueCreateInfos = queueInfos.data();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();
    MUST_SUCCESS(
        vkCreateDevice(vk_physical_device, &createInfo, nullptr, &vkDevice));
}

void SwapChain::initialize(Device *device_) {
    this->device = device_;

    VkFenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = 0;
    MUST_SUCCESS(vkCreateFence(
        device->vkDevice, &fenceCreateInfo, nullptr, &acquire_fence));

    createSwapChain();
}

void SwapChain::finalize() {
    destroySwapChain();

    if (acquire_fence) {
        vkDestroyFence(device->vkDevice, acquire_fence, nullptr);
    }

    if (device && vk_swapchain) {
        vkDestroySwapchainKHR(device->vkDevice, vk_swapchain, nullptr);
    }
}

void SwapChain::resize() {
    destroySwapChain();
    createSwapChain();
}

void SwapChain::acquire() {
    MUST_SUCCESS(vkAcquireNextImageKHR(device->vkDevice,
        vk_swapchain,
        UINT64_MAX,
        VK_NULL_HANDLE,
        acquire_fence,
        &current_image_index));

    MUST_SUCCESS(vkWaitForFences(
        device->vkDevice, 1, &acquire_fence, VK_FALSE, UINT64_MAX));
    MUST_SUCCESS(vkResetFences(device->vkDevice, 1, &acquire_fence));
}

void SwapChain::present() {
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &vk_swapchain;
    presentInfo.pImageIndices = &current_image_index;

    VkQueue q = VK_NULL_HANDLE;
    vkGetDeviceQueue(device->vkDevice, device->present_queue_index, 0, &q);

    MUST_SUCCESS(vkQueuePresentKHR(q, &presentInfo));
}

void SwapChain::createSwapChain() {
    selectImageFormat();

    VkSurfaceCapabilitiesKHR caps = {};
    MUST_SUCCESS(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        device->vk_physical_device, device->vk_surface, &caps));

    imageExtent = caps.currentExtent;

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;

    createInfo.surface = device->vk_surface; // surface

    // image properties
    createInfo.imageArrayLayers = 1;
    createInfo.imageExtent = caps.currentExtent;
    createInfo.imageFormat = image_format;
    createInfo.imageColorSpace = image_color_space;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                            VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                            VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

    // share modes
    const uint32_t indices[] = {
        device->graphicsQueueIndex,
        device->present_queue_index,
    };
    if (indices[0] == indices[1]) {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = indices;
    }

    // other properties
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    createInfo.minImageCount = 2;
    createInfo.clipped = VK_TRUE;
    createInfo.presentMode = vk_present_mode;

    MUST_SUCCESS(vkCreateSwapchainKHR(
        device->vkDevice, &createInfo, nullptr, &vk_swapchain));

    retrieveImages();
    createImageViews();
}

void SwapChain::destroySwapChain() {
    for (auto &view : vk_image_views) {
        vkDestroyImageView(device->vkDevice, view, nullptr);
    }
    vk_image_views.clear();

    if (vk_swapchain) {
        vkDestroySwapchainKHR(device->vkDevice, vk_swapchain, nullptr);
        vk_swapchain = VK_NULL_HANDLE;
    }
}

void SwapChain::selectImageFormat() {
    uint32_t cnt = 0;
    MUST_SUCCESS(vkGetPhysicalDeviceSurfaceFormatsKHR(
        device->vk_physical_device, device->vk_surface, &cnt, nullptr));

    std::vector<VkSurfaceFormatKHR> fmts(cnt);
    MUST_SUCCESS(vkGetPhysicalDeviceSurfaceFormatsKHR(
        device->vk_physical_device, device->vk_surface, &cnt, fmts.data()));

    image_format = VK_FORMAT_UNDEFINED;
    const std::vector<VkFormat> want_formats = {
        VK_FORMAT_R8G8B8A8_UNORM,
        VK_FORMAT_B8G8R8A8_UNORM,
    };
    for (auto &f : want_formats) {
        for (auto &s : fmts) {
            if (s.format == f) {
                image_format = s.format;
                image_color_space = s.colorSpace;
                break;
            }
        }

        if (image_format != VK_FORMAT_UNDEFINED) {
            break;
        }
    }

    if (image_format == VK_FORMAT_UNDEFINED) {
        perror("no suit format found!");
        exit(1);
    }
}

void SwapChain::retrieveImages() {
    uint32_t cnt = 0;
    MUST_SUCCESS(
        vkGetSwapchainImagesKHR(device->vkDevice, vk_swapchain, &cnt, nullptr));

    vkImages.resize(cnt);
    MUST_SUCCESS(vkGetSwapchainImagesKHR(
        device->vkDevice, vk_swapchain, &cnt, vkImages.data()));
}

void SwapChain::createImageViews() {
    vk_image_views.resize(vkImages.size());
    for (size_t i = 0; i < vkImages.size(); ++i) {
        VkImageViewCreateInfo imageViewCreateInfo = {};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.image = vkImages[i];
        imageViewCreateInfo.format = image_format;
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.components = identityComponentMapping();
        imageViewCreateInfo.subresourceRange =
            imageSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1);

        MUST_SUCCESS(vkCreateImageView(device->vkDevice,
            &imageViewCreateInfo,
            nullptr,
            &vk_image_views[i]));
    }
}
