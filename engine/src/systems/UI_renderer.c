#include "systems/UI_renderer.h"

#include "components/transform.h"
#include "components/UI/uiComponents.h"
#include "core/debugger.h"
#include "meshTypes.h"
#include "renderer/render_types.h"
#include "renderer/details/pipeline.h"
#include "renderer/details/uniformBuffer.h"
#include "renderer/details/texture.h"
#include "renderer/details/descriptor.h"
#include "ecs/ecs.h"
#include "core/events.h"

//TODO: temporarry
#include <math/mat.h>
#include <math/vec2.h>
#include <math/trigonometry.h>
#include "game.h"


typedef struct UI_renderer_InternalState_t{
    const Renderer* r;
    Pipeline grapgicsPipeline;
}UI_renderer_InternalState;


static void start(void* _state, void* gState);
static void start_entity(void* _state, void* gState, EntityID e);
static void update(void* _state, void* gState);
static void update_entity(void* _state, void* gState, EntityID e);
static void destroy(void* _state, void* gState);

VkVertexInputBindingDescription ui_getVertexInputBindingDescription();
void ui_getVertexInputAttributeDescriptions(u32* outCount, VkVertexInputAttributeDescription** outAttribDescs);

void ui_createDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout* out);
void ui_createDescriptorPool(VkDevice device, VkDescriptorPool* out);
void ui_createDescriptorSets(VkDevice device, VkDescriptorSetLayout setLayout, VkDescriptorPool pool, VkBuffer* globalUniformBuffers, VkBuffer* elementUniformBuffers, VkDeviceSize elementAlignedUboSize, VkImageView textureImageView, VkSampler textureSampler, VkDescriptorSet* outDescriptorSets);
void ui_updateGlobalUniformBuffer(uint32_t currentImage, void** uniformBuffersMapped, f64 deltatime, Camera* camera);
void ui_updateElementUniformBuffer(void* uniformBufferMapped, f64 deltatime, UI_Element* uiElement, u32 alignIndex, VkDeviceSize alignedUboSize);

System UI_renderer_get_system_ref(Scene* scene, Renderer* r){
    UI_renderer_InternalState* s = malloc(sizeof(UI_renderer_InternalState));
    memcpy(s,&(UI_renderer_InternalState){0}, sizeof(UI_renderer_InternalState));
    s->r = r;
    return (System){
        .Signature = COMPONENT_TYPE(scene, Transform2D) | COMPONENT_TYPE(scene, UI_Element),
        .start = start,
        .startEntity = start_entity,
        .update = update,
        .updateEntity = update_entity,
        .destroy = destroy,
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
    UI_renderer_InternalState* state = _state;
    Renderer* r = &((GameState*)gState)->renderer;
    ui_createDescriptorSetLayout(r->device, &state->grapgicsPipeline.descriptorSetLayout);

    PipelineConfig config = {
        .get_vertex_binding_desc = ui_getVertexInputBindingDescription,
        .get_vertex_attr_descs = ui_getVertexInputAttributeDescriptions,
        .depth_test_enable = VK_TRUE,
        .depth_write_enable = VK_TRUE,
        .depth_compare_op = VK_COMPARE_OP_LESS,
        .subpass_index = 0,
        .push_constant_size = sizeof(UI_PushConstant),
        .push_constant_stage = VK_SHADER_STAGE_VERTEX_BIT,
        .src_color_blend_factor = VK_BLEND_FACTOR_SRC_ALPHA,
        .dst_color_blend_factor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .subpass_index = 1
    };
    create_graphics_pipeline(
        r->device,
        "shaders/default/ui.vert.spv", "shaders/default/ui.frag.spv",
        r->swapchainExtent,
        r->msaaSamples,
        r->renderPass,
        state->grapgicsPipeline.descriptorSetLayout,
        &config,
        &state->grapgicsPipeline.pipelineLayout,
        &state->grapgicsPipeline.ref
    );
    createUniformBuffers(r->gpu, r->device, sizeof(UI_Global_UBO), state->grapgicsPipeline.uniform.global.buffers, state->grapgicsPipeline.uniform.global.buffersMemory, state->grapgicsPipeline.uniform.global.buffersMapped);
    createDynamicOffsetUniformBuffers(r->gpu, r->device, sizeof(UI_Element_UBO), state->grapgicsPipeline.uniform.element.buffers, state->grapgicsPipeline.uniform.element.buffersMemory, state->grapgicsPipeline.uniform.element.buffersMapped, &state->grapgicsPipeline.uniform.element.alignedUboSize);

    createTextureImage(r->gpu, r->device, r->commandPool, r->queue.graphics, &r->mipLevels, &state->grapgicsPipeline.textureImage, &state->grapgicsPipeline.textureImageMemory);
    createTextureImageView(r->device, state->grapgicsPipeline.textureImage, r->mipLevels, &state->grapgicsPipeline.textureImageView);
    createTextureImageSampler(r->gpu, r->device, r->mipLevels, &state->grapgicsPipeline.textureSampler);

    ui_createDescriptorPool(r->device, &state->grapgicsPipeline.descriptorPool);
    ui_createDescriptorSets(r->device, state->grapgicsPipeline.descriptorSetLayout, state->grapgicsPipeline.descriptorPool, state->grapgicsPipeline.uniform.global.buffers, state->grapgicsPipeline.uniform.element.buffers, state->grapgicsPipeline.uniform.element.alignedUboSize, state->grapgicsPipeline.textureImageView, state->grapgicsPipeline.textureSampler,state->grapgicsPipeline.descriptorSets);
}

typedef struct ui_event_context{
    Transform2D* transform;
    UI_Element* elem;
};

EVENT_CALLBACK(onMouseMove){
    GameState* gState = listener;
    Scene* scene = &gState->scene;
    Camera* cam = &gState->camera;
    EntityID* e = data;

    Transform2D* transform = GET_COMPONENT(scene, *e, Transform2D);
    UI_Element* elem = GET_COMPONENT(scene, *e, UI_Element);
    Vec2 mouse_pos = vec2_new(((u32)eContext.u16[0])/cam->pixelsPerPoint,((u32)eContext.u16[1])/cam->pixelsPerPoint);

    Mat4 inv_mat = mat4_inverse(transform->mat);
    if (mat4_compare(inv_mat, MAT4_ZERO)) return;
    
    Vec4 mouse_world = {mouse_pos.x, mouse_pos.y, 0.0f, 1.0f};
    Vec4 mouse_local = mat4_mulVec4(inv_mat, mouse_world);
    
    Vec2 halfSize = vec2_new(elem->style.width/2.f,elem->style.height/2.f);


    if(mouse_local.x >= -halfSize.x && 
        mouse_local.x <= halfSize.x && 
        mouse_local.y >= -halfSize.y && 
        mouse_local.y <= halfSize.y)
    {
        if(!elem->hovered){
            elem->hovered = true;
            if(elem->onMouseEnter)elem->onMouseEnter(gState, *e, elem);
            return;
        }else{
            if(elem->onMouseStay)elem->onMouseStay(gState, *e, elem);
            return;
        }
    }
    
    if(elem->hovered){
        elem->hovered = false;
        if(elem->onMouseLeave)elem->onMouseLeave(gState, *e, elem);
        return;
    }
}

void start_entity(void* _state, void* gState, EntityID e){
    //this is called on the start of the program , when it should be called on the creating of an entity
    EventListener wrapper = {
        .callback=onMouseMove, 
        .listener=gState,
        //TODO: free it on unsubscribe
        .data=malloc(sizeof(EntityID)),
    };
    memcpy(wrapper.data, &e, sizeof(e));
    subscribe_to_event(
        EVENT_TYPE_MOUSE_MOVED, 
        &wrapper
    );
    LOG_WARN("entity subscribed to hover event");
}

static u32 elementCounter;
void update(void* _state, void* gState){
    UI_renderer_InternalState* state = _state;
    Renderer* r = &((GameState*)gState)->renderer;
    f32 dt = ((GameState*)gState)->clock.deltaTime;

    ui_updateGlobalUniformBuffer(r->currentFrame, state->grapgicsPipeline.uniform.global.buffersMapped, dt, &((GameState*)gState)->camera);
    
    vkCmdNextSubpass(r->commandBuffers[r->currentFrame], VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(r->commandBuffers[r->currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, state->grapgicsPipeline.ref);
    vkCmdBindDescriptorSets(r->commandBuffers[r->currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, state->grapgicsPipeline.pipelineLayout, 0, 1, &state->grapgicsPipeline.descriptorSets[r->currentFrame], 1, (u32[]){0});
    elementCounter=0;
}

void update_entity(void* _state, void* gState, EntityID e){
    UI_renderer_InternalState* state = _state;
    InputManager* inputs = &((GameState*)gState)->inputer;
    Renderer* r = &((GameState*)gState)->renderer;
    Scene* scene = &((GameState*)gState)->scene;
    f32 dt = ((GameState*)gState)->clock.deltaTime;

    UI_Element* elem = GET_COMPONENT(scene, e, UI_Element);
    Transform2D* t = GET_COMPONENT(scene, e, Transform2D);

    //emit events
    //click
    //TODO: add elem checking, even if it is hovered that does not mean it is what is selected, a child elem can be selected instead
    if(elem->hovered){
        if(is_key_down(inputs, MOUSE_BUTTON_LEFT) && elem->onMouseLDown)elem->onMouseLDown(gState, e, elem);
        if(is_key_up(inputs, MOUSE_BUTTON_LEFT) && elem->onMouseLUp)elem->onMouseLUp(gState, e, elem);
        if(is_key_pressed(inputs, MOUSE_BUTTON_LEFT) && elem->onMouseLHold)elem->onMouseLHold(gState, e, elem);

        if(is_key_down(inputs, MOUSE_BUTTON_RIGHT) && elem->onMouseRDown)elem->onMouseRDown(gState, e, elem);
        if(is_key_up(inputs, MOUSE_BUTTON_RIGHT) && elem->onMouseRUp)elem->onMouseRUp(gState, e, elem);
        if(is_key_pressed(inputs, MOUSE_BUTTON_RIGHT) && elem->onMouseRHold)elem->onMouseRHold(gState, e, elem);
    }

    transform2D_update(t);

    UI_PushConstant pc = {
        .model = t->mat
    };
    VkBuffer vertexBuffers[] = { elem->renderer.vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdPushConstants(
        r->commandBuffers[r->currentFrame],
        state->grapgicsPipeline.pipelineLayout,
        VK_SHADER_STAGE_VERTEX_BIT,
        0,
        sizeof(UI_PushConstant),
        &pc
    );
    
    u32 dynamicOffset = elementCounter * state->grapgicsPipeline.uniform.element.alignedUboSize;
    vkCmdBindDescriptorSets(r->commandBuffers[r->currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, state->grapgicsPipeline.pipelineLayout, 0, 1, &state->grapgicsPipeline.descriptorSets[r->currentFrame], 1, &dynamicOffset);
    
    ui_updateElementUniformBuffer(state->grapgicsPipeline.uniform.element.buffersMapped[r->currentFrame], dt, elem ,elementCounter++, state->grapgicsPipeline.uniform.element.alignedUboSize);
    
    vkCmdBindVertexBuffers(r->commandBuffers[r->currentFrame], 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(r->commandBuffers[r->currentFrame], elem->renderer.indexBuffer,0, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(r->commandBuffers[r->currentFrame], elem->renderer.indicesCount, 1, 0, 0, 0);
    
}

void destroy(void* _state, void* gState){
    UI_renderer_InternalState* state = _state;
    Renderer* r = &((GameState*)gState)->renderer;

    vkDeviceWaitIdle(r->device);
    destroyUniformBuffers(r->device, state->grapgicsPipeline.uniform.global.buffers, state->grapgicsPipeline.uniform.global.buffersMemory);
    destroyUniformBuffers(r->device, state->grapgicsPipeline.uniform.element.buffers, state->grapgicsPipeline.uniform.element.buffersMemory);
    destroyTextureSampler(r->device, state->grapgicsPipeline.textureSampler);
    destroyTextureImageView(r->device, state->grapgicsPipeline.textureImageView);
    destroyTextureImage(r->device, state->grapgicsPipeline.textureImage, state->grapgicsPipeline.textureImageMemory);
    destroyPipeline(r->device, &state->grapgicsPipeline.ref, state->grapgicsPipeline.pipelineLayout);
    destroyDescriptorPool(r->device, state->grapgicsPipeline.descriptorPool);
    destroyDescriptorSetLayout(r->device, state->grapgicsPipeline.descriptorSetLayout);
}

///////////////////////////////
///////////////////////////////
/////      INTERNAL     ///////
///////////////////////////////
///////////////////////////////
static const u8 bindingsCount = 3;

void ui_createDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout* out){
    VkDescriptorSetLayoutBinding global_uboLayoutBinding = {
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .pImmutableSamplers = 0
    };
    VkDescriptorSetLayoutBinding element_uboLayoutBinding = {
        .binding = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
    };
    VkDescriptorSetLayoutBinding samplerLayoutBinding = {
        .binding = 2,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1,
        .pImmutableSamplers = 0,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
    };

    VkDescriptorSetLayoutBinding bindings[bindingsCount] = {global_uboLayoutBinding, element_uboLayoutBinding, samplerLayoutBinding};

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


void ui_createDescriptorPool(VkDevice device, VkDescriptorPool* out){
    VkDescriptorPoolSize poolSizes[bindingsCount] = {
        //0
        {
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = MAX_FRAMES_IN_FLIGHT
        },
        //1
        {
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
            .descriptorCount = MAX_FRAMES_IN_FLIGHT
        },
        //2
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


void ui_createDescriptorSets(VkDevice device, VkDescriptorSetLayout setLayout, VkDescriptorPool pool, VkBuffer* globalUniformBuffers, VkBuffer* elementUniformBuffers, VkDeviceSize elementAlignedUboSize, VkImageView textureImageView, VkSampler textureSampler, VkDescriptorSet* outDescriptorSets){
    VkDescriptorSetLayout layouts[MAX_FRAMES_IN_FLIGHT];
    for(uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        layouts[i] = setLayout;
    }

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
        VkDescriptorBufferInfo globalBufferInfo = {
            .buffer = globalUniformBuffers[i],
            .offset = 0,
            .range = sizeof(UI_Global_UBO)
        };
        VkDescriptorBufferInfo elementBufferInfo = {
            .buffer = elementUniformBuffers[i],
            .offset = 0,
            .range = elementAlignedUboSize
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
            .pBufferInfo = &globalBufferInfo
        };
        descriptorWrites[1] = (VkWriteDescriptorSet){
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = outDescriptorSets[i],
            .dstBinding = 1,
            .dstArrayElement = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
            .descriptorCount = 1,
            .pBufferInfo = &elementBufferInfo
        };
        descriptorWrites[2] = (VkWriteDescriptorSet){
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = outDescriptorSets[i],
            .dstBinding = 2,
            .dstArrayElement = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 1,
            .pImageInfo = &imageInfo
        };

        vkUpdateDescriptorSets(device, bindingsCount, descriptorWrites, 0, 0);
    }
}


void ui_updateGlobalUniformBuffer(uint32_t currentImage, void** uniformBuffersMapped, f64 deltatime, Camera* camera) {
    UI_Global_UBO ubo;
    
    // View matrix remains identity for screen-space UI
    Vec2 virtualCanvas = vec2_new(camera->viewRect.x/camera->pixelsPerPoint, camera->viewRect.y/camera->pixelsPerPoint);
    ubo.proj = mat4_orthographic(
        0.0f, 
        virtualCanvas.x,  // Left to Right = canvas width
        virtualCanvas.y,  // Top to Bottom = canvas height (Vulkan Y-down)
        0.0f, 
        -1.0f, 
        1.0f
    );
    memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}

void ui_updateElementUniformBuffer(void* uniformBufferMapped, f64 deltatime, UI_Element* uiElement, u32 alignIndex, VkDeviceSize alignedUboSize) {
    UI_Element_UBO ubo;
    
    ubo.color = !uiElement->hovered ? uiElement->style.background.color : uiElement->style.background.hoverColor;

    VkDeviceSize offset = alignIndex * alignedUboSize;
    memcpy((void*)uniformBufferMapped+offset, &ubo, sizeof(ubo));
}

VkVertexInputBindingDescription ui_getVertexInputBindingDescription(){
    return (VkVertexInputBindingDescription) {
        .binding = 0,
        .stride = sizeof(UI_Vertex),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
    };
}

void ui_getVertexInputAttributeDescriptions(u32* outCount, VkVertexInputAttributeDescription** outAttribDescs){
    *outCount = 2;
    *outAttribDescs = (VkVertexInputAttributeDescription*)malloc(sizeof(VkVertexInputAttributeDescription) * (*outCount));
    (*outAttribDescs)[0].binding = 0;
    (*outAttribDescs)[0].location = 0;
    (*outAttribDescs)[0].format = VK_FORMAT_R32G32_SFLOAT;
    (*outAttribDescs)[0].offset = offsetof(Vertex, pos);

    (*outAttribDescs)[1].binding = 0;
    (*outAttribDescs)[1].location = 1;
    (*outAttribDescs)[1].format = VK_FORMAT_R32G32_SFLOAT;
    (*outAttribDescs)[1].offset = offsetof(Vertex, texCoord);
}