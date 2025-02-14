#include "renderer/backend/vulkan/vulkan_renderer.h"

#include "renderer/backend/vulkan/vulkan_context.h"
#include "renderer/backend/vulkan/vulkan_swapchain.h"
#include "renderer/backend/vulkan/vulkan_renderpass.h"
#include "renderer/backend/vulkan/details/descriptor.h"
#include "renderer/backend/vulkan/vulkan_pipeline.h"

typedef struct VulkanRenderer_t{
    VulkanContext context;
    SwapchainObj swapchain;
    CommandObj command;
    RenderPass* renderpasses;
    u8 renderpassesCount;
}VulkanRenderer;

void vulkan_renderer_init(RendererInitConfig config, RenderState* rState, PlatformState* pState){
    VulkanRenderer* r = (VulkanRenderer*)rState->ref;
    r->context.gpu = VK_NULL_HANDLE;
    r->command.currentFrame = 0;
    r->swapchain.framebufferResized = false;
    r->renderpassesCount = config.renderpassesCount;
    for(u8 i=0; i<r->renderpassesCount; i++){
        r->renderpasses[i].msaaSamples = config.renderpasses[i].msaaSamples;
    }
    VkSampleCountFlagBits maxMsaaSamples;
    createVulkanRenderingContext(&r->context, pState, &maxMsaaSamples);
    for(u8 i=0; i<r->renderpassesCount; i++){
        //FIXME: if i remember correctly setting up msaa samples count as 1 breaks up the renderpass creation
        r->renderpasses[i].msaaSamples = (r->renderpasses[i].msaaSamples > maxMsaaSamples) ? maxMsaaSamples : r->renderpasses[i].msaaSamples;
    }

    createVulkanSwapchainObj(&r->context, pState->display.width, pState->display.height, &r->swapchain);
    for(u8 i=0; i<r->renderpassesCount; i++){
        RenderPass* renderpass = &r->renderpasses[i];
        renderpass->pipelinesCount = config.renderpasses[i].pipelinesCount;
        createVulkanRenderPass(r->context.gpu, r->context.device, r->swapchain.imageFormat, r->renderpasses[i].msaaSamples, &r->renderpasses[i].ref);

        //pipelines
        for(u8 j=0; j < renderpass->pipelinesCount; j++){
            Pipeline* pipeline = &renderpass->pipelines[j];
            pipeline->descriptorSetsLayoutCount = config.renderpasses[i].pipelines[j].setsCount;

            for(u8 k=0; k < pipeline->descriptorSetsLayoutCount; k++){
                createVulkanDescriptorSetLayout(r->context.device, &config.renderpasses[i].pipelines[j].sets[k], &pipeline->descriptorSetsLayout[k]);
            }
            createVulkanDescriptorPool
            //TODO: optimizing the pipeline creation by creating all renderpass's subpasses by a single command
            createVulkanPipeline(
                r->context.device, 
                r->swapchain.extent, 
                renderpass->msaaSamples, 
                renderpass->ref, 
                pipeline, j,
                &config.renderpasses[i].pipelines[j]
            );
        }
    }
}

void vulkan_renderer_shutdown(RenderState* rState){
    VulkanRenderer* r = (VulkanRenderer*)rState->ref;
    for(u8 i=0; i< r->renderpassesCount; i++){
        RenderPass* rPass = &r->renderpasses[i];
        for(u8 j=0; j < rPass->pipelinesCount; j++){
            Pipeline* pipeline = &rPass->pipelines[j];
            for(u8 k=0; k < pipeline->descriptorSetsLayoutCount; k++){
                destroyVulkanDescriptorSetLayout(r->context.device, pipeline->descriptorSetsLayout[k]);
            }
            //destroy pipeline
            destroyVulkanPipeline(r->context.device, rPass->pipelines[j].ref, rPass->pipelines[j].layout);
        }
        destroyVulkanRenderPass(&r->context, rPass);
    }
    destroyVulkanSwapchainObj(&r->context, &r->swapchain);
    destroyVulkanRenderingContext(&r->context);
}

void vulkan_renderer_draw(
    Camera* camera, 
    RenderState* r, PlatformState* p, 
    MeshRenderer* meshRenderers, f64 deltatime,
    UI_Manager* uiManager
){

}