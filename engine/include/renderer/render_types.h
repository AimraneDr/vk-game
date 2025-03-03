#pragma once
#ifndef RENDER_TYPES_H
#define RENDER_TYPES_H

#ifdef __linux__
#define VK_USE_PLATFORM_XLIB_KHR
#endif

#ifdef _WIN32 
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#include <vulkan/vulkan.h>
#include <math/mathTypes.h>
#include "data_types.h"


#define MAX_FRAMES_IN_FLIGHT 2

typedef struct RendererInitConfig_t{
    VkSampleCountFlagBits msaaSamples;
}RendererInitConfig;

typedef struct PipelineConfig {
    // Vertex input
    VkVertexInputBindingDescription (*get_vertex_binding_desc)(void);
    void (*get_vertex_attr_descs)(u32* count, VkVertexInputAttributeDescription** descs);
    
    // Depth/stencil
    VkBool32 depth_test_enable;
    VkBool32 depth_write_enable;
    VkCompareOp depth_compare_op;
    
    // Pipeline stages
    u32 subpass_index;
    size_t push_constant_size;
    VkShaderStageFlags push_constant_stage;
    
    // Blend factors
    VkBlendFactor src_color_blend_factor;
    VkBlendFactor dst_color_blend_factor;
} PipelineConfig;

typedef struct Pipeline_t{
    VkPipelineLayout pipelineLayout;
    VkPipeline ref;

    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorPool descriptorPool;
    VkDescriptorSet descriptorSets[MAX_FRAMES_IN_FLIGHT];
    struct{
        struct{
            VkBuffer buffers[MAX_FRAMES_IN_FLIGHT];
            VkDeviceMemory buffersMemory[MAX_FRAMES_IN_FLIGHT];
            void* buffersMapped[MAX_FRAMES_IN_FLIGHT];
        }global;
        struct{
            VkBuffer buffers[MAX_FRAMES_IN_FLIGHT];
            VkDeviceMemory buffersMemory[MAX_FRAMES_IN_FLIGHT];
            void* buffersMapped[MAX_FRAMES_IN_FLIGHT];
            VkDeviceSize alignedUboSize;
        }element;
    }uniform;

    VkImage textureImage;
    VkDeviceMemory textureImageMemory;
    VkImageView textureImageView;
    VkSampler textureSampler;
}Pipeline;

typedef struct Renderer{
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessanger;
    VkPhysicalDevice gpu;
    VkDevice device;
    VkSurfaceKHR surface;
    struct{
        VkQueue graphics;
        VkQueue present;
    }queue;

    VkSwapchainKHR swapchain;
    VkImage* swapchainImages;
    u32 swapchainImagesCount;
    VkFormat swapchainImageFormat;
    VkExtent2D swapchainExtent;
    VkImageView* swapchainImageViews;
    VkFramebuffer* swapchainFrameBuffers;

    VkRenderPass renderPass;
    VkCommandPool commandPool;
    VkCommandBuffer commandBuffers[MAX_FRAMES_IN_FLIGHT];
    VkSampleCountFlagBits msaaSamples;
    u16 mipLevels;

    struct{
        struct{
            VkImage image;
            VkDeviceMemory memory;
            VkImageView view;
        }color;

        struct{
            VkImage image;
            VkDeviceMemory memory;
            VkImageView view;
        }depth;
    }attachments;
    
    //  sync
    struct{
        VkSemaphore imageAvailableSemaphores[MAX_FRAMES_IN_FLIGHT];
        VkSemaphore renderFinishedSemaphores[MAX_FRAMES_IN_FLIGHT];
        VkFence inFlightFences[MAX_FRAMES_IN_FLIGHT];
    }sync;

    u8 currentFrame;
    bool framebufferResized;
}Renderer;

typedef struct QueueFamilyIndices{
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

typedef struct PBR_GLOBAL_UBO_t {
    Mat4 view;
    Mat4 proj;
}PBR_GLOBAL_UBO;

typedef struct PBR_Mesh_UBO_t {

}PBR_Mesh_UBO;

typedef struct PBR_PushConstant_t{
    Mat4 model;
}PBR_PushConstant;

typedef struct UI_Global_UBO_t {
    Mat4 proj;
}UI_Global_UBO;

typedef struct UI_Element_UBO_t {
    Vec4 color;
}UI_Element_UBO;

typedef struct UI_PushConstant_t{
    Mat4 model;
}UI_PushConstant;

#endif //RENDER_TYPES_H