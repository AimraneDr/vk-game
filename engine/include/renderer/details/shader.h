#ifndef SHADER_H
#define SHADER_H

#include "renderer/render_types.h"
#include "core/files.h"

VkShaderModule createShaderModule(VkDevice device, File* code);
void destroyShaderModule(VkDevice device, VkShaderModule shaderModule);
#endif //SHADER_H