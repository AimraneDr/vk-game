#include "renderer/details/image_view.h"

#include "core/debugger.h"
#include <stdlib.h>

VkImageView createImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, u16 mipLevels){
    VkImageViewCreateInfo viewInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = format,
        .subresourceRange.aspectMask = aspectFlags,
        .subresourceRange.baseMipLevel = 0,
        .subresourceRange.levelCount = mipLevels,
        .subresourceRange.baseArrayLayer = 0,
        .subresourceRange.layerCount = 1
    };
    VkImageView imageView;
    VkResult res = vkCreateImageView(device, &viewInfo, 0, &imageView);
    if (res != VK_SUCCESS) {
        LOG_FATAL("failed to create image view!");
    }
    return imageView;
}

bool createSwapChainImageViews(
    VkDevice device, 
    VkImage *swapchain_images,
    u32 swapchain_images_count, 
    VkFormat swapchain_image_format,
    VkImageView* out_views
)
{

    for (int i = 0; i < swapchain_images_count; i++)
    {
        out_views[i] = createImageView(device, swapchain_images[i], swapchain_image_format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
    }
    return TRUE;
}

void destroyImageView(VkDevice device, VkImageView view){
        vkDestroyImageView(device, view, 0);
}
void destroySwapChainImageViews(VkDevice device, VkImageView *views, u32 views_count)
{
    for (int i = 0; i < views_count; i++)
    {
        vkDestroyImageView(device, views[i], 0);
    }
    free(views);
}