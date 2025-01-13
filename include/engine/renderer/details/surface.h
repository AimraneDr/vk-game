#ifndef SURFACE_H
#define SURFACE_H

#include "engine/renderer/render_types.h"

#ifdef _WIN32
Result createSurface(const VkInstance instance, HINSTANCE hInstance, HWND hwnd, VkSurfaceKHR* out);
#endif
#ifdef __linux__
Result createSurface(const VkInstance instance, Display* dpy,Window w, VkSurfaceKHR* out);
#endif

void destroySurface(VkInstance instance, VkSurfaceKHR surface);

#endif //SURFACE_H