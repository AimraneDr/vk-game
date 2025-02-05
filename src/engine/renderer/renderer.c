#include "engine/renderer/renderer.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <stdint.h>
#include "engine/data_types.h"
#include "engine/meshTypes.h"
#include "engine/core/debugger.h"
#include "engine/renderer/details/queue.h"
#include "engine/renderer/details/details.h"
#include "engine/renderer/details/physicalDevice.h"
#include "engine/renderer/details/logical_device.h"
#include "engine/renderer/details/surface.h"
#include "engine/renderer/details/swapchain.h"
#include "engine/renderer/details/image_view.h"
#include "engine/renderer/details/renderpass.h"
#include "engine/renderer/details/descriptor.h"
#include "engine/renderer/details/pipeline.h"
#include "engine/renderer/details/framebuffer.h"
#include "engine/renderer/details/texture.h"
#include "engine/renderer/details/vertexBuffer.h"
#include "engine/renderer/details/indexBuffer.h"
#include "engine/renderer/details/uniformBuffer.h"
#include "engine/renderer/details/commandPool.h"
#include "engine/renderer/details/commandBuffer.h"
#include "engine/renderer/details/sync.h"
#include "engine/renderer/details/depth.h"
#include "engine/core/events.h"

Result renderer_createVulkanInstance(VkInstance *instance);
Result renderer_initDebugMessanger(VkInstance *instance, VkDebugUtilsMessengerEXT *out);
VkDebugUtilsMessengerCreateInfoEXT getDebugMessangerCreateInfo();

void renderer_destroyDebugMessanger(VkInstance instance, VkDebugUtilsMessengerEXT messanger);

void recreateSwapChainObject(Renderer *r, u32 width, u32 height, WindowState visibility);

void onWindowResize(EventType eType, void *sender, void *listener, EventContext context)
{
    ((Renderer *)listener)->framebufferResized = true;
    LOG_TRACE("Window resized handled by the renderer.");
}

u32 verticesCount = 8;
Vertex vertices[] = {
    //pos                //norm     //color         //texCoord
    {{-0.5f, 0.f, -0.5f}, {0}, {1.0f, .0f, .0f, 1.0f}, {1.f, 0.f}}, //left bottom
    {{0.5f, 0.f, -0.5f}, {0}, {0.f, 0.f, 1.0f, 1.f}, {0.f, 0.f}}, //right bottom
    {{0.5f, 0.f, 0.5f}, {0}, {0.f, 0.f, 1.f, 1.f}, {0.f, 1.f}}, //right top
    {{-0.5f, 0.f, 0.5f}, {0}, {1.0f, 0.f, 0.f, 1.f}, {1.f, 1.f}}, //left top

    {{-0.5f, 0.5f, -0.5f}, {0}, {1.0f, .0f, .0f, 1.0f}, {1.f, 0.f}}, //left bottom
    {{0.5f, 0.5f, -0.5f}, {0}, {0.f, 0.f, 1.0f, 1.f}, {0.f, 0.f}}, //right bottom
    {{0.5f, 0.5f, 0.5f}, {0}, {0.f, 0.f, 1.f, 1.f}, {0.f, 1.f}}, //right top
    {{-0.5f, 0.5f, 0.5f}, {0}, {1.0f, 0.f, 0.f, 1.f}, {1.f, 1.f}} //left top
};

u32 indicesCount = 12;
u16 indices[] = {
    4, 5, 6, 6, 7, 4,
    0, 1, 2, 2, 3, 0
};

Result renderer_init(Renderer *r, PlatformState *p)
{
    r->physicalDevice = VK_NULL_HANDLE;
    r->currentFrame = 0;
    r->framebufferResized = false;

    renderer_createVulkanInstance(&r->instance);
    renderer_initDebugMessanger(&r->instance, &r->debugMessanger);

#ifdef __linux__
    createSurface(r->instance, p->display.display, p->display.window, &r->surface);
#endif
#ifdef _WIN32
    createSurface(r->instance, &p->display.hInstance, &p->display.hwnd, &r->surface);
#endif

    selectPhysicalDevice(r->instance, r->surface, &r->physicalDevice);
    createLogicalDevice(r->physicalDevice, r->surface, &r->logicalDevice, &r->graphicsQueue, &r->presentQueue);
    createSwapChain(r->physicalDevice, r->logicalDevice, r->surface, p->display.width, p->display.height, &r->swapchain, &r->swapchainImages, &r->swapchainImagesCount, &r->swapchainImageFormat, &r->swapchainExtent);

    r->swapchainImageViews = (VkImageView *)malloc(sizeof(VkImageView) * r->swapchainImagesCount);
    createSwapChainImageViews(r->logicalDevice, r->swapchainImages, r->swapchainImagesCount, r->swapchainImageFormat, r->swapchainImageViews);

    createRenderPass(r->physicalDevice, r->logicalDevice, r->swapchainImageFormat, &r->renderPass);
    createDescriptorSetLayout(r->logicalDevice, &r->descriptorSetLayout);
    createPipeline(r->logicalDevice, "C:/Users/aimra/Dev/vk-game/bin/shaders/vert.spv", "C:/Users/aimra/Dev/vk-game/bin/shaders/frag.spv", r->swapchainExtent, r->renderPass, r->descriptorSetLayout, &r->pipelineLayout, &r->graphicsPipeline);

    createCommandPool(r->physicalDevice, r->logicalDevice, r->surface, &r->commandPool);

    createDepthResources(r->physicalDevice, r->logicalDevice, r->commandPool, r->graphicsQueue, r->swapchainExtent, &r->depthImage, &r->depthImageMemory, &r->depthImageView);
    createFramebuffers(r->logicalDevice, r->renderPass, r->swapchainImageViews, r->depthImageView, r->swapchainImagesCount, r->swapchainExtent, &r->shwapchainFrameBuffers);

    createTextureImage(r->physicalDevice, r->logicalDevice, r->commandPool, r->graphicsQueue, &r->textureImage, &r->textureImageMemory);
    createTextureImageView(r->logicalDevice, r->textureImage, &r->textureImageView);
    createTextureImageSampler(r->physicalDevice, r->logicalDevice, &r->textureSampler);

    createVertexBuffer(r->physicalDevice, r->logicalDevice, r->graphicsQueue, r->commandPool, verticesCount, vertices, &r->vertexBuffer, &r->vertexBufferMemory);
    createIndexBuffer(r->physicalDevice, r->logicalDevice, r->graphicsQueue, r->commandPool, indicesCount, indices, &r->indexBuffer, &r->indexBufferMemory);
    createUniformBuffer(r->physicalDevice,r->logicalDevice, r->uniformBuffers, r->uniformBuffersMemory, r->uniformBuffersMapped);
    
    createDescriptorPool(r->logicalDevice, &r->descriptorPool);
    createDescriptorSets(r->logicalDevice, r->descriptorSetLayout, r->descriptorPool, r->uniformBuffers, r->textureImageView, r->textureSampler, r->descriptorSets);

    createCommandBuffer(r->logicalDevice, r->commandPool, r->commandBuffers);
    createSyncObjects(r->logicalDevice, r->imageAvailableSemaphores, r->renderFinishedSemaphores, r->inFlightFences);

    // Subscribe to Events
    EventListener onResizeListener = {
        .callback = onWindowResize,
        .listener = r};
    subscribe_to_event(EVENT_TYPE_WINDOW_RESIZED, &onResizeListener);

    return RESULT_CODE_SUCCESS;
};

Result renderer_shutdown(Renderer *r)
{
    vkDeviceWaitIdle(r->logicalDevice);

    destroySyncObjects(r->logicalDevice, r->imageAvailableSemaphores, r->renderFinishedSemaphores, r->inFlightFences);
    destroyUniformBuffer(r->logicalDevice, r->uniformBuffers, r->uniformBuffersMemory);
    destroyTextureSampler(r->logicalDevice, r->textureSampler);
    destroyTextureImageView(r->logicalDevice, r->textureImageView);
    destroyTextureImage(r->logicalDevice, r->textureImage, r->textureImageMemory);
    destroyIndexBuffer(r->logicalDevice, r->indexBuffer, r->indexBufferMemory);
    destroyVertexBuffer(r->logicalDevice, r->vertexBuffer, r->vertexBufferMemory);
    destroyCommandBuffer(r->logicalDevice, r->commandPool, r->commandBuffers);
    destroyCommandPool(r->logicalDevice, r->commandPool);
    destroyFramebuffers(r->logicalDevice, r->shwapchainFrameBuffers, r->swapchainImagesCount);
    destroyDepthResources(r->logicalDevice, r->depthImage, r->depthImageMemory, r->depthImageView);
    destroyPipeline(r->logicalDevice, &r->graphicsPipeline, r->pipelineLayout);

    destroyDescriptorPool(r->logicalDevice, r->descriptorPool);
    destroyDescriptorSetLayout(r->logicalDevice, r->descriptorSetLayout);
    
    destroyRenderPass(r->logicalDevice, r->renderPass);
    destroySwapChainImageViews(r->logicalDevice, r->swapchainImageViews, r->swapchainImagesCount);
    destroySwapChain(r->logicalDevice, r->swapchain);
    destroyLogicalDevice(r->logicalDevice);
    destroySurface(r->instance, r->surface);
    renderer_destroyDebugMessanger(r->instance, r->debugMessanger);
    vkDestroyInstance(r->instance, 0);
    return RESULT_CODE_SUCCESS;
};

void renderer_draw(Renderer *r, PlatformState *p, f64 deltatime)
{
    vkWaitForFences(r->logicalDevice, 1, &r->inFlightFences[r->currentFrame], VK_TRUE, UINT64_MAX);

    u32 imageIndex;
    VkResult res = vkAcquireNextImageKHR(r->logicalDevice, r->swapchain, UINT64_MAX, r->imageAvailableSemaphores[r->currentFrame], VK_NULL_HANDLE, &imageIndex);
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
    vkResetFences(r->logicalDevice, 1, &r->inFlightFences[r->currentFrame]);
    vkResetCommandBuffer(r->commandBuffers[r->currentFrame], 0);


    updateUniformBuffer(r->currentFrame, (Vec2){p->display.width,p->display.height}, r->uniformBuffersMapped, deltatime);
    recordCommandBuffer(r->commandBuffers[r->currentFrame], r->graphicsPipeline, r->pipelineLayout, r->renderPass, r->shwapchainFrameBuffers, r->swapchainExtent, imageIndex, r->vertexBuffer, r->indexBuffer, indicesCount, &r->descriptorSets[r->currentFrame]);

    VkSemaphore waitSemaphores[] = {r->imageAvailableSemaphores[r->currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSemaphore signalSemaphores[] = {r->renderFinishedSemaphores[r->currentFrame]};

    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = waitSemaphores,
        .pWaitDstStageMask = waitStages,
        .commandBufferCount = 1,
        .pCommandBuffers = &r->commandBuffers[r->currentFrame],
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = signalSemaphores};

    if (vkQueueSubmit(r->graphicsQueue, 1, &submitInfo, r->inFlightFences[r->currentFrame]) != VK_SUCCESS)
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

    res = vkQueuePresentKHR(r->presentQueue, &presentInfo);
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

Result renderer_createVulkanInstance(VkInstance *instance)
{
    VkApplicationInfo appInof = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "VK Game",
        .applicationVersion = VK_MAKE_VERSION(0, 0, 1),
        .engineVersion = VK_MAKE_VERSION(0, 0, 1),
        .pEngineName = "internal engine",
        .apiVersion = VK_API_VERSION_1_3};

    // Horrible way to get the required extensions, but a fix is comming
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
        .enabledLayerCount = validationLayersCount(),
        .ppEnabledLayerNames = validationLayersNames(),
        .pNext = isValidationLayersEnabled() ? &debugMessangerInfo : 0};

    VkResult res = vkCreateInstance(&info, 0, instance);

    free(extensions);
    for (u32 i = 0; i < ext_count; i++)
        free(ext_names[i]);
    free(ext_names);

    if (res == VK_SUCCESS)
    {
        LOG_INFO("vulkan instance created successfully");
        return RESULT_CODE_SUCCESS;
    }

    return RESULT_CODE_FAILED_SYS_INIT;
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

Result renderer_initDebugMessanger(VkInstance *instance, VkDebugUtilsMessengerEXT *out)
{
    if (!isValidationLayersEnabled())
        return RESULT_CODE_SUCCESS;

    VkDebugUtilsMessengerCreateInfoEXT info = getDebugMessangerCreateInfo();

    PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(*instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != 0)
    {
        if (func(*instance, &info, 0, out) == VK_SUCCESS)
        {
            return RESULT_CODE_SUCCESS;
        }
    }
    return RESULT_CODE_FAILED_DEBUG_MESSANGER_CREATION;
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
    if (visibility == Minimized)
    {
        return;
    }
    vkDeviceWaitIdle(r->logicalDevice);

    destroyFramebuffers(r->logicalDevice, r->shwapchainFrameBuffers, r->swapchainImagesCount);
    destroySwapChainImageViews(r->logicalDevice, r->swapchainImageViews, r->swapchainImagesCount);
    destroySwapChain(r->logicalDevice, r->swapchain);

    createSwapChain(r->physicalDevice, r->logicalDevice, r->surface, width, height, &r->swapchain, &r->swapchainImages, &r->swapchainImagesCount, &r->swapchainImageFormat, &r->swapchainExtent);
    r->swapchainImageViews = (VkImageView *)malloc(sizeof(VkImageView) * r->swapchainImagesCount);
    createSwapChainImageViews(r->logicalDevice, r->swapchainImages, r->swapchainImagesCount, r->swapchainImageFormat, r->swapchainImageViews);
    createDepthResources(r->physicalDevice, r->logicalDevice, r->commandPool, r->graphicsQueue, r->swapchainExtent, &r->depthImage, &r->depthImageMemory, &r->depthImageView);
    createFramebuffers(r->logicalDevice, r->renderPass, r->swapchainImageViews, r->depthImageView, r->swapchainImagesCount, r->swapchainExtent, &r->shwapchainFrameBuffers);
}
