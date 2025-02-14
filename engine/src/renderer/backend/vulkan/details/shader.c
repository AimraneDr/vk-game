#include "renderer/details/shader.h"

#include "core/files.h"
#include "core/debugger.h"

VkShaderModule createShaderModule(VkDevice device, File* code){
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code->size;
    createInfo.pCode = (u32*)code->content;
    VkShaderModule shaderModule;
    VkResult res = vkCreateShaderModule(device, &createInfo, 0, &shaderModule);
    if(res != VK_SUCCESS){
        LOG_FATAL("Failed to create shader module");
        return VK_NULL_HANDLE;
    }
    return shaderModule;
}

void destroyShaderModule(VkDevice device, VkShaderModule shaderModule){
    vkDestroyShaderModule(device, shaderModule, 0);
}