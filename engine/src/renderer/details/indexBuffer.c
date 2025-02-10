#include "renderer/details/indexBuffer.h"
#include "renderer/details/buffer.h"

void createIndexBuffer(VkPhysicalDevice gpu, VkDevice device, VkQueue queue, VkCommandPool cmdPool, u32 indicesCount, u16* indices, VkBuffer* outBuff, VkDeviceMemory* outMem){
    u32 buffSize = sizeof(u32) * indicesCount;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    createBuffer(gpu, device, buffSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);

    //filling vertex data
    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, buffSize, 0, &data);
    memcpy(data,indices, buffSize);
    vkUnmapMemory(device, stagingBufferMemory);

    createBuffer(gpu, device, buffSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, outBuff, outMem);

    copyBuffer(device, stagingBuffer, *outBuff, buffSize, cmdPool, queue);

    destroyBuffer(device, stagingBuffer, stagingBufferMemory);
}

void destroyIndexBuffer(VkDevice device, VkBuffer vertexBuffer, VkDeviceMemory mem){
    destroyBuffer(device, vertexBuffer, mem);
}
