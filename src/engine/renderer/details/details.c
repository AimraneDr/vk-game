#include "engine/renderer/details/details.h"
#include "engine/renderer/render_types.h"

#define DeviceExtensionsCount 1
const char* const* DeviceExtensionsNames[DeviceExtensionsCount] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#ifdef _DEBUG
    const bool enableValidationLayers = true;
    #define ValidationLayersCount 1
    const char* const* ValidationLayersNames[ValidationLayersCount] = {
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
char* const* validationLayersNames(){
    return ValidationLayersNames;
}

const u32 validationLayersCount(){
    return (u32)ValidationLayersCount;
}

char* const* deviceExtensionsNames(){
    return DeviceExtensionsNames;
}

const u32 deviceExtensionsCount(){
    return (u32)DeviceExtensionsCount;
}