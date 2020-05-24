//
// Created by murmur.wheel@gmail.com on 2020/5/23.
//

#ifndef GLSL_RAYTRACING_VKUT_H
#define GLSL_RAYTRACING_VKUT_H

#include <unordered_map>
#include <vector>
// clang-format off
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
// clang-format on

class SwapChain;
class IVKUTNotifier;

struct Buffer {
    VkBuffer vk_buffer{ VK_NULL_HANDLE };
    VkDeviceMemory vk_device_memory{ VK_NULL_HANDLE };
};

struct Texture {};

class IVKUTNotifier {
public:
    virtual void on_swap_chain_create(SwapChain *swap_chain) = 0;
    virtual void on_swap_chain_destroy(SwapChain *swap_chain) = 0;
};

class VKUT {
public:
    // startup VKUT
    static void startup(GLFWwindow *window);

    // destroy this singleton object
    static void shutdown();

    // get instance
    static VKUT *get();

    [[nodiscard]] const VkInstance &vk_instance() const { return vk_instance_; }
    [[nodiscard]] const VkSurfaceKHR &vk_surface() const { return vk_surface_; }
    [[nodiscard]] const VkPhysicalDevice &vk_physical_device() const {
        return vk_physical_device_;
    }
    [[nodiscard]] const VkDevice &vk_device() const { return vk_device_; }
    [[nodiscard]] SwapChain *swap_chain() const { return swap_chain_; }

    [[nodiscard]] uint32_t get_memory_type(uint32_t type_bits,
        VkMemoryPropertyFlags memory_flags) const;
    void create_buffer(VkDeviceSize size,
        VkBufferUsageFlags usage,
        VkMemoryPropertyFlags memory_flags,
        Buffer *buffer);
    void destroy_buffer(Buffer *buffer);

    [[nodiscard]] VkCommandBuffer begin_transient(uint32_t queue_family_index);
    void flush_transient(VkCommandBuffer command_buffer);

private:
    void create_instance(const std::vector<const char *> &enabled_layers,
        const std::vector<const char *> &enabled_extensions);
    void select_physical_device();
    void create_logic_device();
    void create_swap_chain();

    VkInstance vk_instance_{ VK_NULL_HANDLE };
    VkSurfaceKHR vk_surface_{ VK_NULL_HANDLE };
    VkPhysicalDevice vk_physical_device_{ VK_NULL_HANDLE };
    VkDevice vk_device_{ VK_NULL_HANDLE };
    SwapChain *swap_chain_{ nullptr };

    // for transient command buffer usage
    std::unordered_map<uint32_t, VkCommandPool> transient_command_pools_;
    std::unordered_map<VkCommandBuffer, uint32_t> transient_command_buffers_;
};

class SwapChain {
public:
    void acquire();
    void present();
    [[nodiscard]] uint32_t back_image_index() const {
        return back_image_index_;
    }
    const VkImage &get_image(uint32_t index) const { return vk_images_[index]; }
    const VkImageView &get_image_view(uint32_t index) const {
        return vk_image_views_[index];
    }

private:
    VkFence acquire_fence_{ VK_NULL_HANDLE };
    VkSwapchainKHR vk_swapchain_{ VK_NULL_HANDLE };
    std::vector<VkImage> vk_images_;
    std::vector<VkImageView> vk_image_views_;
    uint32_t back_image_index_{ 0 };
};

const char *vk_result_string(VkResult result);

#define VK_THROW_IF_FAILED(EXPR) \
    do { \
        VkResult r = EXPR; \
        if (r != VK_SUCCESS) { \
            throw std::exception(vk_result_string(r)); \
        } \
    } while (false)

#endif // GLSL_RAYTRACING_VKUT_H
