#include "renderer/backend/vulkan/details/resources/uniformBuffer.h"


#include "core/debugger.h"
#include "renderer/backend/vulkan/details/descriptor.h"
#include "renderer/backend/vulkan/details/resources/buffer.h"

void createVulkanUniformBuffers(
    VkPhysicalDevice gpu, 
    VkDevice device,
    DescriptorSetConfig* descriptorConfig,
    PipelineResources* resources
) {
    u8 uboCount = 0;
    for(u8 b = 0; b < descriptorConfig->bindingsCount; b++) {
        if(descriptorConfig->bindings[b].type == DESCRIPTOR_TYPE_UNIFORM_BUFFER || 
            descriptorConfig->bindings[b].type == DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC) {
             uboCount++;
         } 
    }
    for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        resources[i].uniformBufferCount = uboCount;
        resources[i].uniformBuffers = malloc(sizeof(UniformBuffer) * uboCount);
        if(!resources[i].uniformBuffers){
            LOG_FATAL("Failed to allocate uniform buffers!");
            return;
        }
    }

    u8 uboIndex = 0;
    for(u8 b = 0; b < descriptorConfig->bindingsCount; b++) {
        const BindingConfig* binding = &descriptorConfig->bindings[b];
        
        // Skip non-uniform buffer bindings
        if(binding->type != DESCRIPTOR_TYPE_UNIFORM_BUFFER && 
           binding->type != DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC) {
            continue;
        }

        // Use actual binding data from current descriptor
        u32 elementSize = binding->size;
        bool isDynamic = get_vulkan_descriptor_is_dynamic(binding->type);
        u32 elementCount = isDynamic ? binding->dynamicCount : 1;

        // Calculate alignment requirements
        VkDeviceSize bufferSize = elementSize;
        VkDeviceSize alignedSize = elementSize;
        
        if(isDynamic) {
            VkPhysicalDeviceProperties deviceProperties;
            vkGetPhysicalDeviceProperties(gpu, &deviceProperties);
            VkDeviceSize minAlignment = deviceProperties.limits.minUniformBufferOffsetAlignment;
            
            if(minAlignment > 0) {
                alignedSize = (elementSize + minAlignment - 1) & ~(minAlignment - 1);
            }
            
            // Calculate total buffer size
            bufferSize = alignedSize * elementCount;
            
            // Store alignment information
            for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                resources[i].uniformBuffers[uboIndex].alignment = alignedSize;
            }
        }else{
            for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                resources[i].uniformBuffers[uboIndex].size = elementSize;
            }
        }

        // Create buffers for each frame
        for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            createVulkanBuffer(
                gpu,
                device,
                bufferSize,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                &resources[i].uniformBuffers[uboIndex].buffer,
                &resources[i].uniformBuffers[uboIndex].memory
            );

            // Map entire buffer range
            vkMapMemory(device, 
                resources[i].uniformBuffers[uboIndex].memory,
                0, 
                bufferSize, 
                0, 
                &resources[i].uniformBuffers[uboIndex].mapped
            );
        }

        uboIndex++;  // Move to next uniform buffer slot
    }
}