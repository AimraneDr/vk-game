#include "systems/PBR_renderer.h"

#include "components/transform.h"
#include "components/meshRenderer.h"
#include "core/debugger.h"

#include "renderer/render_types.h"
#include "renderer/details/pipeline.h"
#include "renderer/details/uniformBuffer.h"
#include "renderer/details/texture.h"
#include "renderer/details/descriptor.h"

//TODO: temporarry
#include <math/mat.h>
#include <math/trigonometry.h>

#include "ecs/ecs.h"
#include "game.h"

typedef struct PBR_renderer_InternalState_t{
    const Renderer* r;
    Pipeline graphicsPipeline;

    Camera* camera
}PBR_renderer_InternalState;


static void start(void* _state, void* gState);
static void update(void* _state, void* gState);
static void update_entity(void* _state, void* gState, EntityID e);
static void destroy(void* _state, void* gState);
static void destroy_entity(void* _state, void* gState, EntityID e);

VkVertexInputBindingDescription _getVertexInputBindingDescription();
void _getVertexInputAttributeDescriptions(u32* outCount, VkVertexInputAttributeDescription** outAttribDescs);

void createDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout* out);
void createDescriptorPool(VkDevice device, VkDescriptorPool* out);
void createDescriptorSets(VkDevice device, VkDescriptorSetLayout setLayout, VkDescriptorPool pool, VkBuffer* uniformBuffers, VkImageView textureImageView, VkSampler textureSampler, VkDescriptorSet* outDescriptorSets);
void updateGlobalUniformBuffer(uint32_t currentImage, void** uniformBuffersMapped, f64 deltatime, Camera* camera);

System PBR_renderer_get_system_ref(Scene* scene, Renderer* r, Camera* camera){
    PBR_renderer_InternalState* s = malloc(sizeof(PBR_renderer_InternalState));
    memcpy(s,&(PBR_renderer_InternalState){0}, sizeof(PBR_renderer_InternalState));
    s->r = r;
    s->camera = camera;
    return (System){
        .Signature = ecs_get_component_type(scene, "MeshRenderer") | ecs_get_component_type(scene, "Transform"),
        .start = start,
        .update = update,
        .updateEntity = update_entity,
        .destroy = destroy,
        .destroyEntity = destroy_entity,
        .state = s
    };
}


/////////////////////////////////////
/////////////////////////////////////
/////      IMPLEMENTATION     ///////
/////////////////////////////////////
/////////////////////////////////////


void start(void* _state, void* gState){
    //initialize the pipeline
    PBR_renderer_InternalState* state = _state;
    Renderer* r = &((GameState*)gState)->renderer;
    createDescriptorSetLayout(r->device, &state->graphicsPipeline.descriptorSetLayout);

    PipelineConfig config = {
        .get_vertex_binding_desc = _getVertexInputBindingDescription,
        .get_vertex_attr_descs = _getVertexInputAttributeDescriptions,
        .depth_test_enable = VK_TRUE,
        .depth_write_enable = VK_TRUE,
        .depth_compare_op = VK_COMPARE_OP_LESS,
        .subpass_index = 0,
        .push_constant_size = sizeof(PBR_PushConstant),
        .push_constant_stage = VK_SHADER_STAGE_VERTEX_BIT,
        .src_color_blend_factor = VK_BLEND_FACTOR_SRC_ALPHA,
        .dst_color_blend_factor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA
    };
    create_graphics_pipeline(
        r->device,
        "shaders/default/vert.spv", "shaders/default/frag.spv",
        r->swapchainExtent,
        r->msaaSamples,
        r->renderPass,
        state->graphicsPipeline.descriptorSetLayout,
        &config,
        &state->graphicsPipeline.pipelineLayout,
        &state->graphicsPipeline.ref
    );
    createUniformBuffers(r->gpu, r->device, sizeof(PBR_GLOBAL_UBO), state->graphicsPipeline.uniform.global.buffers, state->graphicsPipeline.uniform.global.buffersMemory, state->graphicsPipeline.uniform.global.buffersMapped);

    createTextureImage(r->gpu, r->device, r->commandPool, r->queue.graphics, &r->mipLevels, &state->graphicsPipeline.textureImage, &state->graphicsPipeline.textureImageMemory);
    createTextureImageView(r->device, state->graphicsPipeline.textureImage, r->mipLevels, &state->graphicsPipeline.textureImageView);
    createTextureImageSampler(r->gpu, r->device, r->mipLevels, &state->graphicsPipeline.textureSampler);

    createDescriptorPool(r->device, &state->graphicsPipeline.descriptorPool);
    createDescriptorSets(r->device, state->graphicsPipeline.descriptorSetLayout, state->graphicsPipeline.descriptorPool, state->graphicsPipeline.uniform.global.buffers, state->graphicsPipeline.textureImageView, state->graphicsPipeline.textureSampler,state->graphicsPipeline.descriptorSets);
}

void update(void* _state, void* gState){
    PBR_renderer_InternalState* state = _state;
    Renderer* r = &((GameState*)gState)->renderer;
    f32 dt =  ((GameState*)gState)->clock.deltaTime;
    
 
    updateGlobalUniformBuffer(r->currentFrame, state->graphicsPipeline.uniform.global.buffersMapped, dt, state->camera);

    vkCmdBindPipeline(r->commandBuffers[r->currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, state->graphicsPipeline.ref);
    vkCmdBindDescriptorSets(r->commandBuffers[r->currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, state->graphicsPipeline.pipelineLayout, 0, 1, state->graphicsPipeline.descriptorSets, 0, 0);
}

void update_entity(void* _state, void* gState, EntityID e){
    PBR_renderer_InternalState* state = _state;
    Renderer* r = &((GameState*)gState)->renderer;
    Scene* scene = &((GameState*)gState)->scene;

    MeshRenderer* m = GET_COMPONENT(scene, e, MeshRenderer);
    Transform* t = GET_COMPONENT(scene, e, Transform);
    t->mat = mat4_mul(
        mat4_scaling(t->scale),
        mat4_mul(
            mat4_rotation(deg_to_rad(t->rotation.x), (Vec3){.0f, 1.0f, .0f}),
            mat4_translation(t->position)
        )
    );
    PBR_PushConstant pc = {
            .model = t->mat
    };
    VkBuffer vertexBuffers[] = { m->renderContext.vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdPushConstants(
        r->commandBuffers[r->currentFrame],
        state->graphicsPipeline.pipelineLayout,
        VK_SHADER_STAGE_VERTEX_BIT,
        0,
        sizeof(PBR_PushConstant),
        &pc
    );
    vkCmdBindVertexBuffers(r->commandBuffers[r->currentFrame], 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(r->commandBuffers[r->currentFrame], m->renderContext.indexBuffer,0, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(r->commandBuffers[r->currentFrame], m->indicesCount, 1, 0, 0, 0);
}

void destroy(void* _state, void* gState){
    PBR_renderer_InternalState* state = _state;
    Renderer* r = &((GameState*)gState)->renderer;

    vkDeviceWaitIdle(r->device);
    
    destroyUniformBuffers(r->device, state->graphicsPipeline.uniform.global.buffers, state->graphicsPipeline.uniform.global.buffersMemory);
    destroyTextureSampler(r->device, state->graphicsPipeline.textureSampler);
    destroyTextureImageView(r->device, state->graphicsPipeline.textureImageView);
    destroyTextureImage(r->device, state->graphicsPipeline.textureImage, state->graphicsPipeline.textureImageMemory);
    destroyPipeline(r->device, &state->graphicsPipeline.ref, state->graphicsPipeline.pipelineLayout);
    destroyDescriptorPool(r->device, state->graphicsPipeline.descriptorPool);
    destroyDescriptorSetLayout(r->device, state->graphicsPipeline.descriptorSetLayout);
}

void destroy_entity(void* _state, void* gState, EntityID e){
    PBR_renderer_InternalState* state = _state;
    MeshRenderer* m = GET_COMPONENT(&((GameState*)gState)->scene, e, MeshRenderer);
    destroyMeshRenderer(&((GameState*)gState)->renderer, m);
}
///////////////////////////////
///////////////////////////////
/////      INTERNAL     ///////
///////////////////////////////
///////////////////////////////
static const u8 bindingsCount = 2;

void createDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout* out){
    VkDescriptorSetLayoutBinding uboLayoutBinding = {
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .pImmutableSamplers = 0
    };
    VkDescriptorSetLayoutBinding samplerLayoutBinding = {
        .binding = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1,
        .pImmutableSamplers = 0,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
    };

    VkDescriptorSetLayoutBinding bindings[bindingsCount] = {uboLayoutBinding, samplerLayoutBinding};

    VkDescriptorSetLayoutCreateInfo layoutInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = bindingsCount,
        .pBindings = bindings
    };

    VkResult res = vkCreateDescriptorSetLayout(device, &layoutInfo, 0, out);
    if(res != VK_SUCCESS){
        LOG_FATAL("failed to create descriptor set layout!");
    }
}


void createDescriptorPool(VkDevice device, VkDescriptorPool* out){
    VkDescriptorPoolSize poolSizes[bindingsCount] = {
        //0
        {
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = MAX_FRAMES_IN_FLIGHT
        },
        //1
        {
            .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = MAX_FRAMES_IN_FLIGHT
        }
    };

    VkDescriptorPoolCreateInfo poolInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .poolSizeCount = bindingsCount,
        .pPoolSizes = poolSizes,
        .maxSets = MAX_FRAMES_IN_FLIGHT
    };

    VkResult res = vkCreateDescriptorPool(device, &poolInfo, 0, out);
    if(res != VK_SUCCESS){
        LOG_FATAL("failed to create descriptor pool!");
    }
}


void createDescriptorSets(VkDevice device, VkDescriptorSetLayout setLayout, VkDescriptorPool pool, VkBuffer* uniformBuffers, VkImageView textureImageView, VkSampler textureSampler, VkDescriptorSet* outDescriptorSets){
    VkDescriptorSetLayout layouts[MAX_FRAMES_IN_FLIGHT] = {setLayout,setLayout};
    VkDescriptorSetAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = pool,
        .descriptorSetCount = MAX_FRAMES_IN_FLIGHT,
        .pSetLayouts = layouts
    };

    VkResult res = vkAllocateDescriptorSets(device, &allocInfo, outDescriptorSets);
    if(res != VK_SUCCESS){
        LOG_FATAL("failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo bufferInfo = {
            .buffer = uniformBuffers[i],
            .offset = 0,
            .range = sizeof(PBR_GLOBAL_UBO)
        };
        VkDescriptorImageInfo imageInfo = {
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            .imageView = textureImageView,
            .sampler = textureSampler
        };
        VkWriteDescriptorSet descriptorWrites[bindingsCount] = {};
        descriptorWrites[0] = (VkWriteDescriptorSet){
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = outDescriptorSets[i],
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
            .pBufferInfo = &bufferInfo
        };
        descriptorWrites[1] = (VkWriteDescriptorSet){
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = outDescriptorSets[i],
            .dstBinding = 1,
            .dstArrayElement = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 1,
            .pImageInfo = &imageInfo
        };

        vkUpdateDescriptorSets(device, bindingsCount, descriptorWrites, 0, 0);
    }
}

void updateGlobalUniformBuffer(uint32_t currentImage, void** uniformBuffersMapped, f64 deltatime, Camera* camera) {

    camera_updateViewMat(camera);
    camera_updateProjectionMat(camera, (Vec2){camera->viewRect.x,camera->viewRect.y});

    PBR_GLOBAL_UBO ubo;
    ubo.view = camera->view;
    ubo.proj = camera->projection;

    memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}

VkVertexInputBindingDescription _getVertexInputBindingDescription(){
    return (VkVertexInputBindingDescription) {
        .binding = 0,
        .stride = sizeof(Vertex),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
    };
}

void _getVertexInputAttributeDescriptions(u32* outCount, VkVertexInputAttributeDescription** outAttribDescs){
    *outCount = 3;
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
    (*outAttribDescs)[2].format = VK_FORMAT_R32G32_SFLOAT;
    (*outAttribDescs)[2].offset = offsetof(Vertex, texCoord);
}