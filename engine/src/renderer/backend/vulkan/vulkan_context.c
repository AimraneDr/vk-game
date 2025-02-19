#include "renderer/backend/vulkan/vulkan_context.h"

#include "core/debugger.h"
#include "renderer/backend/vulkan/details/utils.h"
#include "renderer/backend/vulkan/details/context/physicalDevice.h"
#include "renderer/backend/vulkan/details/context/logical_device.h"
#include "renderer/backend/vulkan/details/context/queue.h"
#include "renderer/backend/vulkan/details/context/surface.h"

void renderer_createVulkanInstance(VkInstance *instance);
void renderer_initDebugMessanger(VkInstance *instance, VkDebugUtilsMessengerEXT *out);
VkDebugUtilsMessengerCreateInfoEXT getDebugMessangerCreateInfo();

void renderer_destroyDebugMessanger(VkInstance instance, VkDebugUtilsMessengerEXT messanger);

void createVulkanRenderingContext(VulkanContext* c, PlatformState* p, VkSampleCountFlagBits* outMssaSamples){

    renderer_createVulkanInstance(&c->instance);
    renderer_initDebugMessanger(&c->instance, &c->debugMessanger);

#ifdef __linux__
    createSurface(c->instance, p->display.display, p->display.window, &c->surface);
#endif
#ifdef _WIN32
    createSurface(c->instance, &p->display.hInstance, &p->display.hwnd, &c->surface);
#endif

    selectPhysicalDevice(c->instance, c->surface, &c->gpu, outMssaSamples);
    createLogicalDevice(c->gpu, c->surface, &c->device, &c->queue.graphics, &c->queue.present);
}

void destroyVulkanRenderingContext(VulkanContext* c){
    destroyLogicalDevice(c->device);
    destroySurface(c->instance, c->surface);
    renderer_destroyDebugMessanger(c->instance, c->debugMessanger);
    vkDestroyInstance(c->instance, 0);
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