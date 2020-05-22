#include <vulkan/vulkan.h>
#include <cstdio>

int main() {
    VkInstance instance = VK_NULL_HANDLE;
    VkInstanceCreateInfo instance_create_info = {};
    instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    if (vkCreateInstance(&instance_create_info, nullptr, &instance) == VK_SUCCESS) {
        printf("hello, vulkan\n");
        vkDestroyInstance(instance, nullptr);
    }
}