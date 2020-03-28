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
        Buffer hierarchyReadonlyBuffer;
        Buffer integersReadonlyBuffer;
        Buffer floatsReadonlyBuffer;
        Buffer lightsReadonlyBuffer;

        VkDescriptorSetLayout sceneDescriptorSetLayout;
        VkDescriptorSet sceneDescriptorSet;
        VkDescriptorPool sceneDescriptorSetPool;
    };

    void sceneInitialize();
    void sceneFinalize();

    struct Trace {
        VkDescriptorPool descriptorPool;

        Image2D resultImage;
        VkSampler resultImmutableSampler;
        VkDescriptorSetLayout resultWriteSetLayout;
        VkDescriptorSet resultWriteSet;
        VkDescriptorSetLayout resultReadSetLayout;
        VkDescriptorSet resultReadSet;

        Buffer cameraUniformBuffer;
        VkDescriptorSetLayout cameraDescriptorSetLayout;
        VkDescriptorSet cameraDescriptorSet;

        VkPipelineLayout pipelineLayout;
        VkPipeline pipeline;
    };

    void traceInitialize();
    void traceFinalize();
    void traceCreateDescriptorPool();
    void traceCreateResultImage();
    void traceCreatePipelineLayout();
    void traceCreatePipeline();
    void traceUpdateResultImageLayout(VkCommandBuffer commandBuffer,
        VkAccessFlags srcAccessFlags,
        VkAccessFlags dstAccessFlags,
        VkImageLayout oldLayout,
        VkImageLayout newLayout);
    void traceWriteDescriptor(VkDescriptorSet set,
        uint32_t dstBinding,
        VkDescriptorType descriptorType,
        const VkDescriptorImageInfo *imageInfo);
    void traceDispatch(VkCommandBuffer commandBuffer);

    struct Display {
        VkRenderPass renderPass = VK_NULL_HANDLE;
        std::vector<VkFramebuffer> framebuffers;

        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
        VkPipeline pipeline = VK_NULL_HANDLE;
    };

    void displayInitialize();
    void displayFinalize();
    void displayCreateRenderPass();
    void displayCreateFramebuffers();
    void displayCreatePipelineLayout();
    void displayCreatePipeline();
    void displayDraw(VkCommandBuffer commandBuffer, uint32_t imageIndex);

    void buildCommandBuffer();

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
