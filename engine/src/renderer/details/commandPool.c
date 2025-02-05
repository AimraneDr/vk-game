#include "renderer/details/commandPool.h"

#include "renderer/details/queue.h"
#include "core/debugger.h"

void createCommandPool(VkPhysicalDevice gpu, VkDevice device, VkSurfaceKHR surface, VkCommandPool *out)
{
    QueueFamilyIndices indices = findQueueFamilies(gpu, surface);

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = indices.graphicsFamily;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    if (vkCreateCommandPool(device, &poolInfo, 0, out) != VK_SUCCESS)
    {
        LOG_FATAL("Failed to create command pool");
        return;
    }
    freeQueeFamiliesInfoStruct(indices);
}

void destroyCommandPool(VkDevice device, VkCommandPool commandPool)
{
    vkDestroyCommandPool(device, commandPool, 0);
}