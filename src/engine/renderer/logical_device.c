#include "engine/renderer/logical_device.h"

#include "engine/renderer/details/queue.h"
#include "engine/renderer/details/details.h"

#include <stdlib.h>

Result createLogicalDevice(const VkPhysicalDevice device, const VkSurfaceKHR surface, VkDevice* out_device, VkQueue* out_graphics_queue, VkQueue* out_present_queue){
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

    VkPhysicalDeviceFeatures enabledFeatures = {};

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

    if(result == VK_SUCCESS){
        vkGetDeviceQueue(*out_device, indices.graphicsFamily, 0, out_graphics_queue);
        vkGetDeviceQueue(*out_device, indices.presentFamily, 0, out_present_queue);
        return RESULT_CODE_SUCCESS;
    }
        
    return RESULT_CODE_FAILED_DEVICE_CREATION;
}

void destroyLogicalDevice(VkDevice lDevice){
    vkDestroyDevice(lDevice, 0);
}
