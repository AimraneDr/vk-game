#ifndef UNIFORM_BUFFER_H
#define UNIFORM_BUFFER_H

#include "renderer/render_types.h"
#include "components/camera.h"
#include "components/UI/ui_types.h"

#include <math/mathTypes.h>

void createUniformBuffer(VkPhysicalDevice gpu, VkDevice device, VkBuffer* uniformBuffers, VkDeviceMemory* buffersMems, void** mappedBuffs);
void UI_createUniformBuffer(VkPhysicalDevice gpu, VkDevice device, VkBuffer* uniformBuffers, VkDeviceMemory* buffersMems, void** mappedBuffs);
void destroyUniformBuffer(VkDevice device, VkBuffer uniformBuffers[], VkDeviceMemory buffersMems[]);
void updateUniformBuffer(uint32_t currentImage, Vec2 frameSize, void** uniformBuffersMapped, f64 deltatime, Camera* camera);
void UI_updateUniformBuffer(uint32_t currentImage, Vec2 frameSize, void** uniformBuffersMapped, f64 deltatime, UI_Manager* uiManager);

#endif