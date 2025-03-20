#include "renderer/renderer.h"

#include <stdlib.h>
#include <string.h>

#include "data_types.h"
#include "meshTypes.h"
#include "core/debugger.h"
#include "renderer/render_context.h"
#include "renderer/details/queue.h"
#include "renderer/details/details.h"
#include "renderer/details/physicalDevice.h"
#include "renderer/details/logical_device.h"
#include "renderer/details/surface.h"
#include "renderer/details/swapchain.h"
#include "renderer/details/image_view.h"
#include "renderer/details/renderpass.h"
#include "renderer/details/descriptor.h"
#include "renderer/details/pipeline.h"
#include "renderer/details/framebuffer.h"
#include "renderer/details/texture.h"
#include "renderer/details/vertexBuffer.h"
#include "renderer/details/indexBuffer.h"
#include "renderer/details/uniformBuffer.h"
#include "renderer/details/commandPool.h"
#include "renderer/details/commandBuffer.h"
#include "renderer/details/sync.h"
#include "renderer/details/color.h"
#include "renderer/details/depth.h"
#include "core/events.h"

#include "core/clock.h"

#include <collections/DynamicArray.h>

#include "ecs/ecs.h"

void recreateSwapChainObject(Renderer *r, u32 width, u32 height, WindowState visibility);

EVENT_CALLBACK(onWindowResize)
{
    ((Renderer *)listener)->framebufferResized = true;
}

void renderer_init(RendererInitConfig config, GameState* gState)
{
    Renderer* r = &gState->renderer;
    PlatformState* p = &gState->platform;
    r->currentFrame = 0;
    r->framebufferResized = false;

    RendererContext * c = createRendererContext(p);
    r->context = c;
    c->msaaSamples = c->msaaSamples > config.msaaSamples ? config.msaaSamples : c->msaaSamples;

    createRenderPass(c->gpu, c->device, c->swapchainImageFormat, c->msaaSamples, &r->renderPass);
    

    createColorResources(c->gpu, c->device, c->commandPool, c->queue.graphics, c->swapchainExtent, c->msaaSamples, c->swapchainImageFormat, &c->color.image, &c->color.memory, &c->color.view);
    createDepthResources(c->gpu, c->device, c->commandPool, c->queue.graphics, c->swapchainExtent, c->msaaSamples, &c->depth.image, &c->depth.memory, &c->depth.view);
    createFramebuffers(c->device, r->renderPass, c->swapchainImageViews, c->color.view, c->depth.view, c->swapchainImagesCount, c->swapchainExtent, &c->swapchainFrameBuffers);

    createSyncObjects(c->device, r->sync.imageAvailableSemaphores, r->sync.renderFinishedSemaphores, r->sync.inFlightFences);

    //start all rendering systems
    ecs_systems_start_group(gState, SYSTEM_GROUP_RENDERING);

    // Subscribe to Events
    EventListener onResizeListener = {
        .callback = onWindowResize,
        .listener = r};
    subscribe_to_event(EVENT_TYPE_WINDOW_RESIZED, &onResizeListener);

    return;
};

void renderer_shutdown(GameState* gState)
{    
    Renderer *r = &gState->renderer;
    vkDeviceWaitIdle(r->context->device);

    destroySyncObjects(r->context->device, r->sync.imageAvailableSemaphores, r->sync.renderFinishedSemaphores, r->sync.inFlightFences);

    ecs_systems_destroy_group(gState, SYSTEM_GROUP_RENDERING);
    
    destroyFramebuffers(r->context->device, r->context->swapchainFrameBuffers, r->context->swapchainImagesCount);
    destroyDepthResources(r->context->device, r->context->depth.image, r->context->depth.memory, r->context->depth.view);
    destroyColorResources(r->context->device, r->context->color.image, r->context->color.memory, r->context->color.view);

    destroyRenderPass(r->context->device, r->renderPass);
    destroyRendererContext();
    return;
};

void renderer_draw(GameState* gState)
{
    if(gState->platform.display.shouldClose) return;
    Renderer* r = &gState->renderer;
    PlatformState* p = &gState->platform;

    vkWaitForFences(r->context->device, 1, &r->sync.inFlightFences[r->currentFrame], VK_TRUE, U64_MAX);
    
    u32 imageIndex;
    VkResult res = vkAcquireNextImageKHR(r->context->device, r->context->swapchain, U64_MAX, r->sync.imageAvailableSemaphores[r->currentFrame], VK_NULL_HANDLE, &imageIndex);
    if (res == VK_ERROR_OUT_OF_DATE_KHR)
    {
        recreateSwapChainObject(r, p->display.width, p->display.height, p->display.visibility);
        return;
    }
    else if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR)
    {
        LOG_FATAL("failed to acquire swap chain image!");
        return;
    }
    vkResetFences(r->context->device, 1, &r->sync.inFlightFences[r->currentFrame]);
    vkResetCommandBuffer(r->context->commandBuffers[r->currentFrame], 0);

    ecs_systems_pre_update_group(gState, SYSTEM_GROUP_RENDERING);
    
    recordCommandBuffer(gState, imageIndex);

    ecs_systems_post_update_group(gState, SYSTEM_GROUP_RENDERING);
    
    VkSemaphore waitSemaphores[] = {r->sync.imageAvailableSemaphores[r->currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSemaphore signalSemaphores[] = {r->sync.renderFinishedSemaphores[r->currentFrame]};

    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = waitSemaphores,
        .pWaitDstStageMask = waitStages,
        .commandBufferCount = 1,
        .pCommandBuffers = &r->context->commandBuffers[r->currentFrame],
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = signalSemaphores};

    if (vkQueueSubmit(r->context->queue.graphics, 1, &submitInfo, r->sync.inFlightFences[r->currentFrame]) != VK_SUCCESS)
    {
        LOG_FATAL("failed to submit draw command buffer!");
        return;
    }

    VkSwapchainKHR swapChains[] = {r->context->swapchain};

    VkPresentInfoKHR presentInfo = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = signalSemaphores,
        .swapchainCount = 1,
        .pSwapchains = swapChains,
        .pImageIndices = &imageIndex,
        .pResults = 0};

    res = vkQueuePresentKHR(r->context->queue.present, &presentInfo);
    if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR || r->framebufferResized)
    {
        r->framebufferResized = false;
        recreateSwapChainObject(r, p->display.width, p->display.height, p->display.visibility);
    }
    else if (res != VK_SUCCESS)
    {
        LOG_FATAL("failed to present swap chain image!");
        return;
    }
    r->currentFrame = (r->currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void recreateSwapChainObject(Renderer *r, u32 width, u32 height, WindowState visibility)
{
    if (visibility == WINDOW_STATE_MINIMIZED) return;

    vkDeviceWaitIdle(r->context->device);

    //TODO: this is all related to the swapchain, so it makes more sense to group it in a swapchain object
    destroyFramebuffers(r->context->device, r->context->swapchainFrameBuffers, r->context->swapchainImagesCount);
    destroyDepthResources(r->context->device, r->context->depth.image,r->context->depth.memory, r->context->depth.view);
    destroyColorResources(r->context->device, r->context->color.image,r->context->color.memory, r->context->color.view);
    destroySwapChainImageViews(r->context->device, r->context->swapchainImageViews, r->context->swapchainImagesCount);
    destroySwapChain(r->context->device, r->context->swapchain);

    createSwapChain(r->context->gpu, r->context->device, r->context->surface, width, height, &r->context->swapchain, &r->context->swapchainImages, &r->context->swapchainImagesCount, &r->context->swapchainImageFormat, &r->context->swapchainExtent);
    r->context->swapchainImageViews = (VkImageView *)malloc(sizeof(VkImageView) * r->context->swapchainImagesCount);
    createSwapChainImageViews(r->context->device, r->context->swapchainImages, r->context->swapchainImagesCount, r->context->swapchainImageFormat, r->context->swapchainImageViews);
    createColorResources(r->context->gpu, r->context->device, r->context->commandPool, r->context->queue.graphics, r->context->swapchainExtent, r->context->msaaSamples, r->context->swapchainImageFormat, &r->context->color.image, &r->context->color.memory, &r->context->color.view);
    createDepthResources(r->context->gpu, r->context->device, r->context->commandPool, r->context->queue.graphics, r->context->swapchainExtent, r->context->msaaSamples, &r->context->depth.image, &r->context->depth.memory, &r->context->depth.view);
    createFramebuffers(r->context->device, r->renderPass, r->context->swapchainImageViews, r->context->color.view, r->context->depth.view, r->context->swapchainImagesCount, r->context->swapchainExtent, &r->context->swapchainFrameBuffers);
}