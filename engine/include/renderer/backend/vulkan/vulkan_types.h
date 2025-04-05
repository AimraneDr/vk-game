#pragma once

#ifdef __linux__
#define VK_USE_PLATFORM_XLIB_KHR
#endif

#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#include <vulkan/vulkan.h>
#include <math/mathTypes.h>
#include "data_types.h"

#define MAX_FRAMES_IN_FLIGHT 3