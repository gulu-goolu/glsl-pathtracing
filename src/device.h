//
// Created by murmur.wheel@gmail.com on 2020/5/23.
//

#ifndef GLSL_RAYTRACING_DEVICE_H
#define GLSL_RAYTRACING_DEVICE_H

#include <vector>
#include <vulkan/vulkan.h>

class SwapChain {
public:
    void acqurie();
    void present();
    [[nodiscard]] uint32_t back_image_index();
    VkImage get_image(uint32_t index)const;
    VkImageView get_image_view(uint32_t index)const;
};

class Device {
#ifdef VK_USE_PLATFORM_WIN32_KHR
    static void startup_win32(HINSTANCE hinstance, HWND hwnd);
#endif //! VK_USE_PLATFORM_WIN32_KHR

    // destroy this singleton object
    static void shutdown();

    // get instance
    static Device *get();

    [[nodiscard]] const VkInstance &vk_instance() const { return vk_instance_; }
    [[nodiscard]] const VkSurfaceKHR &vk_surface() const { return vk_surface_; }
    [[nodiscard]] const VkPhysicalDevice &vk_physical_device() const {
        return vk_physical_device_;
    }
    [[nodiscard]] const VkDevice &vk_device() const { return vk_device_; }
    [[nodiscard]] SwapChain *swap_chain() const { return swap_chain_; }

    [[nodiscard]] uint32_t get_memory_type(uint32_t type_bits,
        VkMemoryPropertyFlags memory_flags) const;

    void create_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memory_flags);
    void destroy_buffer();

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
};

const char *vk_result_string(VkResult result);

#define VK_THROW_IF_FAILED(EXPR) \
    do { \
        VkResult r = EXPR; \
        if (r != VK_SUCCESS) { \
            throw std::exception(vk_result_string(r)); \
        } \
    } while (false)

#endif // GLSL_RAYTRACING_DEVICE_H
