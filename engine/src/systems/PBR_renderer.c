#include "systems/PBR_renderer.h"

#include "components/transform.h"
#include "components/meshRenderer.h"
#include "components/Hierarchy.h"
#include "core/debugger.h"

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

#include "ecs/ecs.h"
#include "game.h"

typedef struct PBR_renderer_InternalState_t{
    const Renderer* r;
    Pipeline graphicsPipeline;
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
void createDescriptorSets(VkDevice device, VkDescriptorSetLayout setLayout, VkDescriptorPool pool, VkBuffer* globalUBOs, VkBuffer* elementUBOs, Material* mat, VkDescriptorSet* outDescriptorSets);
void updateGlobalUniformBuffer(uint32_t currentImage, void** uniformBuffersMapped, f64 deltatime, Camera* camera);

System PBR_renderer_get_system_ref(Scene* scene, Renderer* r, Camera* camera){
    PBR_renderer_InternalState* s = malloc(sizeof(PBR_renderer_InternalState));
    memcpy(s,&(PBR_renderer_InternalState){0}, sizeof(PBR_renderer_InternalState));
    s->r = r;
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
    createUniformBuffers(r->context->gpu, r->context->device, sizeof(PBR_Material_UBO), state->graphicsPipeline.uniform.element.buffers, state->graphicsPipeline.uniform.element.buffersMemory, state->graphicsPipeline.uniform.element.buffersMapped);

    createDefaultTextures(&state->graphicsPipeline.defaultMaterial);

    createDescriptorPool(r->context->device, &state->graphicsPipeline.descriptorPool);
    createDescriptorSets(r->context->device, state->graphicsPipeline.descriptorSetLayout, state->graphicsPipeline.descriptorPool, state->graphicsPipeline.uniform.global.buffers, state->graphicsPipeline.uniform.element.buffers, &state->graphicsPipeline.defaultMaterial, state->graphicsPipeline.descriptorSets);
}

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
    vkCmdBindDescriptorSets(r->context->commandBuffers[r->currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, state->graphicsPipeline.pipelineLayout, 0, 1, state->graphicsPipeline.descriptorSets, 0, 0);
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
#define bindingsCount 7

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
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
        .pImmutableSamplers = 0
    };
    VkDescriptorSetLayoutBinding albidoSamplerLayoutBinding = {
        .binding = 2,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1,
        .pImmutableSamplers = 0,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
    };
    VkDescriptorSetLayoutBinding normalSamplerLayoutBinding = {
        .binding = 3,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1,
        .pImmutableSamplers = 0,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
    };
    VkDescriptorSetLayoutBinding metalRoughAmbOclSamplerLayoutBinding = {
        .binding = 4,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1,
        .pImmutableSamplers = 0,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
    };
    VkDescriptorSetLayoutBinding emmissionSamplerLayoutBinding = {
        .binding = 5,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1,
        .pImmutableSamplers = 0,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
    };
    VkDescriptorSetLayoutBinding heightSamplerLayoutBinding = {
        .binding = 6,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1,
        .pImmutableSamplers = 0,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
    };

    VkDescriptorSetLayoutBinding bindings[bindingsCount] = {
        uboLayoutBinding,
        materialUboLayoutBinding, 
        albidoSamplerLayoutBinding,
        normalSamplerLayoutBinding,
        metalRoughAmbOclSamplerLayoutBinding,
        emmissionSamplerLayoutBinding,
        heightSamplerLayoutBinding
    };

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
        // global ubo
        {
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = MAX_FRAMES_IN_FLIGHT
        },
        // material ubo
        {
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = MAX_FRAMES_IN_FLIGHT
        },
        // albido 
        {
            .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = MAX_FRAMES_IN_FLIGHT
        },
        // norma;
        {
            .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = MAX_FRAMES_IN_FLIGHT
        },
        // metalic-roughness-ambientOclusion
        {
            .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = MAX_FRAMES_IN_FLIGHT
        },
        // emission
        {
            .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = MAX_FRAMES_IN_FLIGHT
        },
        // height
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


void createDescriptorSets(VkDevice device, VkDescriptorSetLayout setLayout, VkDescriptorPool pool, VkBuffer* globalUBOs, VkBuffer* elementUBOs, Material* mat, VkDescriptorSet* outDescriptorSets){
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

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo globalBuffInfo = {
            .buffer = globalUBOs[i],
            .offset = 0,
            .range = sizeof(PBR_GLOBAL_UBO)
        };
        VkDescriptorBufferInfo materialBuffInfo = {
            .buffer = elementUBOs[i],
            .offset = 0,
            .range = sizeof(PBR_Material_UBO)
        };
        VkDescriptorImageInfo albidoImageInfo = {
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            .imageView = mat->albedo->imageView,
            .sampler = mat->albedo->sampler
        };
        VkDescriptorImageInfo normalImageInfo = {
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            .imageView = mat->normal->imageView,
            .sampler = mat->normal->sampler
        };
        VkDescriptorImageInfo metalRoughAOImageInfo = {
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            .imageView = mat->metalRoughAO->imageView,
            .sampler = mat->metalRoughAO->sampler
        };
        VkDescriptorImageInfo emissionImageInfo = {
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            .imageView = mat->emissive->imageView,
            .sampler = mat->emissive->sampler
        };
        VkDescriptorImageInfo heightImageInfo = {
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            .imageView = mat->height->imageView,
            .sampler = mat->height->sampler
        };
        VkWriteDescriptorSet descriptorWrites[bindingsCount] = {};
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
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
            .pBufferInfo = &materialBuffInfo
        };
        descriptorWrites[2] = (VkWriteDescriptorSet){
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = outDescriptorSets[i],
            .dstBinding = 2,
            .dstArrayElement = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 1,
            .pImageInfo = &albidoImageInfo
        };
        descriptorWrites[3] = (VkWriteDescriptorSet){
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = outDescriptorSets[i],
            .dstBinding = 3,
            .dstArrayElement = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 1,
            .pImageInfo = &normalImageInfo
        };
        descriptorWrites[4] = (VkWriteDescriptorSet){
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = outDescriptorSets[i],
            .dstBinding = 4,
            .dstArrayElement = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 1,
            .pImageInfo = &metalRoughAOImageInfo
        };
        descriptorWrites[5] = (VkWriteDescriptorSet){
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = outDescriptorSets[i],
            .dstBinding = 5,
            .dstArrayElement = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 1,
            .pImageInfo = &emissionImageInfo
        };
        descriptorWrites[6] = (VkWriteDescriptorSet){
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = outDescriptorSets[i],
            .dstBinding = 6,
            .dstArrayElement = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 1,
            .pImageInfo = &heightImageInfo
        };

        vkUpdateDescriptorSets(device, bindingsCount, descriptorWrites, 0, 0);
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