#include "renderer/details/logical_device.h"

#include "core/debugger.h"
#include "renderer/backend/vulkan/details/context/queue.h"
#include "renderer/backend/vulkan/details/utils.h"

#include <stdlib.h>

void createLogicalDevice(const VkPhysicalDevice device, const VkSurfaceKHR surface, VkDevice* out_device, VkQueue* out_graphics_queue, VkQueue* out_present_queue){
    QueueFamilyIndices indices = findQueueFamilies(device, surface);

    VkDeviceQueueCreateInfo* queueCreateInfos = (VkDeviceQueueCreateInfo*)malloc(sizeof(VkDeviceQueueCreateInfo) * indices.familiesCount);

    for(u32 i=0; i < indices.familiesCount; i++){
        f32 queuePriority = 1.0f;
        VkDeviceQueueCreateInfo queueCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = indices.uniqueFamilies[i],
            .queueCount = 1,
            .pQueuePriorities = &queuePriority
        };
        queueCreateInfos[i] = queueCreateInfo;
    }

    VkPhysicalDeviceFeatures enabledFeatures = {
        .samplerAnisotropy = VK_TRUE,
        .sampleRateShading = VK_TRUE,
    };

    VkDeviceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pQueueCreateInfos = queueCreateInfos,
        .queueCreateInfoCount = indices.familiesCount,
        .pEnabledFeatures = &enabledFeatures,
        .enabledExtensionCount = deviceExtensionsCount(),
        .ppEnabledExtensionNames = deviceExtensionsNames()
    };
    
    if(isValidationLayersEnabled()){
        createInfo.enabledLayerCount = validationLayersCount();
        createInfo.ppEnabledLayerNames = validationLayersNames();
    }else{
        createInfo.enabledLayerCount = 0;
    }

    VkResult result = vkCreateDevice(device, &createInfo, 0, out_device);
    free(queueCreateInfos);

    if(result != VK_SUCCESS){
        LOG_FATAL("failed to create Logical Device");
        return;
    }
    
    vkGetDeviceQueue(*out_device, indices.graphicsFamily, 0, out_graphics_queue);
    vkGetDeviceQueue(*out_device, indices.presentFamily, 0, out_present_queue);
    return;
}

void destroyLogicalDevice(VkDevice lDevice){
    vkDestroyDevice(lDevice, 0);
}
