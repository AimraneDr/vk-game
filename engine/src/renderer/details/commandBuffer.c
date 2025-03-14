#include "renderer/details/commandBuffer.h"

#include "renderer/details/uniformBuffer.h"
#include "core/debugger.h"
#include "components/UI/uiComponents.h"
#include <collections/DynamicArray.h>
#include <math/mat.h>
#include <math/vec4.h>
#include <math/vec2.h>

#include "ecs/ecs.h"

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

void recordCommandBuffer(GameState* gState, uint32_t imageIndex) 
{
    Renderer* r = &gState->renderer;
    VkCommandBuffer commandBuffer = r->context->commandBuffers[r->currentFrame]; 
    VkRenderPass renderpass = r->renderPass; 
    VkFramebuffer* swapChainFramebuffers = r->context->swapchainFrameBuffers;
    VkExtent2D extent = r->context->swapchainExtent;

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

    #define clearValuesCount 2
    VkClearValue clearValues[clearValuesCount] = {
        //0
        {
            .color = {{0.0f, 0.0f, 0.0f, 1.0f}}
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
    
    ecs_systems_update_group(gState, SYSTEM_GROUP_RENDERING);

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