#include "renderer/backend/vulkan/vulkan_command.h"

#include "renderer/backend/vulkan/details/context/queue.h"
#include "core/debugger.h"

////////////////////////////
///                      ///
///     COMMAND POOL     ///
///                      ///
////////////////////////////

void createVulkanCommandPool(VkPhysicalDevice gpu, VkDevice device, VkSurfaceKHR surface, VkCommandPool *out){
    QueueFamilyIndices indices = findQueueFamilies(gpu, surface);

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = indices.graphicsFamily;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    if (vkCreateCommandPool(device, &poolInfo, 0, out) != VK_SUCCESS)
    {
        LOG_FATAL("Failed to create command pool");
        return;
    }
    freeQueeFamiliesInfoStruct(indices);
}

void destroyVulkanCommandPool(VkDevice device, VkCommandPool commandPool)
{
    vkDestroyCommandPool(device, commandPool, 0);
}

//////////////////////////////
///                        ///
///     COMMAND BUFFER     ///
///                        ///
//////////////////////////////

void createVulkanCommandBuffer(VkDevice device, VkCommandPool pool, VkCommandBuffer* out){
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = pool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = MAX_FRAMES_IN_FLIGHT;

    vkAllocateCommandBuffers(device, &allocInfo, out);
}

void destroyVulkanCommandBuffer(VkDevice device, VkCommandPool pool, VkCommandBuffer* buff){
    vkFreeCommandBuffers(device, pool, 1, buff);
}

////////////////////////////////
///                          ///
///     COMMAND RECORING     ///
///                          ///
////////////////////////////////

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

void recordCommandBuffer(VkCommandBuffer cmdBuff, VkRenderPass renderpass, SwapchainObj swapchain, u32 imageIndx){
    VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = 0, // Optional
        .pInheritanceInfo = 0 // Optional
    };
    VkResult res = vkBeginCommandBuffer(cmdBuff, &beginInfo);
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
        .framebuffer = swapchain.frameBuffers[imageIndx],
        .renderArea = {
            .offset = {0, 0},
            .extent = swapchain.extent
        },
        .clearValueCount = clearValuesCount,
        .pClearValues = clearValues
    };

    vkCmdBeginRenderPass(cmdBuff, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    VkViewport viewport = {
        .x = 0.0f,
        .y = 0.0f,
        .width = (float) swapchain.extent.width,
        .height = (float) swapchain.extent.height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };
    vkCmdSetViewport(cmdBuff, 0, 1, &viewport);
    
    VkRect2D scissor = {
        .offset = {0, 0},
        .extent = swapchain.extent
    };
    vkCmdSetScissor(cmdBuff, 0, 1, &scissor);

}