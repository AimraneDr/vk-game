#ifndef PIPELINE_H
#define PIPELINE_H

#include "renderer/render_types.h"

void createPipeline(
    VkDevice device, 
    const char* vertshadername, const char* fragshadername,
    VkExtent2D extent, VkSampleCountFlagBits msaaSamples, 
    VkRenderPass renderpass, VkDescriptorSetLayout descriptorSetLayout, 
    VkPipelineLayout* out_layout, VkPipeline* out_pipeline);
void destroyPipeline(VkDevice device, VkPipeline* pipeline, VkPipelineLayout layout);

#endif //PIPELINE_H