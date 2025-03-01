#include "renderer/details/depth.h"

#include "renderer/details/image.h"
#include "renderer/details/image_view.h"
#include "core/debugger.h"


void createDepthResources(
    VkPhysicalDevice gpu, VkDevice device, 
    VkCommandPool cmdPool, VkQueue queue,
    VkExtent2D swapChainExtent,  VkSampleCountFlagBits msaaSamples,
    VkImage* outDepthImage, 
    VkDeviceMemory* outDepthImageMemory, VkImageView* outDepthImageView)
{
    VkFormat depthFormat = findDepthFormat(gpu);

    createImage(gpu, device,swapChainExtent.width, swapChainExtent.height, 1, msaaSamples, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, outDepthImage, outDepthImageMemory);
    *outDepthImageView = createImageView(device, *outDepthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);

    transitionImageLayout(device, cmdPool, queue, 1, *outDepthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

}

void destroyDepthResources(VkDevice device, VkImage depthImage, VkDeviceMemory depthImageMemory, VkImageView depthImageView){
    destroyImageView(device, depthImageView);
    destroyImage(device, depthImage, depthImageMemory);
}

VkFormat findSupportedFormat(VkPhysicalDevice gpu, const VkFormat* candidates, const u8 condidatsCount, VkImageTiling tiling, VkFormatFeatureFlags features) {

    for(u8 i=0; i< condidatsCount; i++){
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(gpu, candidates[i], &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
            return candidates[i];
        } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
            return candidates[i];
        }
    }

    
    LOG_ERROR("failed to find supported format!");
    return 0;
}

VkFormat findDepthFormat(VkPhysicalDevice gpu) {
    VkFormat condidates[] = {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT};
    return findSupportedFormat(gpu,
        condidates,
        3,
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}