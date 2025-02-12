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

    struct{
        VkPipelineLayout pipelineLayout;
        VkPipeline graphicsPipeline;

        VkDescriptorSetLayout descriptorSetLayout;
        VkDescriptorPool descriptorPool;
        VkDescriptorSet descriptorSets[MAX_FRAMES_IN_FLIGHT];
        struct{
            VkBuffer buffers[MAX_FRAMES_IN_FLIGHT];
            VkDeviceMemory buffersMemory[MAX_FRAMES_IN_FLIGHT];
            void* buffersMapped[MAX_FRAMES_IN_FLIGHT];
        }uniform;
    }world;

    
    //  sync
    struct{
        VkSemaphore imageAvailableSemaphores[MAX_FRAMES_IN_FLIGHT];
        VkSemaphore renderFinishedSemaphores[MAX_FRAMES_IN_FLIGHT];
        VkFence inFlightFences[MAX_FRAMES_IN_FLIGHT];
    }sync;
    
    struct {
        VkDescriptorPool descriptorPool;
        VkDescriptorSetLayout descriptorSetLayout;
        VkDescriptorSet descriptorSets[MAX_FRAMES_IN_FLIGHT];
        
        VkPipelineLayout pipelineLayout;
        VkPipeline graphicsPipeline;
        
        struct{
            VkBuffer buffers[MAX_FRAMES_IN_FLIGHT];
            VkDeviceMemory buffersMemory[MAX_FRAMES_IN_FLIGHT];
            void* buffersMapped[MAX_FRAMES_IN_FLIGHT];
        }uniform;
        
        VkBuffer vertexBuffer;
        VkDeviceMemory vertexBufferMemory;
        
        VkBuffer indexBuffer;
        VkDeviceMemory indexBufferMemory;
    }ui;

    // TODO: move to texture asset/component
    VkImage textureImage;
    VkDeviceMemory textureImageMemory;
    VkImageView textureImageView;
    VkSampler textureSampler;

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

typedef struct UniformBufferObject {
    Mat4 view;
    Mat4 proj;
}UniformBufferObject;

typedef struct UI_UniformBufferObject {
    Mat4 proj;
}UI_UniformBufferObject;

#endif //RENDER_TYPES_H