#include "renderer/backend/vulkan/details/descriptor.h"

#include "core/debugger.h"
#include "renderer/backend/vulkan/details/utils.h"

#define MAX_POOL_SIZES 32  // Adjust based on your needs


VkDescriptorType get_vulkan_descriptor_type(DescriptorType type) {
    switch (type) {
        case DESCRIPTOR_TYPE_UNIFORM_BUFFER:
            return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        case DESCRIPTOR_TYPE_STORAGE_BUFFER:
            return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        case DESCRIPTOR_TYPE_COMBINED_SAMPLER:
            return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        case DESCRIPTOR_TYPE_SAMPLER:
            return VK_DESCRIPTOR_TYPE_SAMPLER;
        case DESCRIPTOR_TYPE_SAMPLED_IMAGE:
            return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        default:
            LOG_FATAL("Unknown descriptor type!");
            return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    }
}



void createVulkanDescriptorSetLayout(VkDevice device, const DescriptorSetConfig config, VkDescriptorSetLayout* out){
    VkDescriptorSetLayoutBinding* bindings = malloc(config.bindingsCount * sizeof(VkDescriptorSetLayoutBinding));
    if(!bindings){
        LOG_FATAL("failed to create descriptor set layout!");
        return;
    }

    for(u8 i=0; i<config.bindingsCount; i++){
        VkDescriptorSetLayoutBinding uboLayoutBinding = {
            .binding = i,
            .descriptorType = get_vulkan_descriptor_type(config.bindings[i].type),
            .descriptorCount = 1,
            .stageFlags = get_vulkan_shader_stage_flags(config.bindings[i].stages),
            .pImmutableSamplers = 0
        };
    }


    VkDescriptorSetLayoutCreateInfo layoutInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = config.bindingsCount,
        .pBindings = bindings
    };

    VkResult res = vkCreateDescriptorSetLayout(device, &layoutInfo, 0, out);
    free(bindings);
    if(res != VK_SUCCESS){
        LOG_FATAL("failed to create descriptor set layout!");
    }
}

void destroyVulkanDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout layout){
    vkDestroyDescriptorSetLayout(device, layout, 0);
}


void createVulkanDescriptorPool(
    VkDevice device,
    const DescriptorSetConfig* configs,
    u8 setsCount,
    VkDescriptorPool* out
) {
    // First, aggregate all descriptor types across all sets
    VkDescriptorPoolSize poolSizes[MAX_POOL_SIZES] = {0};
    u32 uniquePoolSizes = 0;

    // For each descriptor set
    for (u8 setIdx = 0; setIdx < setsCount; setIdx++) {
        const DescriptorSetConfig* setConfig = &configs[setIdx];
        
        // For each binding in this set
        for (u8 bindingIdx = 0; bindingIdx < setConfig->bindingsCount; bindingIdx++) {
            const BindingConfig* binding = &setConfig->bindings[bindingIdx];
            VkDescriptorType vkType = get_vulkan_descriptor_type(binding->type);  // Assuming you have this conversion function
            
            // Look for existing pool size entry with this type
            bool found = false;
            for (u32 poolIdx = 0; poolIdx < uniquePoolSizes; poolIdx++) {
                if (poolSizes[poolIdx].type == vkType) {
                    // Add one more descriptor of this type per maxSets
                    poolSizes[poolIdx].descriptorCount += MAX_FRAMES_IN_FLIGHT;
                    found = true;
                    break;
                }
            }
            
            // If not found, create new pool size entry
            if (!found) {
                if (uniquePoolSizes >= MAX_POOL_SIZES) {
                    LOG_FATAL("Exceeded maximum number of unique descriptor types!");
                    return;
                }
                
                poolSizes[uniquePoolSizes].type = vkType;
                poolSizes[uniquePoolSizes].descriptorCount = MAX_FRAMES_IN_FLIGHT;  // One descriptor of this type per maxSets
                uniquePoolSizes++;
            }
        }
    }

    // Create the descriptor pool
    VkDescriptorPoolCreateInfo poolInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .maxSets = MAX_FRAMES_IN_FLIGHT * setsCount,  // Total number of sets we might allocate
        .poolSizeCount = uniquePoolSizes,
        .pPoolSizes = poolSizes
    };

    VkResult res = vkCreateDescriptorPool(device, &poolInfo, NULL, out);
    if (res != VK_SUCCESS) {
        LOG_FATAL("Failed to create descriptor pool!");
    }
}

void destroyVulkanDescriptorPool(VkDevice device, VkDescriptorPool pool){
    vkDestroyDescriptorPool(device, pool, 0);
}