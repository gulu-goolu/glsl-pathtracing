//
// Created by murmur wheel on 2020/3/22.
//

#ifndef GLSL_RAYTRACING_DEVICE_H
#define GLSL_RAYTRACING_DEVICE_H

#include <unordered_set>
#include <vulkan/vulkan.h>

#define MUST_SUCCESS(EXPR) \
    do { \
        VkResult res = EXPR; \
        if (res != VK_SUCCESS) { \
            abort(); \
        } \
    } while (false)

class Device {
public:
    explicit Device(bool enableDebug);
    ~Device();

    VkInstance inst_ = VK_NULL_HANDLE;
    VkPhysicalDevice phyDev_ = VK_NULL_HANDLE;
    VkDevice dev_ = VK_NULL_HANDLE;

private:
    void createInstance(bool debug);

    static std::unordered_set<std::string> enumerateInstanceLayers();
    static std::unordered_set<std::string> enumerateInstanceExtensions();
    static std::vector<std::string> requiredDebugLayers();
    static std::vector<std::string> neededInstanceExtensions();
    static std::vector<std::string> neededDeviceExtensions();
};

#endif // GLSL_RAYTRACING_DEVICE_H
