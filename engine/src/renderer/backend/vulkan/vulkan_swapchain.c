#include "renderer/backend/vulkan/vulkan_swapchain.h"

#include "renderer/backend/vulkan/details/swapchain.h"

void createVulkanSwapchainObj(VulkanContext* context, u32 width, u32 height, SwapchainObj* out){
    
    createSwapChain(
        context->gpu, 
        context->device, 
        context->surface, 
        width, 
        height, 
        &out->ref, 
        &out->images, 
        &out->imagesCount, 
        &out->imageFormat, 
        &out->extent);

    out->imageViews = (VkImageView *)malloc(sizeof(VkImageView) * out->imagesCount);

    createSwapChainImageViews(
        context->device, 
        out->images, 
        out->imagesCount, 
        out->imageFormat, 
        out->imageViews);
}

void destroyVulkanSwapchainObj(VulkanContext* context, SwapchainObj* swapchain){
    
    destroySwapChain(context->device, swapchain->ref);
}