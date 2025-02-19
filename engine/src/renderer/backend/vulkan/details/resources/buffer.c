#include "renderer/backend/vulkan/details/resources/buffer.h"

#include "core/debugger.h"
#include "renderer/backend/vulkan/details/context/physicalDevice.h"
#include "renderer/backend/vulkan/vulkan_command.h"


void createVulkanBuffer(
    VkPhysicalDevice gpu, 
    VkDevice device, 
    VkDeviceSize size,  // Changed from u32 to VkDeviceSize
    VkBufferUsageFlags usageFlags, 
    VkMemoryPropertyFlags memProperties, 
    VkBuffer* outBuff, 
    VkDeviceMemory* buffMem)
{
    // Initialize output handles
    *outBuff = VK_NULL_HANDLE;
    *buffMem = VK_NULL_HANDLE;

    VkBufferCreateInfo bufferInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = usageFlags,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };

    VkResult res = vkCreateBuffer(device, &bufferInfo, NULL, outBuff);
    if(res != VK_SUCCESS) {
        LOG_FATAL("Failed to create buffer: %d", res);
        return;
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, *outBuff, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = findMemoryType(gpu, 
            memRequirements.memoryTypeBits, 
            memProperties)
    };

    // Allocate memory
    res = vkAllocateMemory(device, &allocInfo, NULL, buffMem);
    if(res != VK_SUCCESS) {
        LOG_FATAL("Failed to allocate buffer memory: %d", res);
        vkDestroyBuffer(device, *outBuff, NULL);
        return;
    }

    // Bind memory
    res = vkBindBufferMemory(device, *outBuff, *buffMem, 0);
    if(res != VK_SUCCESS) {
        LOG_FATAL("Failed to bind buffer memory: %d", res);
        vkDestroyBuffer(device, *outBuff, NULL);
        vkFreeMemory(device, *buffMem, NULL);
        return;
    }
}

void destroyVulkanBuffer(VkDevice device, VkBuffer buff, VkDeviceMemory mem){
    vkDestroyBuffer(device, buff, 0);
    vkFreeMemory(device, mem, 0);
}

void copyBuffer(VkDevice device,VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkCommandPool cmdPool, VkQueue queue){
    
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(device, cmdPool);

    VkBufferCopy copyRegion = {
        .srcOffset = 0, // Optional
        .dstOffset = 0, // Optional
        .size = size
    };
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    endSingleTimeCommands(device, cmdPool, queue, &commandBuffer);
}