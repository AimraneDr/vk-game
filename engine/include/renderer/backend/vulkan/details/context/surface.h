#ifndef RENDERER_BACKEND_VULKAN_SURFACE_H
#define RENDERER_BACKEND_VULKAN_SURFACE_H

#include "renderer/backend/vulkan/vulkan_types.h"

#ifdef _WIN32
#include <windows.h>
void createSurface(const VkInstance instance, HINSTANCE* hInstance, HWND* hwnd, VkSurfaceKHR* out);
#endif
#ifdef __linux__
void createSurface(const VkInstance instance, Display* dpy,Window w, VkSurfaceKHR* out);
#endif

void destroySurface(VkInstance instance, VkSurfaceKHR surface);

#endif //RENDERER_BACKEND_VULKAN_SURFACE_H