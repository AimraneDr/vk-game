#include "engine/renderer/vertex.h"

#include "engine/meshTypes.h"

VkVertexInputBindingDescription getVertexInputBindingDescription(){
    return (VkVertexInputBindingDescription) {
        .binding = 0,
        .stride = sizeof(Vertex),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
    };
}

void getVertexInputAttributeDescriptions(u32* outCount, VkVertexInputAttributeDescription** outAttribDescs){
    *outCount = 4;
    *outAttribDescs = (VkVertexInputAttributeDescription*)malloc(sizeof(VkVertexInputAttributeDescription) * (*outCount));
    (*outAttribDescs)[0].binding = 0;
    (*outAttribDescs)[0].location = 0;
    (*outAttribDescs)[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    (*outAttribDescs)[0].offset = offsetof(Vertex, pos);

    (*outAttribDescs)[1].binding = 0;
    (*outAttribDescs)[1].location = 1;
    (*outAttribDescs)[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    (*outAttribDescs)[1].offset = offsetof(Vertex, norm);
    
    (*outAttribDescs)[2].binding = 0;
    (*outAttribDescs)[2].location = 2;
    (*outAttribDescs)[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    (*outAttribDescs)[2].offset = offsetof(Vertex, color);

    (*outAttribDescs)[3].binding = 0;
    (*outAttribDescs)[3].location = 3;
    (*outAttribDescs)[3].format = VK_FORMAT_R32G32_SFLOAT;
    (*outAttribDescs)[3].offset = offsetof(Vertex, texCoord);
}