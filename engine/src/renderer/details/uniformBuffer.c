#include "renderer/details/uniformBuffer.h"

#include <math/vec3.h>
#include <math/mathConst.h>
#include <math/trigonometry.h>
#include <math/mat.h>
#include "renderer/details/buffer.h"
#include <windows.h>
#include "core/debugger.h"



void createUniformBuffer(VkPhysicalDevice gpu, VkDevice device, VkBuffer* uniformBuffers, VkDeviceMemory* buffersMems, void** mappedBuffs){
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        createBuffer(
            gpu,
            device,
            bufferSize, 
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
            &(uniformBuffers[i]), 
            &(buffersMems[i])
        );

        vkMapMemory(device, buffersMems[i], 0, bufferSize, 0, &(mappedBuffs[i]));
    }
}

void destroyUniformBuffer(VkDevice device, VkBuffer uniformBuffers[], VkDeviceMemory buffersMems[]){
     for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroyBuffer(device, uniformBuffers[i], 0);
        vkFreeMemory(device, buffersMems[i], 0);
    }
}

void updateUniformBuffer(uint32_t currentImage, Vec2 frameSize, void** uniformBuffersMapped, f64 deltatime) {
    static Vec3 cameraPos = (Vec3){1,2,2};
    // cameraPos.z += time* 0.01;
    
    UniformBufferObject ubo;
    static float angle = 0;
    angle += deltatime * deg_to_rad(90.0f);
    ubo.model = MAT4_IDENTITY;
    ubo.model = mat4_rotate(angle, (Vec3){0.0f, 1.0f, .0f});
    ubo.view = mat4_lookAt(
        cameraPos,  // eye
        VEC3_ZERO,  //target
        VEC3_UP   // up
    );
    // ubo.view = MAT4_IDENTITY;
    ubo.proj = mat4_perspective(
        deg_to_rad(45.0f),       // 45 degrees in radians
        frameSize.x / frameSize.y,
        0.01f,
        100.0f
    );

    memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}