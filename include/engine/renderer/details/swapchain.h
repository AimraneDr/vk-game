#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

#include "engine/renderer/render_types.h"

Result createSwapChain
(
    VkPhysicalDevice gpu, 
    VkDevice device, 
    VkSurfaceKHR surface, 
    u32 window_width, u32 window_height, 
    VkSwapchainKHR* out_swapchain, 
    VkImage* out_images, 
    VkFormat* out_format, 
    VkExtent2D* out_extent
);

void recreateSwapChain(VkDevice device, VkSwapchainKHR swapchain);
void destroySwapChain(VkDevice device, VkSwapchainKHR swapchain); 

void querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface, SwapChainSupportDetails* details);
void freeSwapChainSupportDetails(SwapChainSupportDetails* details);

#endif //SWAPCHAIN_H