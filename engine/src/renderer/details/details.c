#include "renderer/details/details.h"
#include "renderer/render_types.h"

#define DeviceExtensionsCount 2
const char* DeviceExtensionsNames[DeviceExtensionsCount] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME 
};

#ifdef _DEBUG
    const bool enableValidationLayers = true;
    #define ValidationLayersCount 1
    const char* const ValidationLayersNames[ValidationLayersCount] = {
        "VK_LAYER_KHRONOS_validation"
    };
#else
    const bool enableValidationLayers = false;
    #define ValidationLayersCount 0
    #define ValidationLayersNames 0
#endif

bool isValidationLayersEnabled(){
    return enableValidationLayers;
}
const char* const* validationLayersNames(){
    return ValidationLayersNames;
}

const u32 validationLayersCount(){
    return (u32)ValidationLayersCount;
}

const char** deviceExtensionsNames(){
    return DeviceExtensionsNames;
}

const u32 deviceExtensionsCount(){
    return (u32)DeviceExtensionsCount;
}