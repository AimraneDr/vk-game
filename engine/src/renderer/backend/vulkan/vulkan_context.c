#include "renderer/backend/vulkan/vulkan_context.h"

#include "renderer/backend/vulkan/details/utils.h"
#include "renderer/backend/vulkan/details/context/physicalDevice.h"
#include "renderer/backend/vulkan/details/context/logical_device.h"
#include "renderer/backend/vulkan/details/context/queue.h"
#include "renderer/backend/vulkan/details/context/surface.h"

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