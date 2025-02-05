#include "engine/renderer/details/framebuffer.h"

#include "engine/core/debugger.h"
#include <stdlib.h>

void createFramebuffers(
    VkDevice device, 
    VkRenderPass render_pass, 
    VkImageView *swapchain_image_views, 
    VkImageView depthImageView,
    u32 swapchain_image_count, 
    VkExtent2D swapchain_extent, 
    VkFramebuffer* *out_framebuffers
    ){
        VkFramebuffer* temp = (VkFramebuffer*)malloc(sizeof(VkFramebuffer) * swapchain_image_count);
        for(u32 i=0; i < swapchain_image_count; i++){
            const u8 attachmentsCount = 2;
            VkImageView attachments[attachmentsCount] = {
                swapchain_image_views[i],
                depthImageView
            };

            VkFramebufferCreateInfo framebufferInfo = {
                .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                .renderPass = render_pass,
                .attachmentCount = attachmentsCount,
                .pAttachments = attachments,
                .width = swapchain_extent.width,
                .height = swapchain_extent.height,
                .layers = 1
            };

            VkResult res = vkCreateFramebuffer(device, &framebufferInfo, 0, &temp[i]);
            if(res != VK_SUCCESS){
                LOG_FATAL("Failed to create framebuffer");
                return;
            }
        }
        *out_framebuffers = temp;
}

void destroyFramebuffers(VkDevice device, VkFramebuffer *framebuffers, u32 framebuffers_count){
    for(u32 i=0; i < framebuffers_count; i++){
        vkDestroyFramebuffer(device, framebuffers[i], 0);
    }
    free(framebuffers);
}