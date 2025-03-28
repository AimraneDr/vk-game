#include "renderer/details/color.h"

#include "renderer/details/image.h"
#include "renderer/details/image_view.h"

void createColorResources(
    VkPhysicalDevice gpu, VkDevice device, 
    VkCommandPool cmdPool, VkQueue queue,
    VkExtent2D swapChainExtent, VkSampleCountFlagBits msaaSamples,
    VkFormat colorFormat, 
    VkImage* outColorImage, 
    VkDeviceMemory* outColorImageMemory, VkImageView* outColorImageView)
{

    createImage(
        gpu, device,
        swapChainExtent.width, swapChainExtent.height,
        1, msaaSamples,
        colorFormat, VK_IMAGE_TILING_OPTIMAL, 
        VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
        outColorImage, outColorImageMemory);
    
    *outColorImageView = createImageView(device, *outColorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
}

void destroyColorResources(VkDevice device, VkImage colorImage, VkDeviceMemory colorImageMemory, VkImageView colorImageView){
    destroyImageView(device, colorImageView);
    destroyImage(device, colorImage, colorImageMemory);
}