#ifndef SYNC_H
#define SYNC_H

#include "renderer/render_types.h"

void createSyncObjects(VkDevice device, VkSemaphore* imageAvailableSemaphore, VkSemaphore* renderFinishedSemaphore, VkFence* inFlightFence);
void destroySyncObjects(VkDevice device, VkSemaphore* imageAvailableSemaphore, VkSemaphore* renderFinishedSemaphore, VkFence* inFlightFence);

#endif //SYNC_H