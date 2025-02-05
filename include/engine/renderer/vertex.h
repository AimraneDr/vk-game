#ifndef VERTEX_H
#define VERTEX_H

#include "engine/renderer/render_types.h"

VkVertexInputBindingDescription getVertexInputBindingDescription();
void getVertexInputAttributeDescriptions(u32* outCount, VkVertexInputAttributeDescription** outAttribDescs);

#endif //VERTEX_H