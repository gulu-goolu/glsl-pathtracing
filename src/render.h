//
// Created by murmur wheel on 2020/3/22.
//

#ifndef GLSL_RAYTRACING_RENDER_H
#define GLSL_RAYTRACING_RENDER_H

#include "device.h"
#include "glm/glm.hpp"
#include "scene.h"

struct Camera {
    glm::vec3 glm;
};

class Render {
public:
    void initialize(Device *device,
        SwapChain *swap_chain,
        Scene *_scene,
        const Camera &_camera);
    void finalize();

    void updateCamera(const Camera &camera);

    // record render commands
    void drawFrame(uint32_t image_index);

private:
    struct Buffer {
        VkDeviceMemory vk_device_memory;
        VkBuffer vk_buffer;
    };

    struct Image {
        VkDeviceMemory vk_device_memory;
        VkImage vk_image;
        VkImageView vk_image_view;
    };

    struct SceneBuffer {
        Buffer hierarchy_buffer;
        Buffer integers_buffer;
        Buffer floats_buffer;
        Buffer lights_buffer;

        VkDescriptorSetLayout descriptor_set_layout;
        VkDescriptorSet descriptor_set;
        VkDescriptorPool descriptor_set_pool;
    };

    struct Trace {
        VkDescriptorSetLayout scene_set_layout;
        VkDescriptorSet scene_set;
        VkDescriptorPool descriptor_pool;

        Buffer buffer;
        Image result;
    };

    struct Display {
        VkRenderPass renderPass = VK_NULL_HANDLE;
        std::vector<VkFramebuffer> framebuffers;

        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
        VkPipeline pipeline = VK_NULL_HANDLE;
    };

    void createSceneBuffer();
    void destroySceneBuffer();

    void createTrace();
    void destroyTrace();

    void createDisplay();
    void destroyDisplay();
    void createDisplayRenderPass();
    void createDisplayFramebuffers();
    void createDisplayPipelineLayout();
    void createDisplayPipeline();
    void displayDraw(VkCommandBuffer commandBuffer, uint32_t imageIndex);

    void buildCommandBuffer();

    VkShaderModule loadShaderModule(const char *filename);

    Device *device = nullptr;
    SwapChain *swap_chain = nullptr;
    Scene *scene = nullptr;
    VkFence submit_fence_ = VK_NULL_HANDLE;

    SceneBuffer scene_buffer_;
    Trace trace_;
    Display display_;

    VkCommandPool command_pool_ = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> command_buffers_;
};

#endif // GLSL_RAYTRACING_RENDER_H
