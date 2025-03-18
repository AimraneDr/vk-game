#include "renderer/details/physicalDevice.h"

#include "core/debugger.h"
#include "renderer/details/details.h"
#include "renderer/details/queue.h"
#include "renderer/details/swapchain.h"

bool evaluatePhysicalDevice(const VkPhysicalDevice device, const VkSurfaceKHR surface);

void selectPhysicalDevice(const VkInstance instance, const VkSurfaceKHR surface, VkPhysicalDevice *out, VkSampleCountFlagBits *outMsaaSamples)
{
    u32 devicesCount = 0;
    vkEnumeratePhysicalDevices(instance, &devicesCount, 0);

    if (devicesCount == 0)
    {
        LOG_FATAL("A physical device is required to start the application.");
        return;
    }

    VkPhysicalDevice *gpus = malloc(sizeof(VkPhysicalDevice) * devicesCount);

    vkEnumeratePhysicalDevices(instance, &devicesCount, gpus);

    for (u32 i = 0; i < devicesCount; i++)
    {
        if (evaluatePhysicalDevice(gpus[i], surface))
        {
            *out = gpus[i];
            if (*outMsaaSamples == 0)
            {
                *outMsaaSamples = getMaxUsableSampleCount(gpus[i]);
            }
            else
            {
                if (*outMsaaSamples == 1)
                    (*outMsaaSamples)++;
                VkSampleCountFlagBits max = getMaxUsableSampleCount(gpus[i]);
                if (max < *outMsaaSamples)
                    *outMsaaSamples = max;
            }
            break;
        }
    }
    free(gpus);

    if (*out == VK_NULL_HANDLE)
    {
        LOG_FATAL("Physical device does not meet the minimum requirements.");
        return;
    }
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(*out, &properties);
    LOG_DEBUG("Physical Device selected successfully [%s]", properties.deviceName);
}

bool checkDeviceExtensionsSupport(VkPhysicalDevice device)
{
    u32 ext_count = 0;
    vkEnumerateDeviceExtensionProperties(device, 0, &ext_count, 0);

    VkExtensionProperties *ext_props = (VkExtensionProperties *)malloc(sizeof(VkExtensionProperties) * ext_count);

    vkEnumerateDeviceExtensionProperties(device, 0, &ext_count, ext_props);
    for (u32 i = 0; i < deviceExtensionsCount(); i++)
    {
        const char *ext_name = deviceExtensionsNames()[i];
        bool found = false;
        for (u32 j = 0; j < ext_count; j++)
        {
            if (strcmp(ext_name, ext_props[j].extensionName) == 0)
            {
                found = true;
                LOG_TRACE("ext name : '%s' is supported.", ext_name);
            }
        }
        if (!found)
        {
            LOG_ERROR("ext name '%s' not supported.", ext_name);
            return false;
        }
    }
    free(ext_props);

    return true;
}

u32 findMemoryType(VkPhysicalDevice gpu, u32 typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(gpu, &memProperties);
    for (u32 i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }
    LOG_FATAL("failed to find suitable memory type!");
    return -1;
}

/// @brief for now jsut return the first suitable device
/// @param device
/// @return
bool evaluatePhysicalDevice(const VkPhysicalDevice device, const VkSurfaceKHR surface)
{
    // 1. Get core properties/features
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceProperties(device, &properties);
    vkGetPhysicalDeviceFeatures(device, &features);

    // 2. Check bindless support via descriptor indexing features
    VkPhysicalDeviceDescriptorIndexingFeatures indexingFeatures = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES};
    VkPhysicalDeviceFeatures2 deviceFeatures = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .pNext = &indexingFeatures};
    vkGetPhysicalDeviceFeatures2(device, &deviceFeatures);

    // 3. Bindless requires these features
    bool bindlessSupported =
        indexingFeatures.descriptorBindingPartiallyBound &&
        indexingFeatures.runtimeDescriptorArray &&
        indexingFeatures.shaderSampledImageArrayNonUniformIndexing;

    // 4. Check queue families
    QueueFamilyIndices indices = findQueueFamilies(device, surface);
    bool queuesValid = true;
    // Ensure this checks graphics + present
    if (indices.graphicsFamily == -1 || indices.presentFamily == -1)
    {
        queuesValid = false;
    }
    // 5. Check extensions (including VK_EXT_descriptor_indexing if needed)
    bool extensionsSupported = checkDeviceExtensionsSupport(device);

    // 6. Swapchain support
    bool swapchainAdequate = false;
    if (extensionsSupported)
    {
        SwapChainSupportDetails swapchainSupport;
        querySwapChainSupport(device, surface, &swapchainSupport);
        swapchainAdequate =
            swapchainSupport.formatsCount > 0 &&
            swapchainSupport.presentModesCount > 0;
        freeSwapChainSupportDetails(&swapchainSupport);
    }

    // 7. Final suitability
    return (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ||
            properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) &&
           queuesValid &&
           extensionsSupported &&
           swapchainAdequate &&
           features.samplerAnisotropy &&
           bindlessSupported;
}

VkSampleCountFlagBits getMaxUsableSampleCount(VkPhysicalDevice gpu)
{
    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(gpu, &physicalDeviceProperties);

    VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
    if (counts & VK_SAMPLE_COUNT_64_BIT)
    {
        return VK_SAMPLE_COUNT_64_BIT;
    }
    if (counts & VK_SAMPLE_COUNT_32_BIT)
    {
        return VK_SAMPLE_COUNT_32_BIT;
    }
    if (counts & VK_SAMPLE_COUNT_16_BIT)
    {
        return VK_SAMPLE_COUNT_16_BIT;
    }
    if (counts & VK_SAMPLE_COUNT_8_BIT)
    {
        return VK_SAMPLE_COUNT_8_BIT;
    }
    if (counts & VK_SAMPLE_COUNT_4_BIT)
    {
        return VK_SAMPLE_COUNT_4_BIT;
    }
    if (counts & VK_SAMPLE_COUNT_2_BIT)
    {
        return VK_SAMPLE_COUNT_2_BIT;
    }

    return VK_SAMPLE_COUNT_1_BIT;
}