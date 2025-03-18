#include "systems/PBR_renderer.h"

#include "components/transform.h"
#include "components/meshRenderer.h"
#include "components/Hierarchy.h"
#include "core/debugger.h"
#include "assets/asset_manager.h"

#include "renderer/render_types.h"
#include "renderer/details/pipeline.h"
#include "renderer/details/uniformBuffer.h"
#include "renderer/details/texture.h"
#include "renderer/details/descriptor.h"

//TODO: temporarry
#include <math/mat.h>
#include <math/vec2.h>
#include <math/vec3.h>
#include <math/vec4.h>
#include <math/trigonometry.h>

#include <collections/DynamicArray.h>

#include "ecs/ecs.h"
#include "game.h"

typedef struct PBR_renderer_InternalState_t{
    const Renderer* r;
    Pipeline graphicsPipeline;
}PBR_renderer_InternalState;


static void start(void* _state, void* gState);
static void pre_update(void* _state, void* gState);
static void pre_update_entity(void* _state, void* gState, EntityID e);
static void update(void* _state, void* gState);
static void update_entity(void* _state, void* gState, EntityID e);
static void destroy(void* _state, void* gState);
static void destroy_entity(void* _state, void* gState, EntityID e);

VkVertexInputBindingDescription _getVertexInputBindingDescription();
void _getVertexInputAttributeDescriptions(u32* outCount, VkVertexInputAttributeDescription** outAttribDescs);

void createDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout* out);
void createDescriptorPool(VkDevice device, VkDescriptorPool* out);
void updateUBOsDescriptorSets(VkDevice device, VkBuffer* globalUBOs, VkBuffer* elementUBOs, VkDeviceSize elementAlignmentSize, VkDescriptorSet* outDescriptorSets);
void createDescriptorSets(VkDevice device, VkDescriptorSetLayout setLayout, VkDescriptorPool pool, VkDescriptorSet* outDescriptorSets);
void updateGlobalUniformBuffer(uint32_t currentImage, void** uniformBuffersMapped, f64 deltatime, Camera* camera);
void updateElementUniformBuffer(void* uniformBufferMapped, Material* mtl, u32 alignIndex, VkDeviceSize alignedUboSize);
void uploadMaterialTextures(VkDevice device, VkDescriptorSet* descriptorSet, Material* mtl);

System PBR_renderer_get_system_ref(Scene* scene, Renderer* r, Camera* camera){
    PBR_renderer_InternalState* s = malloc(sizeof(PBR_renderer_InternalState));
    memcpy(s,&(PBR_renderer_InternalState){0}, sizeof(PBR_renderer_InternalState));
    s->r = r;
    return (System){
        .Signature = ecs_get_component_type(scene, "MeshRenderer") | ecs_get_component_type(scene, "Transform"),
        .start = start,
        .preUpdate = pre_update,
        .preUpdateEntity = pre_update_entity,
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
    createDescriptorSetLayout(r->context->device, &state->graphicsPipeline.descriptorSetLayout);

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
        r->context->device,
        "shaders/default/vert.spv", "shaders/default/frag.spv",
        r->context->swapchainExtent,
        r->context->msaaSamples,
        r->renderPass,
        state->graphicsPipeline.descriptorSetLayout,
        &config,
        &state->graphicsPipeline.pipelineLayout,
        &state->graphicsPipeline.ref
    );
    createUniformBuffers(r->context->gpu, r->context->device, sizeof(PBR_GLOBAL_UBO), state->graphicsPipeline.uniform.global.buffers, state->graphicsPipeline.uniform.global.buffersMemory, state->graphicsPipeline.uniform.global.buffersMapped);
    createDynamicOffsetUniformBuffers(r->context->gpu, r->context->device, sizeof(PBR_Material_UBO), state->graphicsPipeline.uniform.element.buffers, state->graphicsPipeline.uniform.element.buffersMemory, state->graphicsPipeline.uniform.element.buffersMapped, &state->graphicsPipeline.uniform.element.alignedUboSize);

    createDefaultTextures(&state->graphicsPipeline.defaultMaterial);

    createDescriptorPool(r->context->device, &state->graphicsPipeline.descriptorPool);
    createDescriptorSets(r->context->device, state->graphicsPipeline.descriptorSetLayout, state->graphicsPipeline.descriptorPool, state->graphicsPipeline.descriptorSets);
    updateUBOsDescriptorSets(r->context->device, state->graphicsPipeline.uniform.global.buffers, state->graphicsPipeline.uniform.element.buffers, state->graphicsPipeline.uniform.element.alignedUboSize, state->graphicsPipeline.descriptorSets);
}

static void pre_update(void* _state, void* gState){
    
}

static void pre_update_entity(void* _state, void* gState, EntityID e){
    PBR_renderer_InternalState* state = _state;
    Renderer* r = &((GameState*)gState)->renderer;
    Scene* scene = &((GameState*)gState)->scene;

    MeshRenderer* m = GET_COMPONENT(scene, e, MeshRenderer);
    uploadMaterialTextures(r->context->device, state->graphicsPipeline.descriptorSets, m->material);
}

static u32 elementCounter;
void update(void* _state, void* gState){
    PBR_renderer_InternalState* state = _state;
    Renderer* r = &((GameState*)gState)->renderer;
    Camera* cam = &((GameState*)gState)->camera;
    f32 dt =  ((GameState*)gState)->clock.deltaTime;
    
    updateGlobalUniformBuffer(r->currentFrame, state->graphicsPipeline.uniform.global.buffersMapped, dt, cam);

    PBR_Material_UBO m_ubo = {0};
    m_ubo.uvTiling = vec2_new(.5,.5);
    m_ubo.albedoFactor = vec4_new(1,1,1,1);
    memcpy(state->graphicsPipeline.uniform.element.buffersMapped[r->currentFrame], &m_ubo, sizeof(m_ubo));

    vkCmdBindPipeline(r->context->commandBuffers[r->currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, state->graphicsPipeline.ref);
    vkCmdBindDescriptorSets(r->context->commandBuffers[r->currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, state->graphicsPipeline.pipelineLayout, 0, 1, &state->graphicsPipeline.descriptorSets[r->currentFrame], 1, (u32[]){0});    
    elementCounter=0;
}

void update_entity(void* _state, void* gState, EntityID e){
    PBR_renderer_InternalState* state = _state;
    Renderer* r = &((GameState*)gState)->renderer;
    Scene* scene = &((GameState*)gState)->scene;

    MeshRenderer* m = GET_COMPONENT(scene, e, MeshRenderer);
    Transform* t = GET_COMPONENT(scene, e, Transform);
    Hierarchy* h = GET_COMPONENT(scene, e, Hierarchy);
    Transform* parentT = 0;
    MeshData* mData = (MeshData*)m->data;

    if(h && h->parent != INVALID_ENTITY) parentT = GET_COMPONENT(scene, h->parent, Transform);
    transform_update(t, parentT? &parentT->__local_mat : 0);

     
    PBR_PushConstant pc = {
            .model = t->mat
    };
    VkBuffer vertexBuffers[] = { mData->vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdPushConstants(
        r->context->commandBuffers[r->currentFrame],
        state->graphicsPipeline.pipelineLayout,
        VK_SHADER_STAGE_VERTEX_BIT,
        0,
        sizeof(PBR_PushConstant),
        &pc
    );
    u32 dynamicOffset = elementCounter * state->graphicsPipeline.uniform.element.alignedUboSize;
    
    vkCmdBindDescriptorSets(r->context->commandBuffers[r->currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, state->graphicsPipeline.pipelineLayout, 0, 1, &state->graphicsPipeline.descriptorSets[r->currentFrame], 1, &dynamicOffset);
    
    updateElementUniformBuffer(state->graphicsPipeline.uniform.element.buffersMapped[r->currentFrame], m->material ,elementCounter++, state->graphicsPipeline.uniform.element.alignedUboSize);
    
    vkCmdBindVertexBuffers(r->context->commandBuffers[r->currentFrame], 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(r->context->commandBuffers[r->currentFrame], mData->indexBuffer,0, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(r->context->commandBuffers[r->currentFrame], mData->indicesCount, 1, 0, 0, 0);
}

void destroy(void* _state, void* gState){
    PBR_renderer_InternalState* state = _state;
    Renderer* r = &((GameState*)gState)->renderer;

    vkDeviceWaitIdle(r->context->device);
    
    destroyUniformBuffers(r->context->device, state->graphicsPipeline.uniform.global.buffers, state->graphicsPipeline.uniform.global.buffersMemory);
    destroyUniformBuffers(r->context->device, state->graphicsPipeline.uniform.element.buffers, state->graphicsPipeline.uniform.element.buffersMemory);
    destroyDefaultTextures(&state->graphicsPipeline.defaultMaterial);
    destroyPipeline(r->context->device, &state->graphicsPipeline.ref, state->graphicsPipeline.pipelineLayout);
    destroyDescriptorPool(r->context->device, state->graphicsPipeline.descriptorPool);
    destroyDescriptorSetLayout(r->context->device, state->graphicsPipeline.descriptorSetLayout);
}

void destroy_entity(void* _state, void* gState, EntityID e){
    MeshRenderer* m = GET_COMPONENT(&((GameState*)gState)->scene, e, MeshRenderer);
    destroyMeshRenderer(m);
}
///////////////////////////////
///////////////////////////////
/////      INTERNAL     ///////
///////////////////////////////
///////////////////////////////
#define samplersCount 1000
#define bindingsCount 3

void addTextureToDescriptorSet(
    VkDevice device,
    VkDescriptorSet* descriptorSet,
    Texture* texture
) {
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorImageInfo imageInfo = {
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            .imageView = texture->imageView,
            .sampler = texture->sampler
        };

        VkWriteDescriptorSet write = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = descriptorSet[i],
            .dstBinding = 2,
            .dstArrayElement = texture->Idx,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo = &imageInfo
        };

        vkUpdateDescriptorSets(device, 1, &write, 0, NULL);
    }
}

void createDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout* out){
    VkDescriptorSetLayoutBinding uboLayoutBinding = {
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .pImmutableSamplers = 0
    };
    VkDescriptorSetLayoutBinding materialUboLayoutBinding = {
        .binding = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
        .pImmutableSamplers = 0
    };
    VkDescriptorSetLayoutBinding samplersLayoutBinding = {
        .binding = 2,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = samplersCount,
        .pImmutableSamplers = 0,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
    };

    VkDescriptorSetLayoutBinding bindings[bindingsCount] = {
        uboLayoutBinding,
        materialUboLayoutBinding, 
        samplersLayoutBinding
    };

    VkDescriptorSetLayoutCreateInfo layoutInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = bindingsCount,
        .pBindings = bindings,
        .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT
    };

    VkResult res = vkCreateDescriptorSetLayout(device, &layoutInfo, 0, out);
    if(res != VK_SUCCESS){
        LOG_FATAL("failed to create descriptor set layout!");
    }
}


void createDescriptorPool(VkDevice device, VkDescriptorPool* out){
    VkDescriptorPoolSize poolSizes[bindingsCount] = {
        // global ubo
        {
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = MAX_FRAMES_IN_FLIGHT
        },
        // material ubo
        {
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
            .descriptorCount = MAX_FRAMES_IN_FLIGHT
        },
        // samplers
        {
            .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = samplersCount * MAX_FRAMES_IN_FLIGHT
        }
    };

    VkDescriptorPoolCreateInfo poolInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .poolSizeCount = bindingsCount,
        .pPoolSizes = poolSizes,
        .maxSets = MAX_FRAMES_IN_FLIGHT,
        .flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT
    };

    VkResult res = vkCreateDescriptorPool(device, &poolInfo, 0, out);
    if(res != VK_SUCCESS){
        LOG_FATAL("failed to create descriptor pool!");
    }
}

void updateUBOsDescriptorSets(VkDevice device, VkBuffer* globalUBOs, VkBuffer* elementUBOs, VkDeviceSize elementAlignmentSize, VkDescriptorSet* outDescriptorSets){
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo globalBuffInfo = {
            .buffer = globalUBOs[i],
            .offset = 0,
            .range = sizeof(PBR_GLOBAL_UBO)
        };
        VkDescriptorBufferInfo materialBuffInfo = {
            .buffer = elementUBOs[i],
            .offset = 0,
            .range = elementAlignmentSize //sizeof(PBR_Material_UBO)
        };
        VkWriteDescriptorSet descriptorWrites[2] = {};
        descriptorWrites[0] = (VkWriteDescriptorSet){
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = outDescriptorSets[i],
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
            .pBufferInfo = &globalBuffInfo
        };
        descriptorWrites[1] = (VkWriteDescriptorSet){
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = outDescriptorSets[i],
            .dstBinding = 1,
            .dstArrayElement = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
            .descriptorCount = 1,
            .pBufferInfo = &materialBuffInfo
        };

        vkUpdateDescriptorSets(device, 2, descriptorWrites, 0, 0);
    }
}
void createDescriptorSets(VkDevice device, VkDescriptorSetLayout setLayout, VkDescriptorPool pool, VkDescriptorSet* outDescriptorSets){
    VkDescriptorSetLayout layouts[MAX_FRAMES_IN_FLIGHT];
    for(u8 i=0; i<MAX_FRAMES_IN_FLIGHT; i++){
        layouts[i] = setLayout;
    };
    
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

}


void updateGlobalUniformBuffer(uint32_t currentImage, void** uniformBuffersMapped, f64 deltatime, Camera* camera) {

    camera_updateViewMat(camera);
    camera_updateProjectionMat(camera, vec2_new(camera->viewRect.x,camera->viewRect.y));

    PBR_GLOBAL_UBO ubo;
    ubo.view = camera->view;
    ubo.proj = camera->projection;

    memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}

//TODO: temp
static bool loaded[samplersCount] = {0};
void uploadMaterialTextures(VkDevice device, VkDescriptorSet* descriptorSet, Material* mtl) {
    //upload material textures if not uploaded already
    if(!loaded[mtl->albedo->Idx]){
        addTextureToDescriptorSet(device, descriptorSet, mtl->albedo);
        loaded[mtl->albedo->Idx] = true;
    }
    if(!loaded[mtl->normal->Idx]){
        addTextureToDescriptorSet(device, descriptorSet, mtl->normal);
        loaded[mtl->normal->Idx] = true;
    }
    if(!loaded[mtl->metalRoughAO->Idx]){
        addTextureToDescriptorSet(device, descriptorSet, mtl->metalRoughAO);
        loaded[mtl->metalRoughAO->Idx] = true;
    }
    if(!loaded[mtl->emissive->Idx]){
        addTextureToDescriptorSet(device, descriptorSet, mtl->emissive);
        loaded[mtl->emissive->Idx] = true;
    }
    if(!loaded[mtl->height->Idx]){
        addTextureToDescriptorSet(device, descriptorSet, mtl->height);
        loaded[mtl->height->Idx] = true;
    }
}

void updateElementUniformBuffer(void* uniformBufferMapped, Material* mtl, u32 alignIndex, VkDeviceSize alignedUboSize) {
    
    PBR_Material_UBO ubo = {
        .albedo_idx = mtl->albedo->Idx,
        .normal_idx = mtl->normal->Idx,
        .metalRoughAO_idx = mtl->metalRoughAO->Idx,
        .emissive_idx = mtl->emissive->Idx,
        .height_idx = mtl->height->Idx,

        .albedoFactor = mtl->albedoFactor,
        .aoFactor = mtl->aoFactor,
        .emissiveFactor = mtl->emissiveFactor,
        .metallicFactor = mtl->metallicFactor,
        .heightScale = mtl->heightScale,
        .roughnessFactor = mtl->roughnessFactor,
        .uvOffset = mtl->uvOffset,
        .uvTiling = mtl->uvTiling
    };

    VkDeviceSize offset = alignIndex * alignedUboSize;
    memcpy((void*)uniformBufferMapped+offset, &ubo, sizeof(ubo));
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