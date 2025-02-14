#ifndef RENDERER_BACKEND_VULKAN_DETAILS_SHADER_H
#define RENDERER_BACKEND_VULKAN_DETAILS_SHADER_H

#include "renderer/backend/vulkan/vulkan_types.h"
#include "core/files.h"

VkShaderModule createShaderModule(VkDevice device, File* code);
void destroyShaderModule(VkDevice device, VkShaderModule shaderModule);
#endif //RENDERER_BACKEND_VULKAN_DETAILS_SHADER_H