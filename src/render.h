//
// Created by murmur wheel on 2020/3/22.
//

#ifndef GLSL_RAYTRACING_RENDER_H
#define GLSL_RAYTRACING_RENDER_H

#include "device.h"
#include "glm/glm.hpp"
#include "scene.h"

struct CameraData {
    glm::vec4 from;
    glm::vec4 to;
    glm::vec4 up;
    float fov_angle;
    float aspect;

    // for align
    float reserved1;
    float reserved2;

    glm::vec4 top_left_corner;
    glm::vec4 vertical;
    glm::vec4 horizontal;
};

class Render {
public:
    void initialize(Device *_device,
        SwapChain *_swapChain,
        Scene *_scene,
        const CameraData &_camera);
    void finalize();

    void reset(Scene *_scene, const CameraData &camera);

    // record render commands
    void drawFrame(uint32_t imageIndex);

private:
    struct SceneBuffer {
        Buffer hierarchyReadonlyBuffer;
        Buffer integersReadonlyBuffer;
        Buffer floatsReadonlyBuffer;
        Buffer lightsReadonlyBuffer;

        VkDescriptorSetLayout descriptorSetLayout;
        VkDescriptorSet descriptorSet;
        VkDescriptorPool descriptorPool;
    };

    void sceneInitialize();
    void sceneFinalize();
    void sceneCreateDescriptorPool();
    void sceneCreateDescriptorSetLayout();
    void sceneAllocateDescriptorSet();
    void sceneWriteReadonlyBuffer(uint32_t binding, Buffer *buffer) const;
    void sceneCreateStorageBuffer(Buffer *buffer,
        uint32_t binding,
        size_t data_size,
        const void *data) const;

    struct ResultImage {
        Image2D image;
        VkSampler immutableSampler = VK_NULL_HANDLE;
        DescriptorSetLayout storageSetLayout;
        VkDescriptorSet storageSet = VK_NULL_HANDLE;
        DescriptorSetLayout sampledSetLayout;
        VkDescriptorSet sampledSet = VK_NULL_HANDLE;
    };

    struct CameraUBO {
        Buffer buffer;

        CameraData data;

        DescriptorSetLayout descriptorSetLayout;
        VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
    };

    struct Trace {
        VkDescriptorPool descriptorPool;

        ResultImage result;
        CameraUBO camera;

        VkPipelineLayout pipelineLayout;
        VkPipeline pipeline;
    };

    void traceInitialize();
    void traceFinalize();
    void traceCreateDescriptorPool();
    void traceCreateResultImage();
    void traceCreateCameraUniformBuffer();
    void traceCreatePipelineLayout();
    void traceCreatePipeline();
    void traceUpdateResultImageLayout(VkCommandBuffer commandBuffer,
        VkAccessFlags srcAccessFlags,
        VkAccessFlags dstAccessFlags,
        VkImageLayout oldLayout,
        VkImageLayout newLayout) const;
    void traceWriteImageDescriptor(VkDescriptorSet set,
        uint32_t dstBinding,
        VkDescriptorType descriptorType,
        const VkDescriptorImageInfo *imageInfo);
    void trace_dispatch(VkCommandBuffer commandBuffer) const;

    struct TimesData {
        uint32_t times;
        uint32_t reserved1;
        uint32_t reserved2;
        uint32_t reserved3;
    };

    struct TimesUniformBuffer {
        Buffer buffer;
        TimesData data;
        DescriptorSetLayout descriptorSetLayout;
        VkDescriptorSet descriptorSet;
    };

    struct Display {
        TimesUniformBuffer times;

        VkRenderPass renderPass = VK_NULL_HANDLE;
        std::vector<VkFramebuffer> framebuffers;

        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
        VkPipeline pipeline = VK_NULL_HANDLE;
    };

    void display_initialize();
    void display_finalize();
    void display_createTimesUniformBuffer();
    void display_createRenderPass();
    void display_createFrameBuffers();
    void display_createPipelineLayout();
    void display_createPipeline();
    void display_draw(VkCommandBuffer commandBuffer, uint32_t imageIndex);

    void buildCommandBuffer();

    Device *device = nullptr;
    SwapChain *swapChain_ = nullptr;
    Scene *scene = nullptr;

    SceneBuffer sceneBuffer_;
    Trace trace_;
    Display display_;

    VkCommandPool commandPool_ = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> commandBuffers_;
    VkFence submitFence_ = VK_NULL_HANDLE;
};

#endif // GLSL_RAYTRACING_RENDER_H
