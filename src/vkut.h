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

class IVKUTNotifier;

struct Buffer {
    VkBuffer vk_buffer{ VK_NULL_HANDLE };
    VkDeviceMemory vk_device_memory{ VK_NULL_HANDLE };
};

struct Texture {};

class IVKUTNotifier {
public:
    virtual void on_swap_chain_create() = 0;
    virtual void on_swap_chain_destroy() = 0;
};

// singleton
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

    void acquire();
    void present();
    void resize();
    [[nodiscard]] uint32_t back_image_index() const {
        return back_image_index_;
    }
    [[nodiscard]] VkImage get_swap_chain_image(uint32_t index) const {
        return swap_chain_images_[index];
    }

    // resource creation
    [[nodiscard]] uint32_t get_memory_type(uint32_t type_bits,
        VkMemoryPropertyFlags memory_flags) const;
    void create_buffer(VkDeviceSize size,
        VkBufferUsageFlags usage,
        VkMemoryPropertyFlags memory_flags,
        Buffer *buffer);
    void destroy_buffer(Buffer *buffer);
    void update_buffer_data(Buffer *buffer,
        VkDeviceSize offset,
        VkDeviceSize range,
        const void *data);

    [[nodiscard]] VkCommandBuffer begin_transient(uint32_t queue_family_index);
    void flush_transient(VkCommandBuffer command_buffer);

private:
    void create_instance(const std::vector<const char *> &enabled_layers,
        const std::vector<const char *> &enabled_extensions);
    void select_physical_device();
    void create_logic_device();

    // initialize swap chain
    void create_swap_chain_frame_independent_resources();
    void destroy_swap_chain_frame_independent_resources();
    void create_swap_chain_frame_dependent_resources();
    void destroy_swap_chain_frame_dependent_resources();
    void compute_swap_chain_extent();
    void select_image_format();
    void select_present_mode();

    // device
    VkInstance vk_instance_{ VK_NULL_HANDLE };
    VkSurfaceKHR vk_surface_{ VK_NULL_HANDLE };
    VkPhysicalDevice vk_physical_device_{ VK_NULL_HANDLE };
    VkDevice vk_device_{ VK_NULL_HANDLE };

    // swap chain
    uint32_t present_queue_family_index_{ UINT32_MAX };
    VkExtent2D swap_chain_extent_{ 0, 0 };
    VkFence swap_chain_acquire_fence_{ VK_NULL_HANDLE };
    VkFormat swap_chain_format_{ VK_FORMAT_UNDEFINED };
    VkPresentModeKHR swap_chain_present_mode_{ VK_PRESENT_MODE_FIFO_KHR };
    VkSwapchainKHR vk_swap_chain_{ VK_NULL_HANDLE };
    uint32_t back_image_index_{ 0 };
    std::vector<VkImage> swap_chain_images_;

    // transient command buffer
    std::unordered_map<uint32_t, VkCommandPool> transient_command_pools_;
    std::unordered_map<VkCommandBuffer, uint32_t> transient_command_buffers_;
};

const char *vk_result_string(VkResult result);

#define VKUT_THROW_IF_FAILED(EXPR) \
    do { \
        VkResult r = EXPR; \
        if (r != VK_SUCCESS) { \
            throw std::exception(vk_result_string(r)); \
        } \
    } while (false)

#endif // GLSL_RAYTRACING_VKUT_H
