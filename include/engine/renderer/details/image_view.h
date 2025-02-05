#ifndef IMAGE_VIEW_H
#define IMAGE_VIEW_H

#include "engine/renderer/render_types.h"

VkImageView createImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
bool createSwapChainImageViews(VkDevice device, VkImage *swapchain_images, u32 swapchain_images_count, VkFormat swapchain_image_format, VkImageView* out_views);
void destroyImageView(VkDevice device, VkImageView view);
void destroySwapChainImageViews(VkDevice device, VkImageView *views, u32 views_count);

#endif // IMAGE_VIEW_H