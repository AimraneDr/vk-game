#ifndef SHADER_H
#define SHADER_H

#include "engine/renderer/render_types.h"
#include "engine/core/files.h"

VkShaderModule createShaderModule(VkDevice device, File* code);
void destroyShaderModule(VkDevice device, VkShaderModule shaderModule);
#endif //SHADER_H