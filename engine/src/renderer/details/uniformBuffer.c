#include "renderer/details/uniformBuffer.h"

#include <math/vec3.h>
#include <math/vec2.h>
#include <math/vec4.h>
#include <math/mathConst.h>
#include <math/trigonometry.h>
#include <math/mat.h>
#include "renderer/details/buffer.h"
#include "core/debugger.h"



void createUniformBuffers(VkPhysicalDevice gpu, VkDevice device, VkDeviceSize bufferSize, VkBuffer* uniformBuffers, VkDeviceMemory* buffersMems, void** mappedBuffs){
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        createBuffer(
            gpu,
            device,
            bufferSize, 
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
            &(uniformBuffers[i]), 
            &(buffersMems[i])
        );

        vkMapMemory(device, buffersMems[i], 0, bufferSize, 0, &(mappedBuffs[i]));
    }
}

void createDynamicOffsetUniformBuffers(VkPhysicalDevice gpu, VkDevice device, u32 uboSize, VkBuffer* uniformBuffers, VkDeviceMemory* buffersMems, void** mappedBuffs, VkDeviceSize* outAlignedUboSize){
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(gpu, &deviceProperties);
    VkDeviceSize minUboAlignment = deviceProperties.limits.minUniformBufferOffsetAlignment;

    VkDeviceSize alignedUboSize = uboSize;
    if (minUboAlignment > 0) {
        alignedUboSize = (uboSize + minUboAlignment - 1) & ~(minUboAlignment - 1);
    }
    *outAlignedUboSize = alignedUboSize;
    VkDeviceSize bufferSize = alignedUboSize * 20;//TODO: make this dynamic

    if(alignedUboSize < uboSize){
        LOG_ERROR("uboSize is not aligned to the device's minUniformBufferOffsetAlignment");
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        createBuffer(
            gpu,
            device,
            bufferSize, 
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
            &(uniformBuffers[i]), 
            &(buffersMems[i])
        );

        vkMapMemory(device, buffersMems[i], 0, uboSize, 0, &(mappedBuffs[i]));
    }
}

void destroyUniformBuffers(VkDevice device, VkBuffer uniformBuffers[], VkDeviceMemory buffersMems[]){
     for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroyBuffer(device, uniformBuffers[i], 0);
        vkFreeMemory(device, buffersMems[i], 0);
    }
}