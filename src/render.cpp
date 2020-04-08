//
// Created by murmur wheel on 2020/3/22.
//

#include "render.h"
#include <array>
#include <stack>
#include <unordered_map>

struct NodePack {
    uint32_t shapeType;
    uint32_t shape;
    uint32_t left;
    uint32_t right;
};

struct ScenePack {
    std::vector<NodePack> nodes;
    std::vector<uint32_t> integers;
    std::vector<glm::vec4> floats;
    std::vector<Short2> lights;

    void initialize(Node *root) {
        clear();

        std::unordered_map<Node *, uint32_t> node2id;
        std::unordered_map<Shape *, uint32_t> object2id;

        uint32_t id_alloc = 0;
        node_dfs_helper(id_alloc, node2id, root);

        nodes.resize(id_alloc);
        for (auto &p : node2id) {
            const auto id_at = [&node2id](Node *ptr) {
                return ptr ? node2id.at(ptr) : UINT32_MAX;
            };

            auto idx = node2id[p.first];
            nodes[idx].left = id_at(p.first->left);
            nodes[idx].right = id_at(p.first->right);
        }
    }

    static void node_dfs_helper(uint32_t &id_alloc,
        std::unordered_map<Node *, uint32_t> &node_id,
        Node *node) {
        node_id[node] = id_alloc++;
        if (node->left) {
            node_dfs_helper(id_alloc, node_id, node->left);
        }
        if (node->right) {
            node_dfs_helper(id_alloc, node_id, node->right);
        }
    }

private:
    void clear() {
        nodes.clear();
        integers.clear();
        floats.clear();
        lights.clear();
    }
};

void Render::initialize(Device *_device,
    SwapChain *_swapChain,
    Scene *_scene,
    const CameraData &_camera) {
    // save arguments
    device = _device;
    swapChain_ = _swapChain;
    scene = _scene;

    // initialize components
    sceneInitialize();
    traceInitialize();
    displayInitialize();

    VkFenceCreateInfo submitFenceInfo = {};
    submitFenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    MUST_SUCCESS(vkCreateFence(
        device->vkDevice, &submitFenceInfo, nullptr, &submitFence_));

    VkCommandPoolCreateInfo commandPoolCreateInfo = {};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.queueFamilyIndex = device->graphicsQueueIndex;
    commandPoolCreateInfo.flags = 0;
    MUST_SUCCESS(vkCreateCommandPool(
        device->vkDevice, &commandPoolCreateInfo, nullptr, &commandPool_));

    commandBuffers_.resize(swapChain_->vkImages.size());
    VkCommandBufferAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.commandPool = commandPool_;
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandBufferCount =
        static_cast<uint32_t>(swapChain_->vkImages.size());
    MUST_SUCCESS(vkAllocateCommandBuffers(
        device->vkDevice, &allocateInfo, commandBuffers_.data()));

    buildCommandBuffer();
}

void Render::finalize() {
    if (submitFence_) {
        vkDestroyFence(device->vkDevice, submitFence_, nullptr);
    }
    if (commandPool_) {
        vkDestroyCommandPool(device->vkDevice, commandPool_, nullptr);
    }

    displayFinalize();
    traceFinalize();
    sceneFinalize();
}

void Render::reset(Scene *_scene, const CameraData &camera) {}

void Render::drawFrame(uint32_t imageIndex) {
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers_[imageIndex];

    VkQueue q = VK_NULL_HANDLE;
    vkGetDeviceQueue(device->vkDevice, device->graphicsQueueIndex, 0, &q);

    MUST_SUCCESS(vkQueueSubmit(q, 1, &submitInfo, submitFence_));
    MUST_SUCCESS(vkWaitForFences(
        device->vkDevice, 1, &submitFence_, VK_FALSE, UINT64_MAX));
    MUST_SUCCESS(vkResetFences(device->vkDevice, 1, &submitFence_));
}

void Render::sceneInitialize() {
    sceneCreateDescriptorPool();
    sceneCreateDescriptorSetLayout();
    sceneAllocateDescriptorSet();

    ScenePack pack_;
    // todo
    // pack_.initialize(scene->root);
    //
    NodePack pack[4] = {};
    pack[0].shapeType = 0;
    pack[1].shapeType = 1;
    pack[2].shapeType = 0;
    pack[3].shapeType = 1;
    sceneCreateStorageBuffer(
        &sceneBuffer_.hierarchyReadonlyBuffer, 0, sizeof(pack), pack);

    // integers storage buffer
    uint32_t integers[4] = { 1, 0, 1, 1 };
    sceneCreateStorageBuffer(
        &sceneBuffer_.integersReadonlyBuffer, 1, sizeof(integers), integers);

    // floats storage buffer
    glm::vec4 floats[1] = {};
    floats[0] = glm::vec4(1, 1, 0, 1);
    sceneCreateStorageBuffer(
        &sceneBuffer_.floatsReadonlyBuffer, 2, sizeof(floats), floats);

    // lights storage buffer
}

void Render::sceneFinalize() {
    vkDestroyDescriptorSetLayout(
        device->vkDevice, sceneBuffer_.descriptorSetLayout, nullptr);
    vkDestroyDescriptorPool(
        device->vkDevice, sceneBuffer_.descriptorPool, nullptr);

    // destroy buffers
    device->destroyBuffer(&sceneBuffer_.hierarchyReadonlyBuffer);
    device->destroyBuffer(&sceneBuffer_.integersReadonlyBuffer);
    device->destroyBuffer(&sceneBuffer_.floatsReadonlyBuffer);
}

void Render::sceneCreateDescriptorPool() {
    std::array<VkDescriptorPoolSize, 1> pool_sizes = {};
    pool_sizes[0].descriptorCount = 4;
    pool_sizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    VkDescriptorPoolCreateInfo descriptor_pool_create_info = {};
    descriptor_pool_create_info.sType =
        VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptor_pool_create_info.maxSets = 1;
    descriptor_pool_create_info.poolSizeCount =
        static_cast<uint32_t>(pool_sizes.size());
    descriptor_pool_create_info.pPoolSizes = pool_sizes.data();
    MUST_SUCCESS(vkCreateDescriptorPool(device->vkDevice,
        &descriptor_pool_create_info,
        nullptr,
        &sceneBuffer_.descriptorPool));
}

void Render::sceneCreateDescriptorSetLayout() {
    std::array<VkDescriptorSetLayoutBinding, 4>
        descriptor_set_layout_bindings = {};
    for (uint32_t i = 0; i < 4; ++i) {
        descriptor_set_layout_bindings[i].binding = i;
        descriptor_set_layout_bindings[i].descriptorCount = 1;
        descriptor_set_layout_bindings[i].descriptorType =
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptor_set_layout_bindings[i].stageFlags =
            VK_SHADER_STAGE_COMPUTE_BIT;
    }

    VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info = {};
    descriptor_set_layout_create_info.sType =
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptor_set_layout_create_info.bindingCount =
        static_cast<uint32_t>(descriptor_set_layout_bindings.size());
    descriptor_set_layout_create_info.pBindings =
        descriptor_set_layout_bindings.data();
    MUST_SUCCESS(vkCreateDescriptorSetLayout(device->vkDevice,
        &descriptor_set_layout_create_info,
        nullptr,
        &sceneBuffer_.descriptorSetLayout));
}

void Render::sceneAllocateDescriptorSet() {
    VkDescriptorSetAllocateInfo descriptor_set_allocate_info = {};
    descriptor_set_allocate_info.sType =
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptor_set_allocate_info.descriptorPool = sceneBuffer_.descriptorPool;
    descriptor_set_allocate_info.descriptorSetCount = 1;
    descriptor_set_allocate_info.pSetLayouts =
        &sceneBuffer_.descriptorSetLayout;

    MUST_SUCCESS(vkAllocateDescriptorSets(device->vkDevice,
        &descriptor_set_allocate_info,
        &sceneBuffer_.descriptorSet));
}

void Render::sceneWriteReadonlyBuffer(uint32_t binding, Buffer *buffer) const {
    VkDescriptorBufferInfo buffer_info = {};
    buffer_info.buffer = buffer->vk_buffer;
    buffer_info.offset = 0;
    buffer_info.range = VK_WHOLE_SIZE;

    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    writeDescriptorSet.dstSet = sceneBuffer_.descriptorSet;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.dstBinding = binding;
    writeDescriptorSet.dstArrayElement = 0;
    writeDescriptorSet.pBufferInfo = &buffer_info;
    vkUpdateDescriptorSets(
        device->vkDevice, 1, &writeDescriptorSet, 0, nullptr);
}

void Render::sceneCreateStorageBuffer(Buffer *buffer,
    uint32_t binding,
    size_t data_size,
    const void *data) const {
    device->createBuffer(data_size,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        buffer);

    device->updateBuffer(buffer,
        0,
        data_size,
        VK_PIPELINE_STAGE_HOST_BIT,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        VK_ACCESS_MEMORY_READ_BIT,
        VK_ACCESS_SHADER_READ_BIT,
        data);

    sceneWriteReadonlyBuffer(binding, buffer);
}

void Render::traceInitialize() {
    traceCreateDescriptorPool();
    traceCreateResultImage();
    traceCreatePipelineLayout();
    traceCreatePipeline();
}

void Render::traceFinalize() {
    vkDestroyPipeline(device->vkDevice, trace_.pipeline, nullptr);
    vkDestroyDescriptorPool(device->vkDevice, trace_.descriptorPool, nullptr);
    vkDestroyPipelineLayout(device->vkDevice, trace_.pipelineLayout, nullptr);

    // result image
    vkDestroySampler(device->vkDevice, trace_.result.immutableSampler, nullptr);
    trace_.result.storageSetLayout.finalize();
    trace_.result.sampledSetLayout.finalize();
    device->destroyImage2D(&trace_.result.image);
}

void Render::traceCreateDescriptorPool() {
    std::array<VkDescriptorPoolSize, 4> descriptorPoolSizes = {};
    descriptorPoolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorPoolSizes[0].descriptorCount = 1; // camera

    descriptorPoolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptorPoolSizes[1].descriptorCount = 4;

    descriptorPoolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    descriptorPoolSizes[2].descriptorCount = 1;

    descriptorPoolSizes[3].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorPoolSizes[3].descriptorCount = 1;

    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
    descriptorPoolCreateInfo.sType =
        VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolCreateInfo.maxSets = 3;
    descriptorPoolCreateInfo.poolSizeCount =
        static_cast<uint32_t>(descriptorPoolSizes.size());
    descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSizes.data();
    MUST_SUCCESS(vkCreateDescriptorPool(device->vkDevice,
        &descriptorPoolCreateInfo,
        nullptr,
        &trace_.descriptorPool));
}

void Render::traceCreateResultImage() {
    device->createImage2D(VK_FORMAT_R32G32B32A32_SFLOAT,
        swapChain_->imageExtent,
        VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT |
            VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        VK_IMAGE_ASPECT_COLOR_BIT,
        &trace_.result.image);

    // create sampler
    VkSamplerCreateInfo samplerCreateInfo = {};
    samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
    samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
    samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
    samplerCreateInfo.compareEnable = VK_FALSE;
    samplerCreateInfo.anisotropyEnable = VK_FALSE;
    samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
    samplerCreateInfo.minFilter = VK_FILTER_NEAREST;
    MUST_SUCCESS(vkCreateSampler(device->vkDevice,
        &samplerCreateInfo,
        nullptr,
        &trace_.result.immutableSampler));

    // create descriptor set layout
    std::array<VkDescriptorSetLayoutBinding, 1> storageSetLayoutBindings = {};
    storageSetLayoutBindings[0].descriptorCount = 1;
    storageSetLayoutBindings[0].binding = 0;
    storageSetLayoutBindings[0].descriptorType =
        VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    storageSetLayoutBindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    trace_.result.storageSetLayout.initialize(
        device, 1, storageSetLayoutBindings.data());

    // allocate descriptor set
    trace_.result.storageSet = trace_.result.storageSetLayout.allocateSet();

    // write descriptor
    VkDescriptorImageInfo storageImageDescriptor = {};
    storageImageDescriptor.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    storageImageDescriptor.imageView = trace_.result.image.vk_image_view;
    storageImageDescriptor.sampler = VK_NULL_HANDLE;
    traceWriteImageDescriptor(trace_.result.storageSet,
        0,
        VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
        &storageImageDescriptor);

    // create read setLayout
    std::array<VkDescriptorSetLayoutBinding, 1> sampledSetLayoutBindings = {};
    sampledSetLayoutBindings[0].descriptorCount = 1;
    sampledSetLayoutBindings[0].descriptorType =
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    sampledSetLayoutBindings[0].binding = 0;
    sampledSetLayoutBindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    sampledSetLayoutBindings[0].pImmutableSamplers =
        &trace_.result.immutableSampler;

    trace_.result.sampledSetLayout.initialize(
        device, 1, sampledSetLayoutBindings.data());

    // allocate read set
    trace_.result.sampledSet = trace_.result.sampledSetLayout.allocateSet();

    // write to sampler
    VkDescriptorImageInfo sampledImageDescriptor = {};
    sampledImageDescriptor.sampler = VK_NULL_HANDLE;
    sampledImageDescriptor.imageView = trace_.result.image.vk_image_view;
    sampledImageDescriptor.imageLayout =
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    traceWriteImageDescriptor(trace_.result.sampledSet,
        0,
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        &sampledImageDescriptor);
}

void Render::traceCreateCameraUniformBuffer() {
    // create buffer
    device->createBuffer(sizeof(CameraData),
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
        &trace_.camera.buffer);

    device->updateBuffer(&trace_.camera.buffer,
        0,
        sizeof(CameraData),
        VK_PIPELINE_STAGE_HOST_BIT,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        VK_ACCESS_HOST_WRITE_BIT,
        VK_ACCESS_SHADER_READ_BIT,
        &trace_.camera.data);

    // create descriptor set layout
    std::array<VkDescriptorSetLayoutBinding, 1> cameraBindings = {};
    cameraBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    cameraBindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    cameraBindings[0].descriptorCount = 1;
    cameraBindings[0].binding = 0;
    trace_.camera.descriptorSetLayout.initialize(
        device, 1, cameraBindings.data());

    // allocate descriptor set
    trace_.camera.descriptorSet =
        trace_.camera.descriptorSetLayout.allocateSet();

    // write descriptor set
    device->writeBufferDescriptor(trace_.camera.descriptorSet,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        0,
        0,
        &trace_.camera.buffer);
}

void Render::traceCreatePipelineLayout() {
    std::array<VkDescriptorSetLayout, 2> descriptor_set_layouts = {};
    descriptor_set_layouts[0] =
        trace_.result.storageSetLayout.descriptorSetLayout();
    descriptor_set_layouts[1] = sceneBuffer_.descriptorSetLayout;

    VkPipelineLayoutCreateInfo pipeline_layout_create_info = {};
    pipeline_layout_create_info.sType =
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_create_info.setLayoutCount =
        static_cast<uint32_t>(descriptor_set_layouts.size());
    pipeline_layout_create_info.pSetLayouts = descriptor_set_layouts.data();

    MUST_SUCCESS(vkCreatePipelineLayout(device->vkDevice,
        &pipeline_layout_create_info,
        nullptr,
        &trace_.pipelineLayout));
}

void Render::traceCreatePipeline() {
    VkPipelineShaderStageCreateInfo shaderStageCreateInfo = {};
    shaderStageCreateInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageCreateInfo.module =
        device->loadShaderModule("res/trace.comp.spv");
    shaderStageCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    shaderStageCreateInfo.pName = "main";

    VkComputePipelineCreateInfo computePipelineCreateInfo = {};
    computePipelineCreateInfo.sType =
        VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    computePipelineCreateInfo.layout = trace_.pipelineLayout;
    computePipelineCreateInfo.stage = shaderStageCreateInfo;
    MUST_SUCCESS(vkCreateComputePipelines(device->vkDevice,
        VK_NULL_HANDLE,
        1,
        &computePipelineCreateInfo,
        nullptr,
        &trace_.pipeline));

    vkDestroyShaderModule(
        device->vkDevice, shaderStageCreateInfo.module, nullptr);
}

void Render::traceUpdateResultImageLayout(VkCommandBuffer commandBuffer,
    VkAccessFlags srcAccessFlags,
    VkAccessFlags dstAccessFlags,
    VkImageLayout oldLayout,
    VkImageLayout newLayout) const {
    VkImageMemoryBarrier imageMemoryBarrier = {};
    imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.srcAccessMask = srcAccessFlags;
    imageMemoryBarrier.dstAccessMask = dstAccessFlags;
    imageMemoryBarrier.oldLayout = oldLayout;
    imageMemoryBarrier.newLayout = newLayout;
    imageMemoryBarrier.image = trace_.result.image.vk_image;
    imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
    imageMemoryBarrier.subresourceRange.levelCount = 1;
    imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
    imageMemoryBarrier.subresourceRange.layerCount = 1;

    vkCmdPipelineBarrier(commandBuffer,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        0,
        0,
        nullptr,
        0,
        nullptr,
        1,
        &imageMemoryBarrier);
}

void Render::traceWriteImageDescriptor(VkDescriptorSet set,
    uint32_t dstBinding,
    VkDescriptorType descriptorType,
    const VkDescriptorImageInfo *imageInfo) {
    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.descriptorType = descriptorType;
    writeDescriptorSet.dstSet = set;
    writeDescriptorSet.dstBinding = dstBinding;
    writeDescriptorSet.dstArrayElement = 0;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.pImageInfo = imageInfo;
    vkUpdateDescriptorSets(
        device->vkDevice, 1, &writeDescriptorSet, 0, nullptr);
}

void Render::traceDispatch(VkCommandBuffer commandBuffer) const {
    // update resultImage Layout
    traceUpdateResultImageLayout(commandBuffer,
        VK_ACCESS_MEMORY_READ_BIT,
        VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_GENERAL);

    // bind resources
    // bind compute pipeline
    std::array<VkDescriptorSet, 2> sets = {};
    sets[0] = trace_.result.storageSet;
    sets[1] = sceneBuffer_.descriptorSet;
    vkCmdBindDescriptorSets(commandBuffer,
        VK_PIPELINE_BIND_POINT_COMPUTE,
        trace_.pipelineLayout,
        0,
        static_cast<uint32_t>(sets.size()),
        sets.data(),
        0,
        nullptr);
    vkCmdBindPipeline(
        commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, trace_.pipeline);
    vkCmdDispatch(commandBuffer,
        swapChain_->imageExtent.width / 2,
        swapChain_->imageExtent.height / 2,
        1);

    // update resultImage layout
    traceUpdateResultImageLayout(commandBuffer,
        VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
        VK_ACCESS_SHADER_READ_BIT,
        VK_IMAGE_LAYOUT_GENERAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void Render::displayInitialize() {
    displayCreateDescriptorPool();
    displayCreateTimesUniformBuffer();
    displayCreateRenderPass();
    displayCreateFramebuffers();
    displayCreatePipelineLayout();
    displayCreatePipeline();
}

void Render::displayFinalize() {
    // times uniform buffer
    display_.times.descriptorSetLayout.finalize();
    device->destroyBuffer(&display_.times.buffer);

    // pipeline
    vkDestroyPipeline(device->vkDevice, display_.pipeline, nullptr);
    vkDestroyPipelineLayout(device->vkDevice, display_.pipelineLayout, nullptr);
    for (auto &f : display_.framebuffers) {
        vkDestroyFramebuffer(device->vkDevice, f, nullptr);
    }
    if (display_.renderPass) {
        vkDestroyRenderPass(device->vkDevice, display_.renderPass, nullptr);
    }
}

void Render::displayCreateDescriptorPool() {
    std::array<VkDescriptorPoolSize, 1> poolSizes = {};
    poolSizes[0].descriptorCount = 1;
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

    VkDescriptorPoolCreateInfo poolCreateInfo = {};
    poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolCreateInfo.maxSets = 1;
    poolCreateInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolCreateInfo.pPoolSizes = poolSizes.data();
}

void Render::displayCreateTimesUniformBuffer() {
    // create buffer
    device->createBuffer(sizeof(TimesData),
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
        &display_.times.buffer);

    // update data
    TimesData data = {};
    data.times = 1;
    device->updateBuffer(&display_.times.buffer,
        0,
        sizeof(TimesData),
        VK_PIPELINE_STAGE_HOST_BIT,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        VK_ACCESS_HOST_WRITE_BIT,
        VK_ACCESS_SHADER_READ_BIT,
        &data);

    // create descriptor set layout
    std::array<VkDescriptorSetLayoutBinding, 1> bindings = {};
    bindings[0].binding = 0;
    bindings[0].descriptorCount = 1;
    bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    display_.times.descriptorSetLayout.initialize(device, bindings);

    // allocate descriptor set
    display_.times.descriptorSet =
        display_.times.descriptorSetLayout.allocateSet();

    // write descriptor set
    device->writeBufferDescriptor(display_.times.descriptorSet,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        0,
        0,
        &display_.times.buffer);
}

void Render::displayCreateRenderPass() {
    std::array<VkAttachmentDescription, 1> attachmentDescriptions = {};
    // color buffer
    attachmentDescriptions[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentDescriptions[0].format = swapChain_->imageFormat;
    attachmentDescriptions[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachmentDescriptions[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    attachmentDescriptions[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachmentDescriptions[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachmentDescriptions[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDescriptions[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    VkAttachmentReference colorReference = {
        0,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    VkSubpassDescription subpassDescription = {};
    subpassDescription.colorAttachmentCount = 1;
    subpassDescription.pColorAttachments = &colorReference;

    std::array<VkSubpassDependency, 2> subpassDependencies = {};
    subpassDependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    subpassDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
    subpassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    subpassDependencies[0].dstSubpass = 0;
    subpassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    subpassDependencies[0].dstStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    subpassDependencies[1].srcSubpass = 0;
    subpassDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    subpassDependencies[1].srcStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    subpassDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    subpassDependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    subpassDependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    VkRenderPassCreateInfo renderPassCreateInfo = {};
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.attachmentCount = 1;
    renderPassCreateInfo.pAttachments = attachmentDescriptions.data();
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpassDescription;
    renderPassCreateInfo.dependencyCount = 2;
    renderPassCreateInfo.pDependencies = subpassDependencies.data();
    MUST_SUCCESS(vkCreateRenderPass(device->vkDevice,
        &renderPassCreateInfo,
        nullptr,
        &display_.renderPass));
}

void Render::displayCreateFramebuffers() {
    display_.framebuffers.resize(swapChain_->vkImages.size());
    for (size_t i = 0; i < display_.framebuffers.size(); ++i) {
        VkFramebufferCreateInfo framebufferCreateInfo = {};
        framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferCreateInfo.renderPass = display_.renderPass;
        framebufferCreateInfo.attachmentCount = 1;
        framebufferCreateInfo.pAttachments = &swapChain_->vk_image_views[i];
        framebufferCreateInfo.width = swapChain_->imageExtent.width;
        framebufferCreateInfo.height = swapChain_->imageExtent.height;
        framebufferCreateInfo.layers = 1;
        MUST_SUCCESS(vkCreateFramebuffer(device->vkDevice,
            &framebufferCreateInfo,
            nullptr,
            &display_.framebuffers[i]));
    }
}

void Render::displayCreatePipelineLayout() {
    std::array<VkDescriptorSetLayout, 2> setLayouts = {};
    setLayouts[0] = trace_.result.sampledSetLayout.descriptorSetLayout();
    setLayouts[1] = display_.times.descriptorSetLayout.descriptorSetLayout();

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount =
        static_cast<uint32_t>(setLayouts.size());
    pipelineLayoutInfo.pSetLayouts = setLayouts.data();

    MUST_SUCCESS(vkCreatePipelineLayout(device->vkDevice,
        &pipelineLayoutInfo,
        nullptr,
        &display_.pipelineLayout));
}

void Render::displayCreatePipeline() {
    std::vector<std::pair<VkShaderModule, VkShaderStageFlagBits>>
        shaderStages = {
            {
                device->loadShaderModule("res/display.vert.spv"),
                VK_SHADER_STAGE_VERTEX_BIT,
            },
            {
                device->loadShaderModule("res/display.frag.spv"),
                VK_SHADER_STAGE_FRAGMENT_BIT,
            },
        };
    std::array<VkPipelineShaderStageCreateInfo, 2> shaderStageCreateInfos = {};
    for (int i = 0; i < 2; ++i) {
        shaderStageCreateInfos[i].sType =
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStageCreateInfos[i].module = shaderStages[i].first;
        shaderStageCreateInfos[i].stage = shaderStages[i].second;
        shaderStageCreateInfos[i].pName = "main";
    }

    VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo = {};
    vertexInputStateCreateInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo = {};
    inputAssemblyStateCreateInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo = {};
    rasterizationStateCreateInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizationStateCreateInfo.cullMode = VK_CULL_MODE_NONE;
    rasterizationStateCreateInfo.lineWidth = 1.0f;

    VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo = {};
    multisampleStateCreateInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {};
    viewportStateCreateInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportStateCreateInfo.scissorCount = 1;
    viewportStateCreateInfo.viewportCount = 1;

    std::array<VkPipelineColorBlendAttachmentState, 1> attachmentStates = {};
    attachmentStates[0].colorWriteMask = 0x0f;
    attachmentStates[0].blendEnable = VK_FALSE;
    VkPipelineColorBlendStateCreateInfo blendStateCreateInfo = {};
    blendStateCreateInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    blendStateCreateInfo.attachmentCount = 1;
    blendStateCreateInfo.pAttachments = &attachmentStates[0];

    std::array<VkDynamicState, 2> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
    };
    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {};
    dynamicStateCreateInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateCreateInfo.dynamicStateCount = 2;
    dynamicStateCreateInfo.pDynamicStates = dynamicStates.data();

    VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = {};
    graphicsPipelineCreateInfo.sType =
        VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    graphicsPipelineCreateInfo.stageCount = 2;
    graphicsPipelineCreateInfo.pStages = shaderStageCreateInfos.data();

    graphicsPipelineCreateInfo.pVertexInputState = &vertexInputStateCreateInfo;
    graphicsPipelineCreateInfo.pInputAssemblyState =
        &inputAssemblyStateCreateInfo;
    graphicsPipelineCreateInfo.pRasterizationState =
        &rasterizationStateCreateInfo;
    graphicsPipelineCreateInfo.pMultisampleState = &multisampleStateCreateInfo;
    graphicsPipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
    graphicsPipelineCreateInfo.pColorBlendState = &blendStateCreateInfo;
    graphicsPipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;
    graphicsPipelineCreateInfo.renderPass = display_.renderPass;
    graphicsPipelineCreateInfo.layout = display_.pipelineLayout;

    MUST_SUCCESS(vkCreateGraphicsPipelines(device->vkDevice,
        VK_NULL_HANDLE,
        1,
        &graphicsPipelineCreateInfo,
        nullptr,
        &display_.pipeline));

    for (auto &s : shaderStages) {
        vkDestroyShaderModule(device->vkDevice, s.first, nullptr);
    }
}

void Render::displayDraw(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
    std::array<VkClearValue, 1> clear_values = {};
    clear_values[0].color.float32[0] = 1.0f;
    clear_values[0].color.float32[1] = 0.0f;
    clear_values[0].color.float32[2] = 0.0f;
    clear_values[0].color.float32[3] = 1.0f;

    VkRenderPassBeginInfo renderPassBeginInfo = {};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = display_.renderPass;
    renderPassBeginInfo.framebuffer = display_.framebuffers[imageIndex];
    renderPassBeginInfo.clearValueCount = 1;
    renderPassBeginInfo.pClearValues = clear_values.data();
    renderPassBeginInfo.renderArea.offset = {};
    renderPassBeginInfo.renderArea.extent = swapChain_->imageExtent;
    vkCmdBeginRenderPass(
        commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport = {};
    viewport.x = 0;
    viewport.y = 0;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    viewport.width = float(swapChain_->imageExtent.width);
    viewport.height = float(swapChain_->imageExtent.height);
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &renderPassBeginInfo.renderArea);

    std::array<VkDescriptorSet, 2> descriptorSets = {};
    descriptorSets[0] = trace_.result.sampledSet;
    descriptorSets[1] = display_.times.descriptorSet;
    vkCmdBindDescriptorSets(commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        display_.pipelineLayout,
        0,
        static_cast<uint32_t>(descriptorSets.size()),
        descriptorSets.data(),
        0,
        nullptr);
    vkCmdBindPipeline(
        commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, display_.pipeline);
    vkCmdDraw(commandBuffer, 6, 1, 0, 0);

    vkCmdEndRenderPass(commandBuffer);
}

void Render::buildCommandBuffer() {
    for (size_t i = 0; i < commandBuffers_.size(); ++i) {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        MUST_SUCCESS(vkBeginCommandBuffer(commandBuffers_[i], &beginInfo));

        traceDispatch(commandBuffers_[i]);
        displayDraw(commandBuffers_[i], uint32_t(i));

        VkImageMemoryBarrier barrier = {};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.image = swapChain_->vkImages[i];
        barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        vkCmdPipelineBarrier(commandBuffers_[i],
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            0,
            0,
            nullptr,
            0,
            nullptr,
            1,
            &barrier);

        MUST_SUCCESS(vkEndCommandBuffer(commandBuffers_[i]));
    }
}
