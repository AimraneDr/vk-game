#include "renderer/renderer.h"

#include <stdlib.h>
#include <string.h>

#include "data_types.h"
#include "meshTypes.h"
#include "core/debugger.h"
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

void renderer_createVulkanInstance(VkInstance *instance);
void renderer_initDebugMessanger(VkInstance *instance, VkDebugUtilsMessengerEXT *out);
VkDebugUtilsMessengerCreateInfoEXT getDebugMessangerCreateInfo();

void renderer_destroyDebugMessanger(VkInstance instance, VkDebugUtilsMessengerEXT messanger);

void recreateSwapChainObject(Renderer *r, u32 width, u32 height, WindowState visibility);

EVENT_CALLBACK(onWindowResize)
{
    ((Renderer *)listener)->framebufferResized = true;
}

void renderer_init(RendererInitConfig config, GameState* gState)
{
    Renderer* r = &gState->renderer;
    PlatformState* p = &gState->platform;
    r->gpu = VK_NULL_HANDLE;
    r->currentFrame = 0;
    r->framebufferResized = false;
    r->msaaSamples = config.msaaSamples;

    renderer_createVulkanInstance(&r->instance);
    renderer_initDebugMessanger(&r->instance, &r->debugMessanger);

#ifdef __linux__
    createSurface(r->instance, p->display.display, p->display.window, &r->surface);
#endif
#ifdef _WIN32
    createSurface(r->instance, &p->display.hInstance, &p->display.hwnd, &r->surface);
#endif

    selectPhysicalDevice(r->instance, r->surface, &r->gpu, &r->msaaSamples);
    createLogicalDevice(r->gpu, r->surface, &r->device, &r->queue.graphics, &r->queue.present);
    createSwapChain(r->gpu, r->device, r->surface, p->display.width, p->display.height, &r->swapchain, &r->swapchainImages, &r->swapchainImagesCount, &r->swapchainImageFormat, &r->swapchainExtent);

    r->swapchainImageViews = (VkImageView *)malloc(sizeof(VkImageView) * r->swapchainImagesCount);
    createSwapChainImageViews(r->device, r->swapchainImages, r->swapchainImagesCount, r->swapchainImageFormat, r->swapchainImageViews);

    createRenderPass(r->gpu, r->device, r->swapchainImageFormat, r->msaaSamples, &r->renderPass);
    
    createCommandPool(r->gpu, r->device, r->surface, &r->commandPool);

    createColorResources(r->gpu, r->device, r->commandPool, r->queue.graphics, r->swapchainExtent, r->msaaSamples, r->swapchainImageFormat, &r->attachments.color.image, &r->attachments.color.memory, &r->attachments.color.view);
    createDepthResources(r->gpu, r->device, r->commandPool, r->queue.graphics, r->swapchainExtent, r->msaaSamples, &r->attachments.depth.image, &r->attachments.depth.memory, &r->attachments.depth.view);
    createFramebuffers(r->device, r->renderPass, r->swapchainImageViews, r->attachments.color.view, r->attachments.depth.view, r->swapchainImagesCount, r->swapchainExtent, &r->swapchainFrameBuffers);

    createCommandBuffer(r->device, r->commandPool, r->commandBuffers);
    createSyncObjects(r->device, r->sync.imageAvailableSemaphores, r->sync.renderFinishedSemaphores, r->sync.inFlightFences);

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
    vkDeviceWaitIdle(r->device);

    destroySyncObjects(r->device, r->sync.imageAvailableSemaphores, r->sync.renderFinishedSemaphores, r->sync.inFlightFences);

    ecs_systems_destroy_group(gState, SYSTEM_GROUP_RENDERING);
    
    destroyCommandBuffer(r->device, r->commandPool, r->commandBuffers);
    destroyCommandPool(r->device, r->commandPool);
    destroyFramebuffers(r->device, r->swapchainFrameBuffers, r->swapchainImagesCount);
    destroyDepthResources(r->device, r->attachments.depth.image, r->attachments.depth.memory, r->attachments.depth.view);
    destroyColorResources(r->device, r->attachments.color.image, r->attachments.color.memory, r->attachments.color.view);

    destroyRenderPass(r->device, r->renderPass);
    destroySwapChainImageViews(r->device, r->swapchainImageViews, r->swapchainImagesCount);
    destroySwapChain(r->device, r->swapchain);
    destroyLogicalDevice(r->device);
    destroySurface(r->instance, r->surface);
    renderer_destroyDebugMessanger(r->instance, r->debugMessanger);
    vkDestroyInstance(r->instance, 0);
    return;
};

void renderer_draw(GameState* gState)
{
    Renderer* r = &gState->renderer;
    PlatformState* p = &gState->platform;

    vkWaitForFences(r->device, 1, &r->sync.inFlightFences[r->currentFrame], VK_TRUE, U64_MAX);
    
    u32 imageIndex;
    VkResult res = vkAcquireNextImageKHR(r->device, r->swapchain, U64_MAX, r->sync.imageAvailableSemaphores[r->currentFrame], VK_NULL_HANDLE, &imageIndex);
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
    vkResetFences(r->device, 1, &r->sync.inFlightFences[r->currentFrame]);
    vkResetCommandBuffer(r->commandBuffers[r->currentFrame], 0);

    recordCommandBuffer(gState, imageIndex);

    VkSemaphore waitSemaphores[] = {r->sync.imageAvailableSemaphores[r->currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSemaphore signalSemaphores[] = {r->sync.renderFinishedSemaphores[r->currentFrame]};

    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = waitSemaphores,
        .pWaitDstStageMask = waitStages,
        .commandBufferCount = 1,
        .pCommandBuffers = &r->commandBuffers[r->currentFrame],
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = signalSemaphores};

    if (vkQueueSubmit(r->queue.graphics, 1, &submitInfo, r->sync.inFlightFences[r->currentFrame]) != VK_SUCCESS)
    {
        LOG_FATAL("failed to submit draw command buffer!");
        return;
    }

    VkSwapchainKHR swapChains[] = {r->swapchain};

    VkPresentInfoKHR presentInfo = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = signalSemaphores,
        .swapchainCount = 1,
        .pSwapchains = swapChains,
        .pImageIndices = &imageIndex,
        .pResults = 0};

    res = vkQueuePresentKHR(r->queue.present, &presentInfo);
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

void renderer_createVulkanInstance(VkInstance *instance)
{
    VkApplicationInfo appInof = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "VK Game",
        .applicationVersion = VK_MAKE_VERSION(0, 0, 1),
        .engineVersion = VK_MAKE_VERSION(0, 0, 1),
        .pEngineName = "internal engine",
        .apiVersion = VK_API_VERSION_1_3};

    // FIXME: Horrible way to get the required extensions, but fix it later
    u32 ext_count = 0;
    vkEnumerateInstanceExtensionProperties(0, &ext_count, 0);

    VkExtensionProperties *extensions = malloc(sizeof(VkExtensionProperties) * ext_count);
    char **ext_names = (char **)malloc(sizeof(char *) * ext_count);
    vkEnumerateInstanceExtensionProperties(0, &ext_count, extensions);

    for (u32 i = 0; i < ext_count; i++)
    {
        u32 name_len = strlen(extensions[i].extensionName);
        ext_names[i] = (char *)malloc(sizeof(char) * (name_len + 1));
        memcpy(ext_names[i], extensions[i].extensionName, name_len);
        ext_names[i][name_len] = '\0';
        LOG_TRACE("required ext : %s", ext_names[i]);
    }

    VkDebugUtilsMessengerCreateInfoEXT debugMessangerInfo = getDebugMessangerCreateInfo();

    VkInstanceCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appInof,
        .enabledExtensionCount = ext_count,
        .ppEnabledExtensionNames = ext_names,
        .enabledLayerCount = isValidationLayersEnabled() ? validationLayersCount() : 0,
        .ppEnabledLayerNames = isValidationLayersEnabled() ? validationLayersNames() : 0,
        .pNext = isValidationLayersEnabled() ? &debugMessangerInfo : 0};

    VkResult res = vkCreateInstance(&info, 0, instance);

    free(extensions);
    for (u32 i = 0; i < ext_count; i++)
        free(ext_names[i]);
    free(ext_names);

    if (res != VK_SUCCESS)
    {
        LOG_FATAL("failed to create vulkan instance !");
        return;
    }
    return;
}

VkBool32 debugMessangerCallBack(
    VkDebugUtilsMessageSeverityFlagBitsEXT msgSeverity,
    VkDebugUtilsMessageTypeFlagsEXT msgType,
    const VkDebugUtilsMessengerCallbackDataEXT *callBackData_ptr,
    void *userData_ptr)
{
    switch (msgSeverity)
    {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        LOG_INFO("validation layer : %s\n", callBackData_ptr->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        LOG_WARN("validation layer : %s\n", callBackData_ptr->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        LOG_ERROR("validation layer : %s\n", callBackData_ptr->pMessage);
        break;
    default:
        // unknown
        LOG_DEBUG("validation layer : %s\n", callBackData_ptr->pMessage);
        break;
    }
    return VK_TRUE;
}

VkDebugUtilsMessengerCreateInfoEXT getDebugMessangerCreateInfo()
{
    return (VkDebugUtilsMessengerCreateInfoEXT){
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageType =
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .messageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT,
        .pfnUserCallback = debugMessangerCallBack,
        .pUserData = 0};
}

void renderer_initDebugMessanger(VkInstance *instance, VkDebugUtilsMessengerEXT *out)
{
    if (!isValidationLayersEnabled())
        return;

    VkDebugUtilsMessengerCreateInfoEXT info = getDebugMessangerCreateInfo();

    PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(*instance, "vkCreateDebugUtilsMessengerEXT");
    if (func == 0 || func(*instance, &info, 0, out) != VK_SUCCESS)
    {
        LOG_ERROR("faild to create vulkan debug messanger");
    }
    return;
}

void renderer_destroyDebugMessanger(VkInstance instance, VkDebugUtilsMessengerEXT messanger)
{
    if (!isValidationLayersEnabled())
        return;
    PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != 0)
    {
        func(instance, messanger, 0);
    }
}

void recreateSwapChainObject(Renderer *r, u32 width, u32 height, WindowState visibility)
{
    if (visibility == WINDOW_STATE_MINIMIZED)
    {
        return;
    }
    vkDeviceWaitIdle(r->device);

    destroyFramebuffers(r->device, r->swapchainFrameBuffers, r->swapchainImagesCount);
    destroyDepthResources(r->device, r->attachments.depth.image,r->attachments.depth.memory, r->attachments.depth.view);
    destroyColorResources(r->device, r->attachments.color.image,r->attachments.color.memory, r->attachments.color.view);
    destroySwapChainImageViews(r->device, r->swapchainImageViews, r->swapchainImagesCount);
    destroySwapChain(r->device, r->swapchain);

    createSwapChain(r->gpu, r->device, r->surface, width, height, &r->swapchain, &r->swapchainImages, &r->swapchainImagesCount, &r->swapchainImageFormat, &r->swapchainExtent);
    r->swapchainImageViews = (VkImageView *)malloc(sizeof(VkImageView) * r->swapchainImagesCount);
    createSwapChainImageViews(r->device, r->swapchainImages, r->swapchainImagesCount, r->swapchainImageFormat, r->swapchainImageViews);
    createColorResources(r->gpu, r->device, r->commandPool, r->queue.graphics, r->swapchainExtent, r->msaaSamples, r->swapchainImageFormat, &r->attachments.color.image, &r->attachments.color.memory, &r->attachments.color.view);
    createDepthResources(r->gpu, r->device, r->commandPool, r->queue.graphics, r->swapchainExtent, r->msaaSamples, &r->attachments.depth.image, &r->attachments.depth.memory, &r->attachments.depth.view);
    createFramebuffers(r->device, r->renderPass, r->swapchainImageViews, r->attachments.color.view, r->attachments.depth.view, r->swapchainImagesCount, r->swapchainExtent, &r->swapchainFrameBuffers);
}