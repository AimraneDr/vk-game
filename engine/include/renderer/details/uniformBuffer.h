#ifndef UNIFORM_BUFFER_H
#define UNIFORM_BUFFER_H

#include "renderer/render_types.h"
#include "components/camera.h"
#include "components/UI/ui_types.h"

#include <math/mathTypes.h>

void PBR_createUniformBuffers(VkPhysicalDevice gpu, VkDevice device, VkBuffer* uniformBuffers, VkDeviceMemory* buffersMems, void** mappedBuffs);

void UI_createUniformBuffers(VkPhysicalDevice gpu, VkDevice device, u32 uboSize, VkBuffer* uniformBuffers, VkDeviceMemory* buffersMems, void** mappedBuffs);
void UI_createDynamicOffsetUniformBuffers(VkPhysicalDevice gpu, VkDevice device, u32 uboSize, VkBuffer* uniformBuffers, VkDeviceMemory* buffersMems, void** mappedBuffs, VkDeviceSize* outAlignedUboSize);

void destroyUniformBuffers(VkDevice device, VkBuffer uniformBuffers[], VkDeviceMemory buffersMems[]);

void PBR_updateGlobalUniformBuffer(uint32_t currentImage, Vec2 frameSize, void** uniformBuffersMapped, f64 deltatime, Camera* camera);

void UI_updateGlobalUniformBuffer(uint32_t currentImage, Vec2 frameSize, void** uniformBuffersMapped, f64 deltatime, UI_Manager* uiManager);
void UI_updateElementUniformBuffer(void* uniformBufferMapped, f64 deltatime, UI_Element* uiElement, u32 alignIndex, VkDeviceSize alignedUboSize);


#endif