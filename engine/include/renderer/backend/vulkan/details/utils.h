#ifndef RENDERER_BACKEND_VULKAN_UTILS_H
#define RENDERER_BACKEND_VULKAN_UTILS_H

#include "data_types.h"
#include "renderer/render_types.h"

bool isValidationLayersEnabled();
char* const* validationLayersNames();
const u32 validationLayersCount();

char* const* deviceExtensionsNames();
const u32 deviceExtensionsCount();
VkShaderStageFlags  get_vulkan_shader_stage_flags(ShaderStage bits)

#endif //RENDERER_BACKEND_VULKAN_UTILS_H