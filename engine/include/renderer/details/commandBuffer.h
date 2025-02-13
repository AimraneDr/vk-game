#ifndef COMMAND_BUFFER_H
#define COMMAND_BUFFER_H

#include "renderer/render_types.h"
#include "assets/asset_types.h"
#include "components/UI/ui_types.h"
#include "components/meshRenderer.h"


void createCommandBuffer(VkDevice device, VkCommandPool pool, VkCommandBuffer* out);
VkCommandBuffer beginSingleTimeCommands(VkDevice device, VkCommandPool cmdPool);
void endSingleTimeCommands(VkDevice device, VkCommandPool cmdPool, VkQueue queue,  VkCommandBuffer* cmdBuffer);

void recordCommandBuffer(
    VkCommandBuffer commandBuffer, 
    VkPipeline worldPipeline, 
    VkPipelineLayout worldPipelineLayout, 
    VkPipeline uiPipeline, 
    VkPipelineLayout uiPipelineLayout, 
    VkRenderPass renderpass, 
    VkFramebuffer* swapChainFramebuffers,
    VkExtent2D extent, uint32_t imageIndex, 
    VkDescriptorSet* worldDescriptorSet, 
    VkDescriptorSet* uiDescriptorSet, 
    MeshRenderer* meshRenderers, // TODO: move to didcated system for the compoennt
    UI_Manager* uiManager   //move to didicated system
);

void destroyCommandBuffer(VkDevice device, VkCommandPool pool, VkCommandBuffer* buff);

#endif //COMMAND_BUFFER_H