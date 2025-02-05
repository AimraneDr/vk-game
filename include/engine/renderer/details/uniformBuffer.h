#ifndef UNIFORM_BUFFER_H
#define UNIFORM_BUFFER_H

#include "engine/renderer/render_types.h"
#include "math/mathTypes.h"

void createUniformBuffer(VkPhysicalDevice gpu, VkDevice device, VkBuffer* uniformBuffers, VkDeviceMemory* buffersMems, void** mappedBuffs);
void destroyUniformBuffer(VkDevice device, VkBuffer uniformBuffers[], VkDeviceMemory buffersMems[]);
void updateUniformBuffer(uint32_t currentImage, Vec2 frameSize, void** uniformBuffersMapped, f64 deltatime);

#endif