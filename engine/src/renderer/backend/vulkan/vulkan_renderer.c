#include "renderer/backend/vulkan/vulkan_renderer.h"

#include "renderer/backend/vulkan/vulkan_context.h"
#include "renderer/backend/vulkan/vulkan_swapchain.h"
#include "renderer/backend/vulkan/vulkan_renderpass.h"
#include "renderer/backend/vulkan/details/descriptor.h"
#include "renderer/backend/vulkan/vulkan_pipeline.h"
#include "renderer/backend/vulkan/vulkan_command.h"
#include "renderer/backend/vulkan/details/resources/color.h"
#include "renderer/backend/vulkan/details/resources/framebuffer.h"
#include "renderer/backend/vulkan/details/resources/depth.h"
#include "renderer/backend/vulkan/details/resources/uniformBuffer.h"
#include "renderer/backend/vulkan/details/resources/texture.h"
#include "renderer/backend/vulkan/details/sync.h"

#include "core/debugger.h"


typedef struct VulkanRenderer_t{
    VulkanContext context;
    SwapchainObj swapchain;
    RenderPass* renderpasses;
    u8 renderpassesCount;
    CommandObj command;
}VulkanRenderer;

void recreateSwapChainObject(VulkanRenderer *r, u32 newWidth, u32 newHeight, WindowState visibility);


void vulkan_renderer_init(RendererInitConfig config, RenderState* rState, PlatformState* pState){
    rState->ref = malloc(sizeof(VulkanRenderer));
    VulkanRenderer* r = (VulkanRenderer*)rState->ref;
    r->context.gpu = VK_NULL_HANDLE;
    r->command.currentFrame = 0;
    r->swapchain.framebufferResized = false;
    r->renderpassesCount = config.renderpassesCount;
    r->renderpasses = (RenderPass*)malloc(sizeof(RenderPass) * r->renderpassesCount);
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
    createVulkanCommandPool(r->context.gpu, r->context.device, r->context.surface, &r->command.pool);
    createVulkanCommandBuffer(r->context.device, r->command.pool, r->command.buffers);
    for(u8 i=0; i < r->renderpassesCount; i++){
        RenderPass* renderpass = &r->renderpasses[i];
        renderpass->pipelinesCount = config.renderpasses[i].pipelinesCount;
        renderpass->pipelines = (Pipeline*)malloc(sizeof(Pipeline) * renderpass->pipelinesCount);
        createVulkanRenderPass(r->context.gpu, r->context.device, r->swapchain.imageFormat, &config.renderpasses[i], &r->renderpasses[i].ref);


        //pipelines
        for(u8 j=0; j < renderpass->pipelinesCount; j++){
            Pipeline* pipeline = &renderpass->pipelines[j];
            pipeline->descriptorSetsLayoutCount = config.renderpasses[i].pipelines[j].setsCount;
            pipeline->descriptorSetsLayout = (VkDescriptorSetLayout*)malloc(sizeof(VkDescriptorSetLayout) * pipeline->descriptorSetsLayoutCount);
            
            createVulkanDescriptorPool(
                r->context.device, 
                config.renderpasses[i].pipelines[j].sets, 
                config.renderpasses[i].pipelines[j].setsCount, 
                &pipeline->descriptorPool);
            
            for(u8 k=0; k < pipeline->descriptorSetsLayoutCount; k++){
                createVulkanDescriptorSetLayout(r->context.device, &config.renderpasses[i].pipelines[j].sets[k], &pipeline->descriptorSetsLayout[k]);
                createVulkanUniformBuffers(
                    r->context.gpu,
                    r->context.device,
                    &config.renderpasses[i].pipelines[j].sets[k],
                    pipeline->frameResources
                );
                createVulkanTextures(
                    &r->context, 
                    r->command.pool, 
                    &config.renderpasses[i].pipelines[j].sets[k],
                    &pipeline->textures,
                    &pipeline->texturesCount);

                createVulkanDescriptorSets(
                    r->context.device,
                    &config.renderpasses[i].pipelines[j],
                    pipeline
                );
            }

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

        createVulkanColorResources(
            r->context.gpu, r->context.device, 
            r->command.pool, r->context.queue.graphics,
            r->swapchain.extent, renderpass->msaaSamples,
            r->swapchain.imageFormat, 
            &renderpass->attachments.color.image, 
            &renderpass->attachments.color.memory,
            &renderpass->attachments.color.view);

        createVulkanDepthResources(
            r->context.gpu, r->context.device, 
            r->command.pool, r->context.queue.graphics,
            r->swapchain.extent, renderpass->msaaSamples,
            &renderpass->attachments.depth.image, 
            &renderpass->attachments.depth.memory,
            &renderpass->attachments.depth.view);

        createVulkanFramebuffers(
            r->context.device,
            renderpass->ref,
            r->swapchain.imageViews,
            renderpass->attachments.color.view,
            renderpass->attachments.depth.view,
            r->swapchain.imagesCount,
            r->swapchain.extent,
            &(r->swapchain.frameBuffers)
        );

        
    }
    createVulkanSyncObjects(r->context.device, r->command.syncs);
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
    free(r);
}

void vulkan_renderer_draw(
    Camera* camera, 
    RenderState* rState, PlatformState* pState, 
    MeshRenderer* meshRenderers, f64 deltatime,
    UI_Manager* uiManager
){
    VulkanRenderer* r = rState->ref;
    vkWaitForFences(r->context.device, 1, &r->command.syncs[r->command.currentFrame]->inFlightFence, VK_TRUE, U64_MAX);

    u8 imageIndx;
    VkResult res = vkAcquireNextImageKHR(r->context.device, r->swapchain.ref, U64_MAX, r->command.syncs[r->command.currentFrame]->imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndx);

    if (res == VK_ERROR_OUT_OF_DATE_KHR)
    {
        recreateSwapChainObject(r, pState->display.width, pState->display.height, pState->display.visibility);
        return;
    }
    else if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR)
    {
        LOG_FATAL("failed to acquire swap chain image!");
        return;
    }

    vkResetFences(r->context.device, 1, &r->command.syncs[r->command.currentFrame]->inFlightFence);
    vkResetCommandBuffer(r->command.buffers[imageIndx], 0);

    
}

void recreateSwapChainObject(VulkanRenderer *r, u32 newWidth, u32 newHeight, WindowState visibility)
{}