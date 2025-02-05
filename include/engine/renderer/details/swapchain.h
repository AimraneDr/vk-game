#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

#include "engine/renderer/render_types.h"

bool createSwapChain
(
    VkPhysicalDevice gpu, 
    VkDevice device, 
    VkSurfaceKHR surface, 
    u32 window_width, u32 window_height, 
    VkSwapchainKHR* out_swapchain, 
    VkImage* *out_images, 
    u32 *out_images_count,
    VkFormat* out_format, 
    VkExtent2D* out_extent
);

void destroySwapChain(VkDevice device, VkSwapchainKHR swapchain); 

void querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface, SwapChainSupportDetails* details);
void freeSwapChainSupportDetails(SwapChainSupportDetails* details);

#endif //SWAPCHAIN_H