#include "renderer/backend/vulkan/details/descriptor.h"

#include "core/debugger.h"
#include "renderer/backend/vulkan/details/utils.h"

#define MAX_POOL_SIZES 32  // Adjust based on your needs


VkDescriptorType get_vulkan_descriptor_type(DescriptorType type) {
    switch (type) {
        case DESCRIPTOR_TYPE_UNIFORM_BUFFER:
            return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        case DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
            return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
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
bool get_vulkan_descriptor_is_dynamic(DescriptorType type) {
    switch (type) {
        case DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
        case DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
            return true;
        default:
            return false;
    }
}

///////////////////////////////////
///                             ///
///    DESCRIPTOR SET LAYOUT    ///
///                             ///
///////////////////////////////////


void createVulkanDescriptorSetLayout(VkDevice device, const DescriptorSetConfig* config, VkDescriptorSetLayout* out){
    VkDescriptorSetLayoutBinding* bindings = malloc(config->bindingsCount * sizeof(VkDescriptorSetLayoutBinding));
    if(!bindings){
        LOG_FATAL("failed to create descriptor set layout!");
        return;
    }

    for(u8 i=0; i<config->bindingsCount; i++){
        bindings[i] = (VkDescriptorSetLayoutBinding){
            .binding = i,
            .descriptorType = get_vulkan_descriptor_type(config->bindings[i].type),
            .descriptorCount = config->bindings[i].count <= 0 ? 1 : config->bindings[i].count,
            .stageFlags = get_vulkan_shader_stage_flags(config->bindings[i].stages),
            .pImmutableSamplers = 0
        };
    }


    VkDescriptorSetLayoutCreateInfo layoutInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = config->bindingsCount,
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

/////////////////////////////
///                       ///
///    DESCRIPTOR POOL    ///
///                       ///
/////////////////////////////

void createVulkanDescriptorPool(
    VkDevice device,
    const DescriptorSetConfig* configs,
    u8 setsCount,
    VkDescriptorPool* out
) {
    VkDescriptorPoolSize poolSizes[MAX_POOL_SIZES] = {0};
    u32 uniquePoolSizes = 0;

    // Calculate total number of descriptors needed across all sets
    for (u8 setIdx = 0; setIdx < setsCount; setIdx++) {
        const DescriptorSetConfig* setConfig = &configs[setIdx];
        for (u8 bindingIdx = 0; bindingIdx < setConfig->bindingsCount; bindingIdx++) {
            const BindingConfig* binding = &setConfig->bindings[bindingIdx];
            VkDescriptorType vkType = get_vulkan_descriptor_type(binding->type);
            u32 bindingCount = (binding->count <= 0) ? 1 : binding->count;
            // Calculate for all frames and all sets
            u32 totalDescriptors = bindingCount * MAX_FRAMES_IN_FLIGHT;

            bool found = false;
            for (u32 poolIdx = 0; poolIdx < uniquePoolSizes; poolIdx++) {
                if (poolSizes[poolIdx].type == vkType) {
                    poolSizes[poolIdx].descriptorCount += totalDescriptors;
                    found = true;
                    break;
                }
            }
            if (!found) {
                if (uniquePoolSizes >= MAX_POOL_SIZES) {
                    LOG_FATAL("Exceeded maximum pool size types!");
                    return;
                }
                poolSizes[uniquePoolSizes] = (VkDescriptorPoolSize){
                    .type = vkType,
                    .descriptorCount = totalDescriptors
                };
                uniquePoolSizes++;
            }
        }
    }

    // Add some padding to prevent running out of descriptors
    for (u32 i = 0; i < uniquePoolSizes; i++) {
        poolSizes[i].descriptorCount += poolSizes[i].descriptorCount / 2;  // Add 50% padding
    }

    LOG_DEBUG("Descriptor Pool Sizes:");
    for (u32 i = 0; i < uniquePoolSizes; i++) {
        LOG_DEBUG("Type: %d, Count: %u", poolSizes[i].type, poolSizes[i].descriptorCount);
    }
    
    u32 totalSets = MAX_FRAMES_IN_FLIGHT * setsCount;
    LOG_DEBUG("Total Sets to allocate: %u", totalSets);

    VkDescriptorPoolCreateInfo poolInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,  // Add this flag to allow freeing
        .maxSets = totalSets,
        .poolSizeCount = uniquePoolSizes,
        .pPoolSizes = poolSizes
    };

    VkResult res = vkCreateDescriptorPool(device, &poolInfo, NULL, out);
    if (res != VK_SUCCESS) {
        LOG_FATAL("Failed to create descriptor pool: %d", res);
    }
}

void destroyVulkanDescriptorPool(VkDevice device, VkDescriptorPool pool){
    vkDestroyDescriptorPool(device, pool, 0);
}

////////////////////////////
///                      ///
///    DESCRIPTOR SET    ///
///                      ///
////////////////////////////

void createVulkanDescriptorSets(
    VkDevice device, 
    const PipelineConfig* config, 
    Pipeline* out
) {

    for (size_t frameIdx = 0; frameIdx < MAX_FRAMES_IN_FLIGHT; frameIdx++) {
        out->frameResources[frameIdx].descriptorSets = malloc(config->setsCount * sizeof(VkDescriptorSet));
        if (!out->frameResources[frameIdx].descriptorSets) {
            LOG_FATAL("Failed to allocate memory for descriptor sets array!");
            return;
        }
    }

    for (u8 setIdx = 0; setIdx < config->setsCount; setIdx++) {
        const DescriptorSetConfig* setConfig = &config->sets[setIdx];

        for (size_t frameIdx = 0; frameIdx < MAX_FRAMES_IN_FLIGHT; frameIdx++) {
            VkDescriptorSetLayout layout = out->descriptorSetsLayout[setIdx];

            VkDescriptorSetAllocateInfo allocInfo = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                .descriptorPool = out->descriptorPool,
                .descriptorSetCount = 1,  // Allocate one at a time
                .pSetLayouts = &layout
            };

            VkResult res = vkAllocateDescriptorSets(device, &allocInfo, 
                &out->frameResources[frameIdx].descriptorSets[setIdx]);
            if (res != VK_SUCCESS) {
                LOG_FATAL("Failed to allocate descriptor set! Error: %d", res);
                return;
            }
        }

        // Update Descriptor Sets for each frame
        for (size_t frameIdx = 0; frameIdx < MAX_FRAMES_IN_FLIGHT; frameIdx++) {
            PipelineResources* resources = &out->frameResources[frameIdx];

            VkWriteDescriptorSet descriptorWrites[setConfig->bindingsCount];
            VkDescriptorBufferInfo bufferInfos[setConfig->bindingsCount];
            VkDescriptorImageInfo imageInfos[setConfig->bindingsCount];

            u8 uniformIndex = 0;
            u8 storageIndex = 0;
            u8 textureIndex = 0;

            for (u8 bindingIdx = 0; bindingIdx < setConfig->bindingsCount; bindingIdx++) {
                const BindingConfig* binding = &setConfig->bindings[bindingIdx];
                
                VkDescriptorType bindingType = get_vulkan_descriptor_type(binding->type);
                bool isDynamic = get_vulkan_descriptor_is_dynamic(binding->type);

                descriptorWrites[bindingIdx] = (VkWriteDescriptorSet){
                    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    .dstSet = resources->descriptorSets[setIdx],
                    .dstBinding = bindingIdx,
                    .dstArrayElement = 0,
                    .descriptorType = bindingType,
                    .descriptorCount = binding->count
                };

                // Handle buffer types
                if (bindingType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
                    bindingType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC) 
                {
                    // assert(uniformIndex < MAX_UNIFORM_BUFFERS && "Exceeded uniform buffer count");
                    bufferInfos[bindingIdx] = (VkDescriptorBufferInfo){
                        .buffer = resources->uniformBuffers[uniformIndex].buffer,
                        .offset = 0,
                        .range = isDynamic ? 
                            resources->uniformBuffers[uniformIndex].alignment :
                            resources->uniformBuffers[uniformIndex].size
                    };
                    descriptorWrites[bindingIdx].pBufferInfo = &bufferInfos[bindingIdx];
                    uniformIndex++;
                }
                else if (bindingType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER) 
                {
                    // assert(storageIndex < MAX_STORAGE_BUFFERS && "Exceeded storage buffer count");
                    bufferInfos[bindingIdx] = (VkDescriptorBufferInfo){
                        .buffer = resources->storageBuffers[storageIndex].buffer,
                        .offset = 0,
                        .range = resources->storageBuffers[storageIndex].size
                    };
                    descriptorWrites[bindingIdx].pBufferInfo = &bufferInfos[bindingIdx];
                    storageIndex++;
                }
                else if (bindingType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) 
                {
                    // assert(textureIndex < MAX_TEXTURES && "Exceeded texture count");
                    imageInfos[bindingIdx] = (VkDescriptorImageInfo){
                        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                        .imageView = out->textures[textureIndex].view,
                        .sampler = out->textures[textureIndex].sampler
                    };
                    descriptorWrites[bindingIdx].pImageInfo = &imageInfos[bindingIdx];
                    textureIndex++;
                }
                // Add other descriptor types as needed
            }

            vkUpdateDescriptorSets(device, setConfig->bindingsCount, descriptorWrites, 0, NULL);
        }
    }
}