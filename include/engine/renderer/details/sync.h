#ifndef SYNC_H
#define SYNC_H

#include "engine/renderer/render_types.h"

void createSyncObjects(VkDevice device, VkSemaphore* imageAvailableSemaphore, VkSemaphore* renderFinishedSemaphore, VkFence* inFlightFence);
void destroySyncObjects(VkDevice device, VkSemaphore* imageAvailableSemaphore, VkSemaphore* renderFinishedSemaphore, VkFence* inFlightFence);

#endif //SYNC_H