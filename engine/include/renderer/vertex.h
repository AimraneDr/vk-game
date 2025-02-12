#ifndef VERTEX_H
#define VERTEX_H

#include "renderer/render_types.h"

VkVertexInputBindingDescription getVertexInputBindingDescription();
VkVertexInputBindingDescription getUIVertexInputBindingDescription();

void getVertexInputAttributeDescriptions(u32* outCount, VkVertexInputAttributeDescription** outAttribDescs);
void getUIVertexInputAttributeDescriptions(u32* outCount, VkVertexInputAttributeDescription** outAttribDescs);

#endif //VERTEX_H