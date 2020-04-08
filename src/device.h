//
// Created by murmur wheel on 2020/3/22.
//

#ifndef GLSL_RAYTRACING_DEVICE_H
#define GLSL_RAYTRACING_DEVICE_H

#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>
#include <vulkan/vulkan.h>

#include <GLFW/glfw3.h>

#define MUST_SUCCESS(EXPR) \
    do { \
        VkResult res = EXPR; \
        if (res != VK_SUCCESS) { \
            perror(#EXPR); \
            exit(1); \
        } \
    } while (false)

struct Buffer {
    VkDeviceMemory vk_deviceMemory = VK_NULL_HANDLE;
    VkBuffer vk_buffer = VK_NULL_HANDLE;
    VkMemoryPropertyFlags memoryFlags = 0;
};

struct Image2D {
    VkImage vk_image = VK_NULL_HANDLE;
    VkDeviceMemory vk_device_memory = VK_NULL_HANDLE;
    VkImageView vk_image_view = VK_NULL_HANDLE;
};

class Device {
public:
    void initialize(GLFWwindow *window);

    void finalize();

    [[nodiscard]] uint32_t getMemoryType(uint32_t typeBits,
        uint32_t memoryFlags) const;

    void createBuffer(VkDeviceSize size,
        VkBufferUsageFlags usage,
        VkMemoryPropertyFlags memoryFlags,
        Buffer *buffer);

    void destroyBuffer(Buffer *buffer);

    void updateBuffer(Buffer *buffer,
        size_t offset,
        size_t size,
        VkPipelineStageFlags srcPipelineStage,
        VkPipelineStageFlags dstPipelineStage,
        VkAccessFlags srcAccessMask,
        VkAccessFlags dstAccessMask,
        const void *data);

    void createImage2D(VkFormat format,
        VkExtent2D extent,
        VkImageUsageFlags usageFlags,
        VkMemoryPropertyFlags memoryFlags,
        VkImageAspectFlags aspectFlags,
        Image2D *texture) const;

    void destroyImage2D(Image2D *t) const;

    VkCommandBuffer beginTransientCommandBuffer(uint32_t queueFamilyIndex);

    void flushTransientCommandBuffer(VkCommandBuffer commandBuffer);

    template<size_t PoolSizeCount>
    VkDescriptorPool createDescriptorPool(
        const std::array<VkDescriptorPoolSize, PoolSizeCount> &sizes,
        uint32_t maxSets) {
        VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
        VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
        descriptorPoolCreateInfo.sType =
            VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptorPoolCreateInfo.poolSizeCount =
            static_cast<uint32_t>(PoolSizeCount);
        descriptorPoolCreateInfo.pPoolSizes = sizes.data();
        descriptorPoolCreateInfo.maxSets = maxSets;
        MUST_SUCCESS(vkCreateDescriptorPool(
            vkDevice, &descriptorPoolCreateInfo, nullptr, &descriptorPool));

        return descriptorPool;
    }

    template<size_t BindingCount>
    VkDescriptorSetLayout createDescriptorSetLayout(
        const std::array<VkDescriptorSetLayoutBinding, BindingCount> &bindings)
        const {
        VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {};
        descriptorSetLayoutCreateInfo.sType =
            VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorSetLayoutCreateInfo.bindingCount =
            static_cast<uint32_t>(BindingCount);
        descriptorSetLayoutCreateInfo.pBindings = bindings.data();
        MUST_SUCCESS(vkCreateDescriptorSetLayout(vkDevice,
            &descriptorSetLayoutCreateInfo,
            nullptr,
            &descriptorSetLayout));

        return descriptorSetLayout;
    }

    VkDescriptorSet allocateSingleDescriptorSet(VkDescriptorPool pool,
        VkDescriptorSetLayout layout) const;

    void writeBufferDescriptor(VkDescriptorSet set,
        VkDescriptorType descriptorType,
        uint32_t binding,
        uint32_t arrayElement,
        const Buffer *buffer,
        VkDeviceSize offset = 0,
        VkDeviceSize range = VK_WHOLE_SIZE) const;

    void writeImageDescriptor(VkDescriptorSet set,
        VkDescriptorType descriptorType,
        uint32_t binding,
        uint32_t arrayElement,
        const Image2D *image,
        VkImageLayout imageLayout,
        VkSampler sampler);

    VkShaderModule loadShaderModule(const char *filename) const;

    VkInstance vk_instance = VK_NULL_HANDLE;
    VkSurfaceKHR vk_surface = VK_NULL_HANDLE;

    VkPhysicalDevice vk_physical_device = VK_NULL_HANDLE;
    uint32_t graphicsQueueIndex = UINT32_MAX;
    uint32_t compute_queue_index = UINT32_MAX;
    uint32_t present_queue_index = UINT32_MAX;

    VkDevice vkDevice = VK_NULL_HANDLE;

    VkFence transient_fence = VK_NULL_HANDLE;
    std::unordered_map<uint32_t, VkCommandPool> transient_command_pools;
    std::unordered_map<VkCommandBuffer, uint32_t> transient_command_buffers;

private:
    void createInstance();

    void selectPhysicalDevice();

    void createLogicDevice();
};

class SwapChain {
public:
    void initialize(Device *device);
    void finalize();
    void resize();
    void acquire();
    void present();

    VkSwapchainKHR vkSwapChain = VK_NULL_HANDLE;
    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
    uint32_t currentImageIndex = 0;
    VkFormat imageFormat = VK_FORMAT_UNDEFINED;
    VkColorSpaceKHR imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    VkExtent2D imageExtent = { 0, 0 };

    std::vector<VkImage> vkImages;
    std::vector<VkImageView> vk_image_views;

private:
    void createSwapChain();
    void destroySwapChain();
    void selectPresentMode();
    void selectImageFormat();
    void retrieveImages();
    void createImageViews();

    Device *device = nullptr;
    VkFence acquire_fence = VK_NULL_HANDLE;
};

class DescriptorSetLayout {
public:
    void initialize(Device *device,
        uint32_t bindingCount,
        const VkDescriptorSetLayoutBinding *bindings);
    void finalize();

    template<size_t Count>
    void initialize(Device *device,
        const std::array<VkDescriptorSetLayoutBinding, Count> &bindings) {
        initialize(
            device, static_cast<uint32_t>(bindings.size()), bindings.data());
    }

    VkDescriptorSet allocateSet();
    void freeSet(VkDescriptorSet set) { freeSetList_.push_back(set); }

    [[nodiscard]] const VkDescriptorSetLayout &descriptorSetLayout() const {
        return setLayout_;
    }

private:
    Device *device_ = nullptr;
    VkDescriptorSetLayout setLayout_ = VK_NULL_HANDLE;
    std::vector<VkDescriptorPoolSize> allocationSizes_;

    std::vector<VkDescriptorSet> freeSetList_;
    std::vector<VkDescriptorPool> descriptorPools_;
    size_t poolCapacity_ = 0;
    size_t allocatedSize_ = 0;
};

#endif // GLSL_RAYTRACING_DEVICE_H
