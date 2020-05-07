// #include "app.h"
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <vkut/buffer.h>
#include <vkut/swapchain.h>

using namespace vkut;

int main() {
    if (!glfwInit()) {
        return 1;
    }
    Device::startup_game();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    auto window = glfwCreateWindow(1920, 1080, "demo", nullptr, nullptr);
    auto hwnd = glfwGetWin32Window(window);
    VkWin32SurfaceCreateInfoKHR surface_create_info = {};
    surface_create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surface_create_info.hwnd = hwnd;
    surface_create_info.hinstance = GetModuleHandle(NULL);
    auto surface = vkut::make_ptr<vkut::Surface>(&surface_create_info);

    auto sc = vkut::make_ptr<vkut::Swapchain>(surface);
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        sc->acquire();

        uint32_t idx = sc->back_image_index();
        auto cmd = vkut::Device::get()->begin_transient_universal();

        VkImageMemoryBarrier b = {};
        b.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        b.image = sc->get_image(idx)->vk_image();
        b.subresourceRange = {
            VK_IMAGE_ASPECT_COLOR_BIT,
            0,
            1,
            0,
            1,
        };
        b.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        b.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        b.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        b.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        b.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        b.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        vkCmdPipelineBarrier(cmd,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            0,
            0,
            nullptr,
            0,
            nullptr,
            1,
            &b);

        vkut::Device::get()->flush_transient(cmd);

        sc->present();
    }
    sc.reset();
    surface.reset();

    Device::shutdown();
    glfwTerminate();
}
