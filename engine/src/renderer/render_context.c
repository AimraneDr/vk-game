#include "renderer/render_context.h"

#include "core/debugger.h"
#include "renderer/details/queue.h"
#include "renderer/details/details.h"
#include "renderer/details/physicalDevice.h"
#include "renderer/details/logical_device.h"
#include "renderer/details/surface.h"
#include "renderer/details/swapchain.h"
#include "renderer/details/image_view.h"
#include "renderer/details/commandPool.h"
#include "renderer/details/commandBuffer.h"


void renderer_createVulkanInstance(VkInstance *instance);
void renderer_initDebugMessanger(VkInstance *instance, VkDebugUtilsMessengerEXT *out);
static VkValidationFeaturesEXT getValidationFeatures();
static VkDebugUtilsMessengerCreateInfoEXT getDebugMessangerCreateInfo();
void renderer_destroyDebugMessanger(VkInstance instance, VkDebugUtilsMessengerEXT messanger);

static RendererContext* context = 0;
static bool newFrame;

void renderContext_signalNewFrameForDebugger(){
    newFrame = true;
}

RendererContext *getRendererContext()
{
    return context;
}

RendererContext* createRendererContext(PlatformState* pState)
{
    context = malloc(sizeof(RendererContext));
    context->gpu = VK_NULL_HANDLE;
    renderer_createVulkanInstance(&context->instance);
    renderer_initDebugMessanger(&context->instance, &context->debugMessanger);

#ifdef __linux__
    createSurface(context->instance, pState->display.display, pState->display.window, &context->surface);
#endif
#ifdef _WIN32
    createSurface(context->instance, &pState->display.hInstance, &pState->display.hwnd, &context->surface);
#endif

    selectPhysicalDevice(context->instance, context->surface, &context->gpu, &context->msaaSamples);
    createLogicalDevice(context->gpu, context->surface, &context->device, &context->queue.graphics, &context->queue.present);
    createSwapChain(context->gpu, context->device, context->surface, pState->display.width, pState->display.height, &context->swapchain, &context->swapchainImages, &context->swapchainImagesCount, &context->swapchainImageFormat, &context->swapchainExtent);

    context->swapchainImageViews = (VkImageView *)malloc(sizeof(VkImageView) * context->swapchainImagesCount);
    createSwapChainImageViews(context->device, context->swapchainImages, context->swapchainImagesCount, context->swapchainImageFormat, context->swapchainImageViews);

    createCommandPool(context->gpu, context->device, context->surface, &context->commandPool);
    createCommandBuffer(context->device, context->commandPool, context->commandBuffers);
    return context;
}

void destroyRendererContext()
{
    destroyCommandBuffer(context->device, context->commandPool, context->commandBuffers);
    destroyCommandPool(context->device, context->commandPool);
    destroySwapChainImageViews(context->device, context->swapchainImageViews, context->swapchainImagesCount);
    destroySwapChain(context->device, context->swapchain);
    destroyLogicalDevice(context->device);
    destroySurface(context->instance, context->surface);
    renderer_destroyDebugMessanger(context->instance, context->debugMessanger);
    vkDestroyInstance(context->instance, 0);
}

/////////////////////////////////
/////////////////////////////////
/////       INTERNALS       /////
/////////////////////////////////
/////////////////////////////////

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
    char **ext_names = (char **)malloc(sizeof(char *) * (ext_count));
    vkEnumerateInstanceExtensionProperties(0, &ext_count, extensions);

    for (u32 i = 0; i < ext_count; i++)
    {
        u32 name_len = strlen(extensions[i].extensionName);
        ext_names[i] = (char *)malloc(sizeof(char) * (name_len + 1));
        memcpy(ext_names[i], extensions[i].extensionName, name_len);
        ext_names[i][name_len] = '\0';
        LOG_TRACE("required ext : %s", ext_names[i]);
    }

    //TODO: Enable GPU asssisted validation features
    VkDebugUtilsMessengerCreateInfoEXT debugMessangerInfo = getDebugMessangerCreateInfo();
    VkInstanceCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appInof,
        .enabledExtensionCount = ext_count,
        .ppEnabledExtensionNames = (const char *const *)ext_names,
        .enabledLayerCount = isValidationLayersEnabled() ? validationLayersCount() : 0,
        .ppEnabledLayerNames = isValidationLayersEnabled() ? (const char *const *)validationLayersNames() : 0,
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
    if(newFrame){
        LOG_TRACE("new frame");
        newFrame = false;
    }

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

static VkValidationFeaturesEXT getValidationFeatures(){
    static VkValidationFeatureEnableEXT enables[] = {
        VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT,
        VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT // optional
    };
    
    static VkValidationFeaturesEXT validationFeatures = {
        .sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT,
        .enabledValidationFeatureCount = sizeof(enables) / sizeof(VkValidationFeatureEnableEXT),
        .pEnabledValidationFeatures = enables,
        .disabledValidationFeatureCount = 0,
        .pDisabledValidationFeatures = 0
    };

    return validationFeatures;
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
        .pUserData = 0
    };
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
