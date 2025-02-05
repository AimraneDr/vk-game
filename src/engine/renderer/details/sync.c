#include "engine/renderer/details/sync.h"

#include "engine/core/debugger.h"

void createSyncObjects(VkDevice device, VkSemaphore* imageAvailableSemaphore, VkSemaphore* renderFinishedSemaphore, VkFence* inFlightFence){
    VkSemaphoreCreateInfo semaphoreInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
    };

    VkFenceCreateInfo fenceInfo = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };

    inFlightFence[0] = 0;
    inFlightFence[1] = 0;

    for(u8 i=0; i<MAX_FRAMES_IN_FLIGHT; i++){
        if (vkCreateSemaphore(device, &semaphoreInfo, 0, &imageAvailableSemaphore[i]) != VK_SUCCESS ||
            vkCreateSemaphore(device, &semaphoreInfo, 0, &renderFinishedSemaphore[i]) != VK_SUCCESS ||
            vkCreateFence(device, &fenceInfo, 0, &inFlightFence[i]) != VK_SUCCESS) {
            LOG_FATAL("failed to create synchronization objects for frame %d !", i);
            return;
        }
    }
}

void destroySyncObjects(VkDevice device, VkSemaphore* imageAvailableSemaphore, VkSemaphore* renderFinishedSemaphore, VkFence* inFlightFence){
    for(u8 i=0; i<MAX_FRAMES_IN_FLIGHT; i++){
        vkDestroySemaphore(device, imageAvailableSemaphore[i], 0);
        vkDestroySemaphore(device, renderFinishedSemaphore[i], 0);
        vkDestroyFence(device, inFlightFence[i], 0);
    }
}