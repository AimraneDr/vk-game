#include "engine/renderer/details/buffer.h"

#include "engine/core/debugger.h"
#include "engine/renderer/details/physicalDevice.h"
#include "engine/renderer/details/commandBuffer.h"


void createBuffer(VkPhysicalDevice gpu, VkDevice device, u32 size, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memProperties, VkBuffer* outBuff, VkDeviceMemory* buffMem){
    VkBufferCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = usageFlags,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };

    VkResult res = vkCreateBuffer(device, &info, 0, outBuff);
    if(res != VK_SUCCESS){
        LOG_FATAL("Failed to create buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, *outBuff, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(
        gpu,
        memRequirements.memoryTypeBits, 
        memProperties);

    res = vkAllocateMemory(device, &allocInfo, 0, buffMem);
    if(res != VK_SUCCESS){
        LOG_FATAL("failed to allocate vertex buffer memory!");
    }
    
    vkBindBufferMemory(device, *outBuff, *buffMem, 0);
}
void destroyBuffer(VkDevice device, VkBuffer buff, VkDeviceMemory mem){
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