#include "renderer/details/commandBuffer.h"

#include "core/debugger.h"

#include <collections/DynamicArray.h>

void createCommandBuffer(VkDevice device, VkCommandPool pool, VkCommandBuffer* out){
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = pool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = MAX_FRAMES_IN_FLIGHT;

    vkAllocateCommandBuffers(device, &allocInfo, out);
}

VkCommandBuffer beginSingleTimeCommands(VkDevice device, VkCommandPool cmdPool){
    VkCommandBufferAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandPool = cmdPool,
        .commandBufferCount = 1
    };

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };
    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    return commandBuffer;
}

void endSingleTimeCommands(VkDevice device, VkCommandPool cmdPool, VkQueue queue,  VkCommandBuffer* cmdBuffer){
    vkEndCommandBuffer(*cmdBuffer);

    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = cmdBuffer
    };

    vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);

    vkFreeCommandBuffers(device, cmdPool, 1, cmdBuffer);
}

void recordCommandBuffer(VkCommandBuffer commandBuffer, VkPipeline pipeline, VkPipelineLayout pipelineLayout, VkRenderPass renderpass, VkFramebuffer* swapChainFramebuffers, VkExtent2D extent, uint32_t imageIndex, VkDescriptorSet* descriptorSet, MeshRenderer_Component* meshRenderers) {
    VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = 0, // Optional
        .pInheritanceInfo = 0 // Optional
    };
    VkResult res = vkBeginCommandBuffer(commandBuffer, &beginInfo);
    if (res != VK_SUCCESS) {
        LOG_FATAL("failed to begin recording command buffer!");
        return;
    }

    const u8 clearValuesCount = 2;
    VkClearValue clearValues[clearValuesCount] = {
        //0
        {
            .color = {0.0f, 0.0f, 0.0f, 1.0f}
        },
        //1
        {
            .depthStencil = {1.f, 0}
        }
    };

    VkRenderPassBeginInfo renderPassInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = renderpass,
        .framebuffer = swapChainFramebuffers[imageIndex],
        .renderArea = {
            .offset = {0, 0},
            .extent = extent
        },
        .clearValueCount = clearValuesCount,
        .pClearValues = clearValues
    };

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    VkViewport viewport = {
        .x = 0.0f,
        .y = 0.0f,
        .width = (float) extent.width,
        .height = (float) extent.height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor = {
        .offset = {0, 0},
        .extent = extent
    };
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, descriptorSet, 0, 0);
    
    for(u16 i=0; i< DynamicArray_Length(meshRenderers); i++){
        MeshRenderer_Component* m = &meshRenderers[i];
        VkBuffer vertexBuffers[] = { m->renderContext.vertexBuffer};
        VkDeviceSize offsets[] = {0};
        vkCmdPushConstants(
            commandBuffer,
            pipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT,
            0,
            sizeof(Mat4),
            &m->mat4
        );
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(commandBuffer, m->renderContext.indexBuffer,0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(commandBuffer, m->indicesCount, 1, 0, 0, 0);
    }
    vkCmdEndRenderPass(commandBuffer);

    res = vkEndCommandBuffer(commandBuffer);
    if (res != VK_SUCCESS) {
        LOG_FATAL("failed to end recording command buffer!");
        return;
    }
}

void destroyCommandBuffer(VkDevice device, VkCommandPool pool, VkCommandBuffer* buff){
    vkFreeCommandBuffers(device, pool, 1, buff);
}