#ifndef PIPELINE_H
#define PIPELINE_H

#include "renderer/render_types.h"

void create_graphics_pipeline(
    VkDevice device,
    const char* vert_shader,
    const char* frag_shader,
    VkExtent2D extent,
    VkSampleCountFlagBits msaa_samples,
    VkRenderPass render_pass,
    VkDescriptorSetLayout descriptor_set_layout,
    const PipelineConfig* config,
    VkPipelineLayout* out_layout,
    VkPipeline* out_pipeline);


void createPipeline(
    VkDevice device, 
    const char* vertshadername, const char* fragshadername,
    VkExtent2D extent, VkSampleCountFlagBits msaaSamples, 
    VkRenderPass renderpass, VkDescriptorSetLayout descriptorSetLayout, 
    VkPipelineLayout* out_layout, VkPipeline* out_pipeline);

void UI_createPipeline(
    VkDevice device, 
    const char* vertshadername, const char* fragshadername,
    VkExtent2D extent, VkSampleCountFlagBits msaaSamples, 
    VkRenderPass renderpass, VkDescriptorSetLayout descriptorSetLayout, 
    VkPipelineLayout* out_layout, VkPipeline* out_pipeline);
void destroyPipeline(VkDevice device, VkPipeline* pipeline, VkPipelineLayout layout);

#endif //PIPELINE_H