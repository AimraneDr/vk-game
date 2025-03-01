#include "renderer/details/framebuffer.h"

#include "core/debugger.h"
#include <stdlib.h>

void createFramebuffers(
    VkDevice device, 
    VkRenderPass render_pass, 
    VkImageView *swapchain_image_views, 
    VkImageView colorImageView,
    VkImageView depthImageView,
    u32 swapchain_image_count, 
    VkExtent2D swapchain_extent, 
    VkFramebuffer* *out_framebuffers
    ){
        VkFramebuffer* temp = (VkFramebuffer*)malloc(sizeof(VkFramebuffer) * swapchain_image_count);
        for(u32 i=0; i < swapchain_image_count; i++){
            #define attachmentsCount 3
            VkImageView attachments[attachmentsCount] = {
                colorImageView,
                depthImageView,
                swapchain_image_views[i],
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