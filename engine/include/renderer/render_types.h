#pragma once

#ifdef __linux__
#define VK_USE_PLATFORM_XLIB_KHR
#endif

#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#include <vulkan/vulkan.h>
#include <math/mathTypes.h>
#include "data_types.h"

#define MAX_FRAMES_IN_FLIGHT 3

typedef struct RendererInitConfig_t
{
    VkSampleCountFlagBits msaaSamples;
} RendererInitConfig;

typedef struct PipelineConfig
{
    // Vertex input
    VkVertexInputBindingDescription (*get_vertex_binding_desc)(void);
    void (*get_vertex_attr_descs)(u32 *count, VkVertexInputAttributeDescription **descs);

    // Depth/stencil
    VkBool32 depth_test_enable;
    VkBool32 depth_write_enable;
    VkCompareOp depth_compare_op;

    // Pipeline stages
    u32 subpass_index;
    u64 push_constant_size;
    VkShaderStageFlags push_constant_stage;

    // Blend factors
    VkBlendFactor src_color_blend_factor;
    VkBlendFactor dst_color_blend_factor;
} PipelineConfig;

typedef struct Texture_t
{
    u32 Idx;
    VkImage image;
    VkDeviceMemory memory;
    VkImageView imageView;
    VkSampler sampler;
    u32 width, height;
    u16 mipLevels;
} Texture;


typedef struct Material_t
{
    // --- Textures ---

    Texture* albedo;     // Base color (sRGB)
    Texture* normal;     // Normal map (RGBA)
    Texture* metalRoughAO; // Metallic (R), Roughness (G), AO (B)
    Texture* emissive;   // Emissive (RGB)
    Texture* height;     // Height (R)

    // --- Material Factors (GPU UBO) ---

    Vec4 albedoFactor;
    Vec4 emissiveFactor;
    float metallicFactor;
    float roughnessFactor;
    float aoFactor;
    float heightScale;

    // --- UV Transform ---
    
    Vec2 uvTiling; // Texture repeat (e.g., [2,2])
    Vec2 uvOffset; // Texture offset (e.g., [0.5,0.5])
    // --- Descriptor Set ---
    //set per frame in flight
    VkDescriptorSet descriptorSets[MAX_FRAMES_IN_FLIGHT];
} Material;

typedef struct Pipeline_t
{
    VkPipelineLayout pipelineLayout;
    VkPipeline ref;

    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorPool descriptorPool;
    VkDescriptorSet descriptorSets[MAX_FRAMES_IN_FLIGHT];
    struct
    {
        struct
        {
            VkDescriptorPool pool;
            VkDescriptorSetLayout setLayout;
            VkBuffer buffers[MAX_FRAMES_IN_FLIGHT];
            VkDeviceMemory buffersMemory[MAX_FRAMES_IN_FLIGHT];
            void *buffersMapped[MAX_FRAMES_IN_FLIGHT];
            VkDescriptorSet sets[MAX_FRAMES_IN_FLIGHT];
        } global;
        struct
        {
            VkDescriptorPool pool;
            VkDescriptorSetLayout setLayout;
            VkBuffer buffers[MAX_FRAMES_IN_FLIGHT];
            VkDeviceMemory buffersMemory[MAX_FRAMES_IN_FLIGHT];
            void *buffersMapped[MAX_FRAMES_IN_FLIGHT];
            VkDeviceSize alignedUboSize;
        } element;
    } uniform;

    VkImage textureImage;
    VkDeviceMemory textureImageMemory;
    VkImageView textureImageView;
    VkSampler textureSampler;

    Material defaultMaterial;
} Pipeline;

typedef struct RenderPass_t{
    Pipeline* pipelines;
    u8 pipelines_count;
}RenderPass;

typedef struct attachment_t
{
    VkImage image;
    VkDeviceMemory memory;
    VkImageView view;
} Attachment;

typedef struct RendererContext_t {
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessanger;
    VkPhysicalDevice gpu;
    VkDevice device;
    VkSurfaceKHR surface;
    struct
    {
        VkQueue graphics;
        VkQueue present;
    } queue;

    VkSwapchainKHR swapchain;
    VkImage *swapchainImages;
    u32 swapchainImagesCount;
    VkFormat swapchainImageFormat;
    VkExtent2D swapchainExtent;
    VkImageView *swapchainImageViews;
    VkFramebuffer *swapchainFrameBuffers;
    Attachment color;
    Attachment depth;
    VkSampleCountFlagBits msaaSamples;

    //TODO: remove from here
    VkCommandPool commandPool;
    VkCommandBuffer commandBuffers[MAX_FRAMES_IN_FLIGHT];
}RendererContext;

typedef struct Renderer
{
    RendererContext* context;
    VkRenderPass renderPass;
    u16 mipLevels;

    //  sync
    struct
    {
        VkSemaphore imageAvailableSemaphores[MAX_FRAMES_IN_FLIGHT];
        VkSemaphore renderFinishedSemaphores[MAX_FRAMES_IN_FLIGHT];
        VkFence inFlightFences[MAX_FRAMES_IN_FLIGHT];
    } sync;

    u8 currentFrame;
    bool framebufferResized;
} Renderer;

typedef struct MeshData_t{
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;

    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;
    u32 indicesCount;
}MeshData;

typedef struct QueueFamilyIndices
{
    i32 graphicsFamily;
    i32 computeFamily;
    i32 presentFamily;
    u32 familiesCount;
    i32 *uniqueFamilies;
} QueueFamilyIndices;

typedef struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    u32 formatsCount;
    VkSurfaceFormatKHR *formats;
    u32 presentModesCount;
    VkPresentModeKHR *presentModes;
} SwapChainSupportDetails;


typedef struct PBR_GLOBAL_UBO_t
{
    Mat4 view;
    Mat4 proj;
} PBR_GLOBAL_UBO;

typedef struct PBR_Material_UBO_t
{
    Vec4 albedoFactor;
    Vec4 emissiveFactor;
    Vec2 uvTiling;
    Vec2 uvOffset;
    
    u32 albedo_idx;
    u32 normal_idx;
    u32 metalRoughAO_idx;
    u32 emissive_idx;
    u32 height_idx;

    f32 metallicFactor;
    f32 roughnessFactor;
    f32 aoFactor;
    f32 heightScale;

} PBR_Material_UBO;

typedef struct PBR_PushConstant_t
{
    Mat4 model;
} PBR_PushConstant;

typedef struct UI_Global_UBO_t
{
    Mat4 proj;
} UI_Global_UBO;

typedef struct UI_Element_UBO_t
{
    Vec4 color;
    Vec2 size;
} UI_Element_UBO;

typedef struct UI_PushConstant_t
{
    Mat4 model;
} UI_PushConstant;