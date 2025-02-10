#include "renderer/details/surface.h"

#include "core/debugger.h"
#ifdef __linux__
void createSurface(const VkInstance instance, Display *dpy, Window w, VkSurfaceKHR *out)
{
    VkXlibSurfaceCreateInfoKHR createInfo = {
        .sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
        .dpy = dpy,
        .window = w};

    VkResult res = vkCreateXlibSurfaceKHR(instance, &createInfo, 0, out);
    if (res != VK_SUCCESS)
    {
        LOG_FATAL("Failed to create surface.");
    }
}
#endif
#ifdef _WIN32
void createSurface(const VkInstance instance, HINSTANCE* hInstance, HWND* hwnd, VkSurfaceKHR *out)
{
    VkWin32SurfaceCreateInfoKHR createInfo = {
        .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
        .hinstance = *hInstance,
        .hwnd = *hwnd
    };

    VkResult res = vkCreateWin32SurfaceKHR(instance, &createInfo, 0, out);
    if (res != VK_SUCCESS)
    {
        LOG_FATAL("Failed to create surface.");
    }
}
#endif

void destroySurface(VkInstance instance, VkSurfaceKHR surface)
{
    vkDestroySurfaceKHR(instance, surface, 0);
}