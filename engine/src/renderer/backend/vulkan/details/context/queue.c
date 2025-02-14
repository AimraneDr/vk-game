#include "renderer/backend/vulkan/details/context/queue.h"

#include <stdlib.h>

QueueFamilyIndices findQueueFamilies(const VkPhysicalDevice device, const VkSurfaceKHR surface){
    QueueFamilyIndices indices = { -1, -1, -1, .familiesCount = 0};
    i32 uniques[32];
    u32 queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, 0);

    VkQueueFamilyProperties* QueueFamProps = (VkQueueFamilyProperties*)malloc(sizeof(VkQueueFamilyProperties) * queueFamilyCount);
    for(u32 i=0; i < queueFamilyCount; i++){
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, &QueueFamProps[i]);
    }

    for(u32 i=0; i < queueFamilyCount; i++){
        //if families are selected just break from the loop
        if(indices.graphicsFamily > -1 && indices.computeFamily > -1) break;
        bool selected = false;

        //select graphics queue
        if(QueueFamProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT){
            indices.graphicsFamily = i;
            if(!selected){
                uniques[indices.familiesCount] = i;
                indices.familiesCount++;
            }
            selected = true;
        }

        //select compute queue
        if(QueueFamProps[i].queueFlags & VK_QUEUE_COMPUTE_BIT){
            indices.computeFamily = i;
            if(!selected) {
                uniques[indices.familiesCount] = i;
                indices.familiesCount++;
            }
            selected = true;
        }

        VkBool32 presentSupport = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
        if(presentSupport){
            indices.presentFamily = i;
            if(!selected){
                uniques[indices.familiesCount] = i;
                indices.familiesCount++;
            }
            selected = true;
        }
    }

    indices.uniqueFamilies = (i32*)malloc(sizeof(i32) * indices.familiesCount);
    for(u32 i=0; i< indices.familiesCount; i++){
        indices.uniqueFamilies[i] = uniques[i];
    }

    free(QueueFamProps);

    return indices;
}

void freeQueeFamiliesInfoStruct(QueueFamilyIndices q){
    free(q.uniqueFamilies);
}