#ifndef VERTEX_H
#define VERTEX_H

#include "renderer/render_types.h"

VkVertexInputBindingDescription getVertex3DInputBindingDescription();
VkVertexInputBindingDescription getVertex2DInputBindingDescription();

void getVertex3DInputAttributeDescriptions(u32* outCount, VkVertexInputAttributeDescription** outAttribDescs);
void getVertex2DInputAttributeDescriptions(u32* outCount, VkVertexInputAttributeDescription** outAttribDescs);

#endif //VERTEX_H