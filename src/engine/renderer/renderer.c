#include "engine/renderer/renderer.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "engine/data_types.h"
#include "engine/renderer/details/queue.h"
#include "engine/renderer/details/details.h"
#include "engine/renderer/logical_device.h"
#include "engine/renderer/details/surface.h"
#include "engine/renderer/details/swapchain.h"




Result renderer_createVulkanInstance(VkInstance* instance);
Result renderer_initDebugMessanger(VkInstance* instance, VkDebugUtilsMessengerEXT* out);
VkDebugUtilsMessengerCreateInfoEXT getDebugMessangerCreateInfo();

void renderer_destroyDebugMessanger(VkInstance instance, VkDebugUtilsMessengerEXT messanger);
Result selectPhysicalDevice(const VkInstance instance, const VkSurfaceKHR surface, VkPhysicalDevice* out);
bool checkDeviceExtensionsSupport(VkPhysicalDevice device);

Result renderer_init(Renderer* r, PlatformState* p){
    r->physicalDevice = VK_NULL_HANDLE;

    renderer_createVulkanInstance(&r->instance);
    renderer_initDebugMessanger(&r->instance, &r->debugMessanger);
    // createSurface(r->instance, p->display.display, p->display.window, &r->surface);
    #ifdef __linux__
    createSurface(r->instance, p->display.display, p->display.window, &r->surface);
    #endif
    #ifdef _WIN32
    createSurface(r->instance, p->display.hInstance, p->display.hwnd, &r->surface);
    #endif
    
    selectPhysicalDevice(r->instance, r->surface, &r->physicalDevice);
    createLogicalDevice(r->physicalDevice, r->surface, &r->logicalDevice, &r->graphicsQueue, &r->presentQueue);
    createSwapChain(r->physicalDevice, r->logicalDevice, r->surface, p->display.width, p->display.height, &r->swapchain, &r->swapchainImages, &r->swapchainImageFormat, &r->swapchainExtent);

    return RESULT_CODE_SUCCESS;
};

Result renderer_shutdown(Renderer* r){
    destroySwapChain(r->logicalDevice, r->swapchain);
    destroyLogicalDevice(r->logicalDevice);
    destroySurface(r->instance, r->surface);
    renderer_destroyDebugMessanger(r->instance, r->debugMessanger);
    vkDestroyInstance(r->instance, 0);
    return RESULT_CODE_SUCCESS;
};

Result renderer_createVulkanInstance(VkInstance* instance){
    VkApplicationInfo appInof = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "VK Game",
        .applicationVersion = VK_MAKE_VERSION(0, 0, 1),
        .engineVersion = VK_MAKE_VERSION(0, 0, 1),
        .pEngineName = "internal engine",
        .apiVersion = VK_API_VERSION_1_3
    };
    

    // Horrible way to get the required extensions, but a fix is comming
    u32 ext_count = 0;
    vkEnumerateInstanceExtensionProperties(0, &ext_count, 0);

    VkExtensionProperties* extensions = malloc(sizeof(VkExtensionProperties) * ext_count);
    char** ext_names = (char**)malloc(sizeof(char*) * ext_count);
    vkEnumerateInstanceExtensionProperties(0, &ext_count, extensions);

    for(u32 i = 0; i < ext_count; i++){
        u32 name_len = strlen(extensions[i].extensionName);
        ext_names[i] = (char*)malloc(sizeof(char) * name_len); 
        strcpy(ext_names[i], extensions[i].extensionName);
        printf("required ext : %s\n", ext_names[i]);
    }

    VkDebugUtilsMessengerCreateInfoEXT debugMessangerInfo = getDebugMessangerCreateInfo();

    VkInstanceCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appInof,
        .enabledExtensionCount = ext_count,
        .ppEnabledExtensionNames = ext_names,
        .enabledLayerCount = validationLayersCount(),
        .ppEnabledLayerNames = validationLayersNames(),
        .pNext = isValidationLayersEnabled() ? &debugMessangerInfo : 0
    };

    VkResult res = vkCreateInstance(&info, 0, instance);

    free(extensions);
    for(u32 i=0; i<ext_count; i++) free(ext_names[i]);
    free(ext_names);

    if(res == VK_SUCCESS){
        printf("vulkan instance created successfully\n");
        return RESULT_CODE_SUCCESS;
    }

    return RESULT_CODE_FAILED_SYS_INIT;
}

VkBool32 debugMessangerCallBack(
    VkDebugUtilsMessageSeverityFlagBitsEXT msgSeverity,
    VkDebugUtilsMessageTypeFlagsEXT msgType,
    const VkDebugUtilsMessengerCallbackDataEXT* callBackData_ptr,
    void* userData_ptr
){
    printf("validation layer : %s\n", callBackData_ptr->pMessage);
    return VK_FALSE;
}

VkDebugUtilsMessengerCreateInfoEXT getDebugMessangerCreateInfo(){
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

Result renderer_initDebugMessanger(VkInstance* instance, VkDebugUtilsMessengerEXT* out){
    if(!isValidationLayersEnabled()) return RESULT_CODE_SUCCESS;

    VkDebugUtilsMessengerCreateInfoEXT info = getDebugMessangerCreateInfo();

    PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(*instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != 0) {
        if(func(*instance, &info, 0, out) == VK_SUCCESS){
            return RESULT_CODE_SUCCESS;
        }
    }
    return RESULT_CODE_FAILED_DEBUG_MESSANGER_CREATION;
}

void renderer_destroyDebugMessanger(VkInstance instance, VkDebugUtilsMessengerEXT messanger){
    if(!isValidationLayersEnabled()) return;
    PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != 0) {
        func(instance, messanger, 0);
    }
}

/// @brief for now jsut return the first suitable device
/// @param device 
/// @return 
bool evaluatePhysicalDevice(const VkPhysicalDevice device, const VkSurfaceKHR surface){
    
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceProperties(device, &properties);
    vkGetPhysicalDeviceFeatures(device, &features);

    bool suitable = true;
    // device type
    if(properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU){
        suitable = false;
    }
    
    //support geometry shader
    if(!features.geometryShader) suitable = false;

    QueueFamilyIndices indices = findQueueFamilies(device, surface);
    if(indices.graphicsFamily == -1 || indices.computeFamily == -1)  suitable = false; 

    bool extensionsSuppoerted = checkDeviceExtensionsSupport(device);

    bool swapChainAdequate = false;
    if(extensionsSuppoerted){
        SwapChainSupportDetails d = {}; 
        querySwapChainSupport(device, surface, &d);
        if(d.formatsCount > 0 && d.presentModesCount > 0) swapChainAdequate = true;
        // printf("swapchain surface formats support count : %d\n", d.formatsCount);
        // printf("swapchain surface present modes support count : %d\n", d.presentModesCount);
        freeSwapChainSupportDetails(&d);
    }
    return suitable && extensionsSuppoerted && swapChainAdequate;
}


Result selectPhysicalDevice(const VkInstance instance, const VkSurfaceKHR surface, VkPhysicalDevice* out){
    u32 devicesCount = 0;
    vkEnumeratePhysicalDevices(instance, &devicesCount, 0);

    if(devicesCount == 0){
        printf("error : can not run the game since no gpu is present.\n");
        return RESULT_CODE_NO_GPU_PRESENT;
    }

    VkPhysicalDevice* devices = malloc(sizeof(VkPhysicalDevice) * devicesCount);

    vkEnumeratePhysicalDevices(instance, &devicesCount, devices);

    for(u32 i=0;i < devicesCount; i++){
        if(evaluatePhysicalDevice(devices[i], surface)){
            *out = devices[i];
            break;
        }
    }

    if(out == VK_NULL_HANDLE){
        printf("error : the present gpu does not meet the minimum requirements to run the game.\n");
        return RESULT_CODE_GPU_NOT_SUITABLE;
    }
    
    return RESULT_CODE_SUCCESS;
}

bool checkDeviceExtensionsSupport(VkPhysicalDevice device){
    u32 ext_count = 0;
    vkEnumerateDeviceExtensionProperties(device, 0, &ext_count, 0);

    VkExtensionProperties* ext_props = (VkExtensionProperties*)malloc(sizeof(VkExtensionProperties) * ext_count);

    vkEnumerateDeviceExtensionProperties(device, 0, &ext_count, ext_props);
    for(u32 i=0; i < deviceExtensionsCount(); i++){
        char* ext_name = deviceExtensionsNames()[i];
        bool found = false;
        for(u32 j=0; j < ext_count; j++){
            if(strcmp(ext_name, ext_props[j].extensionName) == 0){
                found = true;
                printf("ext name : '%s' is supported.\n", ext_name);
            }
        }
        if(!found){
            printf("ext name '%s' not supported.\n", ext_name);
            return false;
        }
    }
    free(ext_props);
    
    return true;
}