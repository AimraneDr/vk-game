#ifndef COMMAND_BUFFER_H
#define COMMAND_BUFFER_H

#include "renderer/render_types.h"
#include "assets/asset_types.h"
#include "components/UI/ui_types.h"
#include "components/meshRenderer.h"
#include "game.h"


void createCommandBuffer(VkDevice device, VkCommandPool pool, VkCommandBuffer* out);
VkCommandBuffer beginSingleTimeCommands(VkDevice device, VkCommandPool cmdPool);
void endSingleTimeCommands(VkDevice device, VkCommandPool cmdPool, VkQueue queue,  VkCommandBuffer* cmdBuffer);

void recordCommandBuffer(GameState* gState, uint32_t imageIndex);

void destroyCommandBuffer(VkDevice device, VkCommandPool pool, VkCommandBuffer* buff);

#endif //COMMAND_BUFFER_H