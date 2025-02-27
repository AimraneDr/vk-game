#ifndef UNIFORM_BUFFER_H
#define UNIFORM_BUFFER_H

#include "renderer/render_types.h"
#include "components/camera.h"
#include "components/UI/ui_types.h"

#include <math/mathTypes.h>

void createUniformBuffers(VkPhysicalDevice gpu, VkDevice device, VkDeviceSize bufferSize, VkBuffer* uniformBuffers, VkDeviceMemory* buffersMems, void** mappedBuffs);
void createDynamicOffsetUniformBuffers(VkPhysicalDevice gpu, VkDevice device, u32 uboSize, VkBuffer* uniformBuffers, VkDeviceMemory* buffersMems, void** mappedBuffs, VkDeviceSize* outAlignedUboSize);

void destroyUniformBuffers(VkDevice device, VkBuffer uniformBuffers[], VkDeviceMemory buffersMems[]);

#endif