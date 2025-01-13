#include "engine/renderer/details/surface.h"

#ifdef __linux__
Result createSurface(const VkInstance instance, Display* dpy,Window w,  VkSurfaceKHR* out){
    VkXlibSurfaceCreateInfoKHR createInfo = {
        .sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
        .dpy = dpy,
        .window = w
    };

    VkResult res = vkCreateXlibSurfaceKHR(instance, &createInfo, 0, out);
    if(res == VK_SUCCESS){
        return RESULT_CODE_SUCCESS;
    }

    return RESULT_CODE_FAILED_SURFACE_CREATION;
}
#endif
#ifdef _WIN32
Result createSurface(const VkInstance instance, HINSTANCE hInstance, HWND hwnd, VkSurfaceKHR* out) {
    VkWin32SurfaceCreateInfoKHR createInfo = {
        .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
        .hinstance = hInstance,
        .hwnd = hwnd
    };

    VkResult res = vkCreateWin32SurfaceKHR(instance, &createInfo, 0, out);
    if (res == VK_SUCCESS) {
        return RESULT_CODE_SUCCESS;
    }

    return RESULT_CODE_FAILED_SURFACE_CREATION;
}
#endif

void destroySurface(VkInstance instance, VkSurfaceKHR surface){
    vkDestroySurfaceKHR(instance, surface, 0);
}