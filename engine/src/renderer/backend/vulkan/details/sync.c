#include "renderer/backend/vulkan/details/sync.h"

#include "core/debugger.h"

void createVulkanSyncObjects(const VkDevice device, SyncObj* syncObjs){
    VkSemaphoreCreateInfo semaphoreInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
    };

    VkFenceCreateInfo fenceInfo = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };

    
    for(u8 i=0; i<MAX_FRAMES_IN_FLIGHT; i++){
        syncObjs[i].inFlightFence = 0;
        if (vkCreateSemaphore(device, &semaphoreInfo, 0, &syncObjs[i].imageAvailableSemaphore) != VK_SUCCESS ||
            vkCreateSemaphore(device, &semaphoreInfo, 0, &syncObjs[i].renderFinishedSemaphore) != VK_SUCCESS ||
            vkCreateFence(device, &fenceInfo, 0, &syncObjs[i].inFlightFence) != VK_SUCCESS) {
            LOG_FATAL("failed to create synchronization objects for frame %d !", i);
            return;
        }
    }
}

void destroyVulkanSyncObjects(VkDevice device, SyncObj* syncObjs){
    for(u8 i=0; i<MAX_FRAMES_IN_FLIGHT; i++){
        vkDestroySemaphore(device, syncObjs[i].imageAvailableSemaphore, 0);
        vkDestroySemaphore(device, syncObjs[i].renderFinishedSemaphore, 0);
        vkDestroyFence(device, syncObjs[i].inFlightFence, 0);
    }
}