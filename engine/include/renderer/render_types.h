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
#include <string/str_types.h>
#include "data_types.h"


#define MAX_FRAMES_IN_FLIGHT 2

typedef enum RendererBackend_e{
    RENDERER_BACKEND_VULKAN,
    RENDERER_BACKEND_OPENGL,
    RENDERER_BACKEND_DIRECTX,
    RENDERER_BACKEND_METAL,
    MAX_RENDERER_BACKENDS
}RendererBackend;

typedef enum DescriptorType_e{
    DESCRIPTOR_TYPE_UNIFORM_BUFFER,
    DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
    DESCRIPTOR_TYPE_STORAGE_BUFFER,
    DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
    DESCRIPTOR_TYPE_COMBINED_SAMPLER,
    DESCRIPTOR_TYPE_SAMPLER,
    DESCRIPTOR_TYPE_SAMPLED_IMAGE
} DescriptorType;

typedef enum ShaderStage_e{
    SHADER_STAGE_VERTEX = 1<<0,
    SHADER_STAGE_FRAGMENT = 1<<1,
    SHADER_STAGE_GEOMETRY = 1<<2
}ShaderStage;

typedef enum PipelineType_e{
    PIPELINE_TYPE_2D,
    PIPELINE_TYPE_3D,
}PipelineType;

typedef enum Topology_t{
    TOPOLOGY_POINT,
    TOPOLOGY_LINE,
    TOPOLOGY_LINE_STRIP,
    TOPOLOGY_TRIANGLE,
    TOPOLOGY_TRIANGLE_STRIP,
}Topology;


typedef struct BindingConfig_t{
    DescriptorType type;
    ShaderStage stages;
    u8 count;
    u32 size;
    u32 dynamicCount;  // Number of dynamic elements for dynamic buffers
    String defaultTexturePath; //if the binding a texture

    //
    
}BindingConfig;

typedef struct DescriptorSetConfig_t{
    BindingConfig* bindings;
    u8 bindingsCount;
}DescriptorSetConfig;

typedef struct PushConstantConfig_t{
    bool enabled;
    u8 size;
    ShaderStage stages;
}PushConstantConfig;

typedef struct PipelineConfig_t{
    DescriptorSetConfig* sets;
    u8 setsCount;

    //TODO: change to a file instead of string if it is more convienient
    String vertexShaderPath;
    String fragmentShaderPath;
    String geometryShaderPath;

    PipelineType type;
    Topology topology;

    PushConstantConfig pushConstant;
    bool depthTestingEnabled;
}PipelineConfig;

typedef struct RenderpassInitConfig_t{
    PipelineConfig* pipelines;
    u8 pipelinesCount;

    u8 msaaSamples;
}RenderpassInitConfig;

typedef struct RendererInitConfig_t{
    //new
    RendererBackend backend;
    RenderpassInitConfig* renderpasses;
    u8 renderpassesCount;

}RendererInitConfig;


typedef u64* RendererRef;

typedef struct RenderState_t{
    RendererBackend backend;
    RendererRef ref;
}RenderState;





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