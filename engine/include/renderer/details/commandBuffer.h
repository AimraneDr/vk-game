#ifndef COMMAND_BUFFER_H
#define COMMAND_BUFFER_H

#include "renderer/render_types.h"

void createCommandBuffer(VkDevice device, VkCommandPool pool, VkCommandBuffer* out);
VkCommandBuffer beginSingleTimeCommands(VkDevice device, VkCommandPool cmdPool);
void endSingleTimeCommands(VkDevice device, VkCommandPool cmdPool, VkQueue queue,  VkCommandBuffer* cmdBuffer);
void recordCommandBuffer(VkCommandBuffer commandBuffer, VkPipeline pipeline, VkPipelineLayout pipelineLayout, VkRenderPass renderpass, VkFramebuffer* swapChainFramebuffers, VkExtent2D extent, uint32_t imageIndex, VkBuffer vertexBuffer, VkBuffer indexBuffer, u32 indicesCount, VkDescriptorSet* descriptorSet);
void destroyCommandBuffer(VkDevice device, VkCommandPool pool, VkCommandBuffer* buff);

#endif //COMMAND_BUFFER_H