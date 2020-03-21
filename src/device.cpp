//
// Created by murmur wheel on 2020/3/22.
//

#include "device.h"

Device::Device(bool enableDebug) {
    createInstance(enableDebug);
}

Device::~Device() {
    if (inst_) {
        vkDestroyInstance(inst_, nullptr);
    }
}

void Device::createInstance(bool debug) {
    auto curLayers = enumerateInstanceLayers();
    std::vector<std::string> layers = requiredDebugLayers();
    std::vector<const char *> layers_ptr;
    for (auto &l : layers) {
        if (curLayers.find(l) != curLayers.end()) {
            layers_ptr.push_back(l.c_str());
        }
    }

    auto curExtensions = enumerateInstanceExtensions();
    std::vector<std::string> wantExtensions = neededInstanceExtensions();
    std::vector<const char *> extensions_ptr;
    for (auto &ext : wantExtensions) {
        if (curExtensions.find(ext) == curExtensions.end()) {
            char msg[512] = {};
            sprintf_s(msg, "not found extension: %s", ext.c_str());
            perror(msg);
            exit(1);
        } else {
            extensions_ptr.push_back(ext.c_str());
        }
    }

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.enabledExtensionCount =
        static_cast<uint32_t>(extensions_ptr.size());
    createInfo.ppEnabledExtensionNames = extensions_ptr.data();
    createInfo.enabledLayerCount = static_cast<uint32_t>(layers_ptr.size());
    createInfo.ppEnabledLayerNames = layers_ptr.data();
    MUST_SUCCESS(vkCreateInstance(&createInfo, nullptr, &inst_));
}

std::unordered_set<std::string> Device::enumerateInstanceLayers() {
    uint32_t cnt = 0;
    MUST_SUCCESS(vkEnumerateInstanceLayerProperties(&cnt, nullptr));

    std::vector<VkLayerProperties> layers(cnt);
    MUST_SUCCESS(vkEnumerateInstanceLayerProperties(&cnt, nullptr));

    std::unordered_set<std::string> res;
    for (auto &l : layers) {
        res.insert(l.layerName);
    }

    return res;
}

std::unordered_set<std::string> Device::enumerateInstanceExtensions() {
    uint32_t cnt = 0;
    MUST_SUCCESS(
        vkEnumerateInstanceExtensionProperties(nullptr, &cnt, nullptr));

    std::vector<VkExtensionProperties> props(cnt);
    MUST_SUCCESS(
        vkEnumerateInstanceExtensionProperties(nullptr, &cnt, props.data()));

    std::unordered_set<std::string> res;
    for (auto p : props) {
        res.insert(p.extensionName);
    }

    return res;
}

std::vector<std::string> Device::requiredDebugLayers() {
    std::vector<std::string> layers = {};
    layers.emplace_back("VK_LAYER_LUNARG_standard_validation");
    return layers;
}

std::vector<std::string> Device::neededInstanceExtensions() {
    std::vector<std::string> extensions;
    extensions.emplace_back(VK_KHR_SURFACE_EXTENSION_NAME);

#if defined(VK_USE_PLATFORM_WIN32_KHR)
    extensions.emplace_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#endif

    return extensions;
}

std::vector<std::string> Device::neededDeviceExtensions() {
    std::vector<std::string> extensions;

    extensions.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    return extensions;
}
