#include "engine/renderer/details/texture.h"

#include "engine/core/debugger.h"
#include "engine/renderer/details/buffer.h"
#include "engine/renderer/details/image.h"
#include "engine/renderer/details/image_view.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

void createTextureImage(VkPhysicalDevice gpu, VkDevice device, VkCommandPool cmdPool, VkQueue queue, VkImage* outTextureImage, VkDeviceMemory* outTextureMemory){
    u32 texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load("C:/Users/aimra/Dev/vk-game/resources/textures/image.jpeg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    if (!pixels) {
        LOG_ERROR("failed to load texture image!");
    }

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    createBuffer(gpu, device, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);

    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
        memcpy(data, pixels, (u32)imageSize);
    vkUnmapMemory(device, stagingBufferMemory);

    stbi_image_free(pixels);

    createImage(gpu, device, texWidth, texHeight,VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, outTextureImage, outTextureMemory);
    
    transitionImageLayout(device, cmdPool, queue, *outTextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    copyBufferToImage(device, cmdPool, queue, stagingBuffer, *outTextureImage, texWidth, texHeight);
    transitionImageLayout(device, cmdPool, queue, *outTextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkDestroyBuffer(device, stagingBuffer, 0);
    vkFreeMemory(device, stagingBufferMemory, 0);
}

void createTextureImageSampler(VkPhysicalDevice gpu, VkDevice device, VkSampler* outTextureSampler){
    VkPhysicalDeviceProperties properties = {};
    vkGetPhysicalDeviceProperties(gpu, &properties);

    VkSamplerCreateInfo samplerInfo = {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .anisotropyEnable = VK_TRUE,
        .maxAnisotropy = properties.limits.maxSamplerAnisotropy,
        .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        .unnormalizedCoordinates = VK_FALSE,
        .compareEnable = VK_FALSE,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .mipLodBias = 0.f,
        .minLod = 0.f,
        .maxLod = 0.f
    };

    VkResult res = vkCreateSampler(device, &samplerInfo, 0, outTextureSampler);
    if(res != VK_SUCCESS){
        LOG_FATAL("failed to create texture image sampler!");
    }
}

void createTextureImageView(VkDevice device, VkImage textureImage, VkImageView* outTextureImageView){
    *outTextureImageView = createImageView(device, textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
}

void destroyTextureImage(VkDevice device, VkImage textureImage, VkDeviceMemory textureImageMemory){
    destroyImage(device, textureImage, textureImageMemory);
}

void destroyTextureImageView(VkDevice device, VkImageView textureImageView){
    destroyImageView(device, textureImageView);
}

void destroyTextureSampler(VkDevice device, VkSampler sampler){
    vkDestroySampler(device, sampler, 0);
}