#ifndef RENDERER_BACKEND_VULKAN_TYPES_H
#define RENDERER_BACKEND_VULKAN_TYPES_H

#ifdef __linux__
#define VK_USE_PLATFORM_XLIB_KHR
#endif

#ifdef _WIN32 
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#include "data_types.h"
#include <vulkan/vulkan.h>
#include "renderer/render_types.h"

typedef struct VulkanContext_t{
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessanger;
    VkPhysicalDevice gpu;
    VkDevice device;
    VkSurfaceKHR surface;
    struct{
        VkQueue graphics;
        VkQueue present;
    }queue;
}VulkanContext;

typedef struct SwapchainObj_t{
    VkSwapchainKHR ref;
    VkImage* images;
    u8 imagesCount;
    VkFormat imageFormat;
    VkExtent2D extent;
    VkImageView* imageViews;
    VkFramebuffer* frameBuffers;
    bool framebufferResized;
}SwapchainObj;

typedef struct FramebufferAttachment {
    VkImage image;
    VkDeviceMemory memory;
    VkImageView view;
} FramebufferAttachment;

typedef struct SyncObj_t{
    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;
    VkFence inFlightFence;
}SyncObj;

typedef struct CommandObj_t{
    VkCommandPool pool;
    VkCommandBuffer buffers[MAX_FRAMES_IN_FLIGHT];
    SyncObj* syncs[MAX_FRAMES_IN_FLIGHT];
    u8 currentFrame;
}CommandObj;

typedef struct UniformBuffer_t {
    VkBuffer buffer;
    VkDeviceMemory memory;
    void* mapped;
    VkDeviceSize size;
    VkDeviceSize alignment;
} UniformBuffer;

typedef struct Texture_t{
    VkImage image;
    VkDeviceMemory memory;
    VkSampler sampler;
    VkImageView view;
    u8 mipLevels;
}Texture;

typedef struct PipelineResources_t{
    VkDescriptorSet* descriptorSets;
    u8 descriptorSetCount;
    UniformBuffer* uniformBuffers;
    u8 uniformBufferCount;
    UniformBuffer* storageBuffers;
    u8 storageBufferCount;
}PipelineResources;

//TODO: move from here as this struct should be API independent
typedef struct Material {
    Texture* textures;
    u16 texturesCount;
    VkDescriptorSet descriptorSet;
} Material;


typedef struct Pipeline_t{
    VkPipelineLayout layout;
    VkPipeline ref;
    VkDescriptorPool descriptorPool;
    VkDescriptorSetLayout* descriptorSetsLayout;
    u8 descriptorSetsLayoutCount;
    PipelineResources frameResources[MAX_FRAMES_IN_FLIGHT];
    Texture* textures;
    u16 texturesCount;
}Pipeline;

typedef struct RenderPass_t{
    VkRenderPass ref;
    struct{
        FramebufferAttachment color;
        FramebufferAttachment depth;
    }attachments;
    
    Pipeline* pipelines;
    u8 pipelinesCount;

    VkSampleCountFlagBits msaaSamples;
}RenderPass;

typedef struct QueueFamilyIndices_t{
    i32 graphicsFamily;
    i32 computeFamily;
    i32 presentFamily;
    u32 familiesCount;
    i32* uniqueFamilies;
}QueueFamilyIndices;

typedef struct SwapChainSupportDetails{
    VkSurfaceCapabilitiesKHR capabilities;
    u32 formatsCount;
    VkSurfaceFormatKHR* formats;
    u32 presentModesCount;
    VkPresentModeKHR* presentModes;
}SwapChainSupportDetails;

#endif //RENDERER_BACKEND_VULKAN_TYPES_H