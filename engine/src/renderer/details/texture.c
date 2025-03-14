#include "renderer/details/texture.h"

#include "core/debugger.h"
#include "assets/asset_manager.h"
#include "renderer/details/buffer.h"
#include "renderer/details/commandBuffer.h"
#include "renderer/details/image.h"
#include "renderer/details/image_view.h"
#include "renderer/render_context.h"


void createTextureImage(VkPhysicalDevice gpu, VkDevice device, VkCommandPool cmdPool, VkQueue queue, void* pixels, Texture* outTexture);
void createTextureImageView(VkDevice device, VkImage textureImage, u16 mipLevels, VkImageView* outTextureImageView);
void createTextureImageSampler(VkPhysicalDevice gpu, VkDevice device, u16 mipLevels, VkSampler* outTextureSampler);
void destroyTextureImage(VkDevice device, VkImage   , VkDeviceMemory textureImageMemory);
void destroyTextureImageView(VkDevice device, VkImageView textureImageView);
void destroyTextureSampler(VkDevice device, VkSampler sampler);


/// @brief the out->hight, out->width, out->mipmaplevels should be set before passing it down
/// @param pixels 
/// @param out 
void createTexture(void* pixels, Texture* out){
    RendererContext* rc = getRendererContext();
    createTextureImage(rc->gpu, rc->device, rc->commandPool, rc->queue.graphics, pixels, out);
    createTextureImageView(rc->device, out->image, out->mipLevels,&out->imageView);
    createTextureImageSampler(rc->gpu, rc->device, out->mipLevels, &out->sampler);
}

void destroyTexture(Texture* tex){
    RendererContext* rc = getRendererContext();
    destroyTextureImage(rc->device, tex->image, tex->memory);
    destroyTextureImageView(rc->device, tex->imageView);
    destroyTextureSampler(rc->device, tex->sampler);
}

void createDefaultTextures(Material* default_material){
    //TODO: change to aquiring the texture instead of loading it, the load opperation should be handled by the assets manager
    Asset* texture = load_asset("./../resources/textures/default.png", "default");
    
    //albedo
    default_material->albedo = texture->data;
    //normal
    default_material->normal = texture->data;
    //mtalic/roughness/AO
    default_material->metalRoughAO = texture->data;
    //emmision
    default_material->emissive = texture->data;
    //height
    default_material->height = texture->data;
}

void destroyDefaultTextures(Material* default_material){
    //already released by the asset manager when shutting down
    // release_asset("default", ASSET_TYPE_TEXTURE);

    //albedo
    default_material->albedo = 0;
    //normal
    default_material->normal = 0;
    //mtalic/roughness/AO
    default_material->metalRoughAO = 0;
    //emmision
    default_material->emissive = 0;
    //height
    default_material->height = 0;
}

void generateMipmaps(VkPhysicalDevice gpu, VkDevice device, VkCommandPool cmdPool, VkQueue queue, VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels) {
    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(gpu, imageFormat, &formatProperties);
    if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
        LOG_ERROR("texture image format does not support linear blitting!");
    }

    VkCommandBuffer commandBuffer = beginSingleTimeCommands(device, cmdPool);

    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;

    u32 mipWidth = texWidth;
    u32 mipHeight = texHeight;

    for (u32 i = 1; i < mipLevels; i++) {
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        
        vkCmdPipelineBarrier(commandBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
            0, 0,
            0, 0,
            1, &barrier);
        
        VkImageBlit blit = {
            .srcOffsets[0] = { 0, 0, 0 },
            .srcOffsets[1] = { mipWidth, mipHeight, 1 },
            .srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .srcSubresource.mipLevel = i - 1,
            .srcSubresource.baseArrayLayer = 0,
            .srcSubresource.layerCount = 1,
            .dstOffsets[0] = { 0, 0, 0 },
            .dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 },
            .dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .dstSubresource.mipLevel = i,
            .dstSubresource.baseArrayLayer = 0,
            .dstSubresource.layerCount = 1
        };

        vkCmdBlitImage(commandBuffer,
            image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1, &blit,
            VK_FILTER_LINEAR);

        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
            0, 0,
            0, 0,
            1, &barrier);

        if (mipWidth > 1) mipWidth /= 2;
        if (mipHeight > 1) mipHeight /= 2;
    }

    barrier.subresourceRange.baseMipLevel = mipLevels - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(commandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
        0, 0,
        0, 0,
        1, &barrier);

    endSingleTimeCommands(device, cmdPool, queue, &commandBuffer);
}

void createTextureImage(VkPhysicalDevice gpu, VkDevice device, VkCommandPool cmdPool, VkQueue queue, void* pixels, Texture* outTexture){
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    VkDeviceSize imageSize = outTexture->width*outTexture->height * 4 ; //4 channels
    createBuffer(gpu, device, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);

    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
        memcpy(data, pixels, (u32)imageSize);
    vkUnmapMemory(device, stagingBufferMemory);


    createImage(gpu, device, outTexture->width, outTexture->height, outTexture->mipLevels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &outTexture->image, &outTexture->memory);
    
    transitionImageLayout(device, cmdPool, queue, outTexture->mipLevels, outTexture->image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    copyBufferToImage(device, cmdPool, queue, stagingBuffer, outTexture->image, outTexture->width, outTexture->height);
    // transitionImageLayout(device, cmdPool, queue, *outMipLevels, *outTextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkDestroyBuffer(device, stagingBuffer, 0);
    vkFreeMemory(device, stagingBufferMemory, 0);

    generateMipmaps(gpu, device, cmdPool, queue, outTexture->image, VK_FORMAT_R8G8B8A8_SRGB, outTexture->width, outTexture->height, outTexture->mipLevels);
}

void createTextureImageSampler(VkPhysicalDevice gpu, VkDevice device, u16 mipLevels, VkSampler* outTextureSampler){
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
        .maxLod = (f32) mipLevels
    };

    VkResult res = vkCreateSampler(device, &samplerInfo, 0, outTextureSampler);
    if(res != VK_SUCCESS){
        LOG_FATAL("failed to create texture image sampler!");
    }
}

void createTextureImageView(VkDevice device, VkImage textureImage, u16 mipLevels, VkImageView* outTextureImageView){
    *outTextureImageView = createImageView(device, textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels);
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