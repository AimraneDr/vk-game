#include "renderer/details/commandBuffer.h"

#include "renderer/details/uniformBuffer.h"
#include "core/debugger.h"
#include "components/UI/uiComponents.h"
#include <collections/DynamicArray.h>
#include <math/mat.h>

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

void recursiveUIElementsDraw(VkCommandBuffer commandBuffer, void* uniformBufferMapped, VkDeviceSize alignedUboSize, f64 deltatime, VkPipelineLayout layout, VkDescriptorSet set,UI_Element* root, u32 elementCounter) {
    for(u32 i=0; i<ui_elementChildrenCount(root); i++){
        VkBuffer ui_vertexBuffers[] = { root->children[i].renderer.vertexBuffer};
        VkDeviceSize offsets[] = {0};
        
        transform2D_update(&root->children[i].transform);

        UI_PushConstant pc = {
            .model = mat3_to_mat4(root->children[i].transform.mat)
        };
        vkCmdPushConstants(
            commandBuffer, 
            layout, 
            VK_SHADER_STAGE_VERTEX_BIT,0,
            sizeof(UI_PushConstant),
            &pc);

        
        u32 dynamicOffset = elementCounter * alignedUboSize;
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1, set, 1, &dynamicOffset);

        UI_updateElementUniformBuffer(uniformBufferMapped, deltatime, &root->children[i],elementCounter++, alignedUboSize);

        vkCmdBindVertexBuffers(commandBuffer, 0, 1, ui_vertexBuffers, offsets);
        vkCmdBindIndexBuffer(commandBuffer, root->children[i].renderer.indexBuffer,0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(commandBuffer, root->children[i].renderer.indicesCount, 1, 0, 0, 0);
        recursiveUIElementsDraw(commandBuffer, uniformBufferMapped, alignedUboSize, deltatime, layout, set, &root->children[i], elementCounter);
    }
};

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
    void* uiElementUniformBufferMapped,
    VkDeviceSize alignedUboSize,
    MeshRenderer* meshRenderers,
    UI_Manager* uiManager,
    f64 deltatime
) 
{
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
    
    
    //World pipeline
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, worldPipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, worldPipelineLayout, 0, 1, worldDescriptorSet, 0, 0);
    
    for(u16 i=0; i< DynamicArray_Length(meshRenderers); i++){
        MeshRenderer* m = &meshRenderers[i];
        PBR_PushConstant pc = {
            .model = m->mat4
        };
        VkBuffer vertexBuffers[] = { m->renderContext.vertexBuffer};
        VkDeviceSize offsets[] = {0};
        vkCmdPushConstants(
            commandBuffer,
            worldPipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT,
            0,
            sizeof(PBR_PushConstant),
            &pc
        );
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(commandBuffer, m->renderContext.indexBuffer,0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(commandBuffer, m->indicesCount, 1, 0, 0, 0);
    }

    vkCmdNextSubpass(commandBuffer, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, uiPipeline);
    
    //vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, uiPipelineLayout, 0, 1, uiDescriptorSet, 0, 0);

    recursiveUIElementsDraw(commandBuffer, uiElementUniformBufferMapped, alignedUboSize, deltatime, uiPipelineLayout, uiDescriptorSet, &uiManager->root, 0);

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