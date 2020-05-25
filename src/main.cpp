//
// Created by murmur.wheel@gmail.com on 2020/5/23.
//

#include "vkut.h"

int main() {
    if (!glfwInit()) {
        return 1;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    auto window = glfwCreateWindow(1920, 1080, "window", nullptr, nullptr);
    VKUT::startup(window);

    Buffer buffer;
    VKUT::get()->create_buffer(1024,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &buffer);
    VKUT::get()->destroy_buffer(&buffer);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        VKUT::get()->acquire();

        auto cmd = VKUT::get()->begin_transient(0);

        VkImageMemoryBarrier barrier = {};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.image =
            VKUT::get()->get_swap_chain_image(VKUT::get()->back_image_index());
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        barrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

        vkCmdPipelineBarrier(cmd,
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            0,
            0,
            nullptr,
            0,
            nullptr,
            1,
            &barrier);

        VKUT::get()->flush_transient(cmd);

        VKUT::get()->present();
    }

    VKUT::shutdown();
    glfwTerminate();
}