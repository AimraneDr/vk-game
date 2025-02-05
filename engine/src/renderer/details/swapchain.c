#include "renderer/details/swapchain.h"

#include <stdlib.h>
#include "renderer/details/queue.h"
#include "renderer/details/image_view.h"
#include "renderer/details/framebuffer.h"
#include "core/debugger.h"

// to be moved to math
u32 clamp(u32 value, u32 min, u32 max)
{
    return value > max ? max : value < min ? min
                                           : value;
};

void querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface, SwapChainSupportDetails *details)
{
    VkResult res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details->capabilities);
    if(res != VK_SUCCESS){
        LOG_FATAL("Failed to get physical device surface capabilities");
        return;
    }
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &details->formatsCount, 0);
    if (details->formatsCount > 0)
    {
        details->formats = (VkSurfaceFormatKHR *)malloc(sizeof(VkSurfaceFormatKHR) * details->formatsCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &details->formatsCount, details->formats);
    }

    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &details->presentModesCount, 0);
    if (details->presentModesCount > 0)
    {
        details->presentModes = (VkPresentModeKHR *)malloc(sizeof(VkPresentModeKHR) * details->presentModesCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &details->presentModesCount, details->presentModes);
    }
};

void freeSwapChainSupportDetails(SwapChainSupportDetails *details)
{
    free(details->formats);
    free(details->presentModes);
}

VkSurfaceFormatKHR chooseSeapChainSurfaceFormat(VkSurfaceFormatKHR *formats, u32 formats_count)
{
    for (u32 i = 0; i < formats_count; i++)
    {
        if (formats[i].format == VK_FORMAT_B8G8R8A8_SRGB && formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return formats[i];
        }
    }
    return formats[0];
}

VkPresentModeKHR chooseSeapChainPresentMode(VkPresentModeKHR *modes, u32 modes_count)
{
    for (u32 i = 0; i < modes_count; i++)
    {
        if (modes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return modes[i];
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D chooseSeapChainExtent(const VkSurfaceCapabilitiesKHR capabilities, u32 window_w, u32 window_h)
{
    if (capabilities.currentExtent.height != UINT32_MAX)
    {
        return capabilities.currentExtent;
    }
    else
    {
        VkExtent2D actualExtent = {
            .width = clamp(window_w, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
            .height = clamp(window_h, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)};
        return actualExtent;
    }
}

bool createSwapChain(
    VkPhysicalDevice gpu,
    VkDevice device,
    VkSurfaceKHR surface,
    u32 window_width, u32 window_height,
    VkSwapchainKHR *out_swapchain,
    VkImage **out_images,
    u32 *out_images_count,
    VkFormat *out_format,
    VkExtent2D *out_extent)
{
    SwapChainSupportDetails swapChainSupport;
    querySwapChainSupport(gpu, surface, &swapChainSupport);

    VkSurfaceFormatKHR surfaceFormat = chooseSeapChainSurfaceFormat(swapChainSupport.formats, swapChainSupport.formatsCount);
    VkPresentModeKHR presentMode = chooseSeapChainPresentMode(swapChainSupport.presentModes, swapChainSupport.presentModesCount);
    VkExtent2D extent = chooseSeapChainExtent(swapChainSupport.capabilities, window_width, window_height);

    u32 imagesCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imagesCount > swapChainSupport.capabilities.maxImageCount)
    {
        imagesCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = surface,
        .minImageCount = imagesCount,
        .imageFormat = surfaceFormat.format,
        .imageColorSpace = surfaceFormat.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT};

    QueueFamilyIndices indices = findQueueFamilies(gpu, surface);
    if (indices.graphicsFamily != indices.presentFamily)
    {
        u32 queueFamilies[] = {indices.graphicsFamily, indices.presentFamily};
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilies;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = 0;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    VkResult res = vkCreateSwapchainKHR(device, &createInfo, 0, out_swapchain);

    if (res != VK_SUCCESS)
    {
        freeSwapChainSupportDetails(&swapChainSupport);
        LOG_FATAL("Failed to create swapchain");
        return FALSE;
    }
    LOG_DEBUG("Swapchain created successfully");

    *out_extent = extent;
    *out_format = surfaceFormat.format;
    // suspected error here
    vkGetSwapchainImagesKHR(device, *out_swapchain, out_images_count, 0);
    *out_images = (VkImage *)malloc(sizeof(VkImage) * (*out_images_count));
    vkGetSwapchainImagesKHR(device, *out_swapchain, out_images_count, *out_images);

    freeSwapChainSupportDetails(&swapChainSupport);
    return TRUE;
}


void destroySwapChain(VkDevice device, VkSwapchainKHR swapchain)
{
    vkDestroySwapchainKHR(device, swapchain, 0);
}