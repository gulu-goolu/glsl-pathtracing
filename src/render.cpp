//
// Created by murmur wheel on 2020/3/22.
//

#include "render.h"
#include <array>
#include <stack>
#include <unordered_map>

struct NodePack {
    uint32_t type;
    uint32_t object;
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

        std::unordered_map<Node *, uint32_t> node_id;
        std::unordered_map<Shape *, uint32_t> object_id;

        uint32_t id_alloc = 0;
        node_dfs_helper(id_alloc, node_id, root);

        nodes.resize(id_alloc);
        for (auto &p : node_id) {
            const auto id_at = [&node_id](Node *ptr) {
                return ptr ? node_id.at(ptr) : UINT32_MAX;
            };

            auto idx = node_id[p.first];
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
    SwapChain *_swap_chain,
    Scene *_scene,
    const Camera &_camera) {
    device = _device;
    swap_chain = _swap_chain;
    scene = _scene;

    createSceneBuffer();
    createTrace();
    createDisplay();

    VkFenceCreateInfo submitFenceInfo = {};
    submitFenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    MUST_SUCCESS(vkCreateFence(
        device->vk_device, &submitFenceInfo, nullptr, &submit_fence_));

    VkCommandPoolCreateInfo commandPoolCreateInfo = {};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.queueFamilyIndex = device->graphics_queue_index;
    commandPoolCreateInfo.flags = 0;
    MUST_SUCCESS(vkCreateCommandPool(
        device->vk_device, &commandPoolCreateInfo, nullptr, &command_pool_));

    command_buffers_.resize(swap_chain->vk_images.size());
    VkCommandBufferAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.commandPool = command_pool_;
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandBufferCount =
        static_cast<uint32_t>(swap_chain->vk_images.size());
    MUST_SUCCESS(vkAllocateCommandBuffers(
        device->vk_device, &allocateInfo, command_buffers_.data()));

    buildCommandBuffer();
}

void Render::finalize() {
    if (submit_fence_) {
        vkDestroyFence(device->vk_device, submit_fence_, nullptr);
    }
    if (command_pool_) {
        vkDestroyCommandPool(device->vk_device, command_pool_, nullptr);
    }

    destroyDisplay();
    destroyTrace();
    destroySceneBuffer();
}

void Render::updateCamera(const Camera &camera) {}

void Render::drawFrame(uint32_t image_index) {
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &command_buffers_[image_index];

    VkQueue q = VK_NULL_HANDLE;
    vkGetDeviceQueue(device->vk_device, device->graphics_queue_index, 0, &q);

    MUST_SUCCESS(vkQueueSubmit(q, 1, &submitInfo, submit_fence_));
    MUST_SUCCESS(vkWaitForFences(
        device->vk_device, 1, &submit_fence_, VK_FALSE, UINT64_MAX));
    MUST_SUCCESS(vkResetFences(device->vk_device, 1, &submit_fence_));
}

void Render::createSceneBuffer() {
    ScenePack pack_;
    // todo
    // pack_.initialize(scene->root);
}

void Render::destroySceneBuffer() {}

void Render::createTrace() {}

void Render::destroyTrace() {}

void Render::createDisplay() {
    createDisplayRenderPass();
    createDisplayFramebuffers();
    createDisplayPipelineLayout();
    createDisplayPipeline();
}

void Render::destroyDisplay() {
    vkDestroyPipeline(device->vk_device, display_.pipeline, nullptr);
    vkDestroyPipelineLayout(
        device->vk_device, display_.pipelineLayout, nullptr);
    for (auto &f : display_.framebuffers) {
        vkDestroyFramebuffer(device->vk_device, f, nullptr);
    }
    if (display_.renderPass) {
        vkDestroyRenderPass(device->vk_device, display_.renderPass, nullptr);
    }
}

void Render::createDisplayRenderPass() {
    std::array<VkAttachmentDescription, 1> attachmentDescriptions = {};
    // color buffer
    attachmentDescriptions[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentDescriptions[0].format = swap_chain->image_format;
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
    MUST_SUCCESS(vkCreateRenderPass(device->vk_device,
        &renderPassCreateInfo,
        nullptr,
        &display_.renderPass));
}

void Render::createDisplayFramebuffers() {
    display_.framebuffers.resize(swap_chain->vk_images.size());
    for (size_t i = 0; i < display_.framebuffers.size(); ++i) {
        VkFramebufferCreateInfo framebufferCreateInfo = {};
        framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferCreateInfo.renderPass = display_.renderPass;
        framebufferCreateInfo.attachmentCount = 1;
        framebufferCreateInfo.pAttachments = &swap_chain->vk_image_views[i];
        framebufferCreateInfo.width = swap_chain->image_extent.width;
        framebufferCreateInfo.height = swap_chain->image_extent.height;
        framebufferCreateInfo.layers = 1;
        MUST_SUCCESS(vkCreateFramebuffer(device->vk_device,
            &framebufferCreateInfo,
            nullptr,
            &display_.framebuffers[i]));
    }
}

void Render::createDisplayPipelineLayout() {
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
    pipelineLayoutCreateInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    MUST_SUCCESS(vkCreatePipelineLayout(device->vk_device,
        &pipelineLayoutCreateInfo,
        nullptr,
        &display_.pipelineLayout));
}

void Render::createDisplayPipeline() {
    std::vector<std::pair<VkShaderModule, VkShaderStageFlagBits>>
        shaderStages = {
            {
                loadShaderModule("res/display.vert.spv"),
                VK_SHADER_STAGE_VERTEX_BIT,
            },
            {
                loadShaderModule("res/display.frag.spv"),
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

    MUST_SUCCESS(vkCreateGraphicsPipelines(device->vk_device,
        VK_NULL_HANDLE,
        1,
        &graphicsPipelineCreateInfo,
        nullptr,
        &display_.pipeline));

    for (auto &s : shaderStages) {
        vkDestroyShaderModule(device->vk_device, s.first, nullptr);
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
    renderPassBeginInfo.renderArea.extent = swap_chain->image_extent;
    vkCmdBeginRenderPass(
        commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport = {};
    viewport.x = 0;
    viewport.y = 0;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    viewport.width = float(swap_chain->image_extent.width);
    viewport.height = float(swap_chain->image_extent.height);
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &renderPassBeginInfo.renderArea);

    vkCmdBindPipeline(
        commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, display_.pipeline);
    vkCmdDraw(commandBuffer, 3, 1, 0, 0);

    vkCmdEndRenderPass(commandBuffer);
}

void Render::buildCommandBuffer() {
    for (size_t i = 0; i < command_buffers_.size(); ++i) {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        MUST_SUCCESS(vkBeginCommandBuffer(command_buffers_[i], &beginInfo));

        displayDraw(command_buffers_[i], uint32_t(i));

        VkImageMemoryBarrier barrier = {};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.image = swap_chain->vk_images[i];
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
        vkCmdPipelineBarrier(command_buffers_[i],
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            0,
            0,
            nullptr,
            0,
            nullptr,
            1,
            &barrier);

        MUST_SUCCESS(vkEndCommandBuffer(command_buffers_[i]));
    }
}

VkShaderModule Render::loadShaderModule(const char *filename) {
    auto blob = readFile(filename);
    if (blob.empty()) {
        return VK_NULL_HANDLE;
    }

    VkShaderModule shaderModule = VK_NULL_HANDLE;
    VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
    shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModuleCreateInfo.codeSize = static_cast<uint32_t>(blob.size());
    shaderModuleCreateInfo.pCode =
        reinterpret_cast<const uint32_t *>(blob.data());
    MUST_SUCCESS(vkCreateShaderModule(
        device->vk_device, &shaderModuleCreateInfo, nullptr, &shaderModule));

    return shaderModule;
}
