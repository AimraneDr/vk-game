#include "systems/UI_renderer.h"

#include "components/transform.h"
#include "components/Hierarchy.h"
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
#include "assets/asset_manager.h"

//TODO: temporarry
#include <math/mat.h>
#include <math/vec2.h>
#include <math/vec4.h>
#include <math/trigonometry.h>
#include "game.h"

#include <collections/DynamicArray.h>

typedef struct UI_renderer_InternalState_t{
    const Renderer* r;
    Pipeline graphicsPipeline;
    UI_Element** hovered_elems; //dynamic array

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
    s->hovered_elems = 0;
    return (System){
        .Signature = COMPONENT_TYPE(scene, Transform2D) | COMPONENT_TYPE(scene, UI_Element),
        .properties = SYSTEM_PROPERTY_HIERARCHY_UPDATE,
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
    state->hovered_elems = DynamicArray_Create(UI_Element*);
    
    Renderer* r = &((GameState*)gState)->renderer;
    ui_createDescriptorSetLayout(r->context->device, &state->graphicsPipeline.descriptorSetLayout);

    PipelineConfig config = {
        .get_vertex_binding_desc = ui_getVertexInputBindingDescription,
        .get_vertex_attr_descs = ui_getVertexInputAttributeDescriptions,
        .depth_test_enable = VK_TRUE,
        .depth_write_enable = VK_TRUE,
        .depth_compare_op = VK_COMPARE_OP_LESS,
        .push_constant_size = sizeof(UI_PushConstant),
        .push_constant_stage = VK_SHADER_STAGE_VERTEX_BIT,
        .src_color_blend_factor = VK_BLEND_FACTOR_SRC_ALPHA,
        .dst_color_blend_factor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .subpass_index = 1
    };
    create_graphics_pipeline(
        r->context->device,
        "shaders/default/ui.vert.spv", "shaders/default/ui.frag.spv",
        r->context->swapchainExtent,
        r->context->msaaSamples,
        r->renderPass,
        state->graphicsPipeline.descriptorSetLayout,
        &config,
        &state->graphicsPipeline.pipelineLayout,
        &state->graphicsPipeline.ref
    );
    createUniformBuffers(r->context->gpu, r->context->device, sizeof(UI_Global_UBO), state->graphicsPipeline.uniform.global.buffers, state->graphicsPipeline.uniform.global.buffersMemory, state->graphicsPipeline.uniform.global.buffersMapped);
    createDynamicOffsetUniformBuffers(r->context->gpu, r->context->device, sizeof(UI_Element_UBO), state->graphicsPipeline.uniform.element.buffers, state->graphicsPipeline.uniform.element.buffersMemory, state->graphicsPipeline.uniform.element.buffersMapped, &state->graphicsPipeline.uniform.element.alignedUboSize);

    //TODO: move to appropriate system
    Asset* texture = load_asset("./../resources/textures/image.jpeg", "image");
    Texture* t = (Texture*)texture->data;
    state->graphicsPipeline.textureImage = t->image;
    state->graphicsPipeline.textureImageMemory = t->memory;
    state->graphicsPipeline.textureImageView = t->imageView;
    state->graphicsPipeline.textureSampler = t->sampler;

    ui_createDescriptorPool(r->context->device, &state->graphicsPipeline.descriptorPool);
    ui_createDescriptorSets(r->context->device, state->graphicsPipeline.descriptorSetLayout, state->graphicsPipeline.descriptorPool, state->graphicsPipeline.uniform.global.buffers, state->graphicsPipeline.uniform.element.buffers, state->graphicsPipeline.uniform.element.alignedUboSize, state->graphicsPipeline.textureImageView, state->graphicsPipeline.textureSampler,state->graphicsPipeline.descriptorSets);
}

typedef struct onMouseMoveData_t{
    EntityID e;
    UI_renderer_InternalState* state;
}onMouseMoveData;

void checkMouseEvents(GameState* gState, EntityID e, UI_renderer_InternalState* state){
    InputManager* inputs = &gState->inputer;
    Scene* scene = &gState->scene;
    Camera* cam = &gState->camera;

    UI_Element* elem = GET_COMPONENT(scene, e, UI_Element);

    if(inputs->mouse.pos.x == -1){
        elem->hovered = false;
        return;
    } 

    Transform2D* transform = GET_COMPONENT(scene, e, Transform2D);
    Vec4 mouse_screen = vec4_new(inputs->mouse.pos.x/cam->pixelsPerPoint,inputs->mouse.pos.y/cam->pixelsPerPoint, 0, 1);
    Vec4 mouse_local = vec4_zero();
    
    Mat4 model_matrix = mat4_mul(transform->mat, transform->__local_mat);

    // Correct inverse for hit detection:
    Mat4 inv_model = mat4_inverse(model_matrix);
    mouse_local = mat4_mulVec4(inv_model, mouse_screen);
    
    //TODO: unless the width and height afects rendering do not use them in calculations
    Vec4 halfSize = vec4_new(elem->style.width/2.f, elem->style.height/2.f, 0, 1);
    if(mouse_local.x >= -halfSize.x && 
        mouse_local.x <= halfSize.x && 
        mouse_local.y >= -halfSize.y && 
        mouse_local.y <= halfSize.y)
    {
        if(!elem->hovered){
            elem->hovered = true;
            bool present_in_hovered = false;
            for(u8 i=0; i< DynamicArray_Length(state->hovered_elems); i++){
                if(state->hovered_elems[i]->id == elem->id){
                    present_in_hovered = true;
                    break;
                }
            }
            if(!present_in_hovered)DynamicArray_Push(state->hovered_elems, elem);
            if(elem->events.onMouseEnter)elem->events.onMouseEnter(gState, e, elem);
            return;
        }else{
            if(elem->events.onMouseStay)elem->events.onMouseStay(gState, e, elem);
            return;
        }
    }

    if(elem->hovered){
        elem->hovered = false;
        if(elem->events.onMouseLeave)elem->events.onMouseLeave(gState, e, elem);
        for(u8 i=0; i< DynamicArray_Length(state->hovered_elems); i++){
            if(state->hovered_elems[i]->id == elem->id){
                DynamicArray_PopAt(state->hovered_elems, i, 0);
            }
        }
        return;
    }
}

void start_entity(void* _state, void* gState, EntityID e){
    //this is called on the start of the program , when it should be called on the creating of an entity

}

static u32 elementCounter;
void update(void* _state, void* gState){
    UI_renderer_InternalState* state = _state;
    Renderer* r = &((GameState*)gState)->renderer;
    f32 dt = ((GameState*)gState)->clock.deltaTime;

    ui_updateGlobalUniformBuffer(r->currentFrame, state->graphicsPipeline.uniform.global.buffersMapped, dt, &((GameState*)gState)->camera);
    
    vkCmdNextSubpass(r->context->commandBuffers[r->currentFrame], VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(r->context->commandBuffers[r->currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, state->graphicsPipeline.ref);
    vkCmdBindDescriptorSets(r->context->commandBuffers[r->currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, state->graphicsPipeline.pipelineLayout, 0, 1, &state->graphicsPipeline.descriptorSets[r->currentFrame], 1, (u32[]){0});
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
    Hierarchy* h = GET_COMPONENT(scene, e, Hierarchy);
    Transform2D* parentT = 0;

    if(elem->id != e)elem->id = e;

    if(h && h->parent != INVALID_ENTITY) parentT = GET_COMPONENT(scene, h->parent, Transform2D);
    if(parentT){
        Mat4 m =mat4_mul(parentT->mat, parentT->__local_mat);
        transform2D_update(t, &m);
    }else{
        transform2D_update(t, 0);
    }

    //emit events
    //click
    //TODO: add elem checking, even if it is hovered that does not mean it is what is selected, a child elem can be selected instead
    checkMouseEvents(gState, e, _state);

    if(elem->hovered){
        bool topMost = true;
        u32 hovered_count = DynamicArray_Length(state->hovered_elems);
        for(u8 i=0; i< hovered_count; i++){
            EntityID id = state->hovered_elems[i]->id;
            Hierarchy* _h = GET_COMPONENT(scene, id, Hierarchy);
            if(_h && _h->depth_level > h->depth_level){
                topMost = false;
                break;
            }
        }
        if(topMost){
            if(is_key_down(inputs, MOUSE_BUTTON_LEFT)){
                elem->mouse_state.Lclicked = true;
                if(elem->events.onMouseLDown)elem->events.onMouseLDown(gState, e, elem);
            }
            if(is_key_up(inputs, MOUSE_BUTTON_LEFT)){
                elem->mouse_state.Lclicked = false;
                if(elem->events.onMouseLUp)elem->events.onMouseLUp(gState, e, elem);
            }
            
            if(is_key_down(inputs, MOUSE_BUTTON_RIGHT)){
                elem->mouse_state.Rclicked = true;
                if(elem->events.onMouseRDown)elem->events.onMouseRDown(gState, e, elem);
            }
            if(is_key_up(inputs, MOUSE_BUTTON_RIGHT)){
                elem->mouse_state.Rclicked = false;
                if(elem->events.onMouseRUp)elem->events.onMouseRUp(gState, e, elem);
            }
        }
    }

    if(elem->mouse_state.Lclicked && is_key_pressed(inputs, MOUSE_BUTTON_LEFT) && elem->events.onMouseLHold)elem->events.onMouseLHold(gState, e, elem);
    if(elem->mouse_state.Rclicked && is_key_pressed(inputs, MOUSE_BUTTON_RIGHT) && elem->events.onMouseRHold)elem->events.onMouseRHold(gState, e, elem);
    

    UI_PushConstant pc = {
        .model = mat4_mul(t->mat, t->__local_mat)
    };
    VkBuffer vertexBuffers[] = { elem->renderer.vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdPushConstants(
        r->context->commandBuffers[r->currentFrame],
        state->graphicsPipeline.pipelineLayout,
        VK_SHADER_STAGE_VERTEX_BIT,
        0,
        sizeof(UI_PushConstant),
        &pc
    );
    
    u32 dynamicOffset = elementCounter * state->graphicsPipeline.uniform.element.alignedUboSize;
    vkCmdBindDescriptorSets(r->context->commandBuffers[r->currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, state->graphicsPipeline.pipelineLayout, 0, 1, &state->graphicsPipeline.descriptorSets[r->currentFrame], 1, &dynamicOffset);
    
    ui_updateElementUniformBuffer(state->graphicsPipeline.uniform.element.buffersMapped[r->currentFrame], dt, elem ,elementCounter++, state->graphicsPipeline.uniform.element.alignedUboSize);
    
    vkCmdBindVertexBuffers(r->context->commandBuffers[r->currentFrame], 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(r->context->commandBuffers[r->currentFrame], elem->renderer.indexBuffer,0, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(r->context->commandBuffers[r->currentFrame], elem->renderer.indicesCount, 1, 0, 0, 0);
    
}

void destroy(void* _state, void* gState){
    UI_renderer_InternalState* state = _state;
    Renderer* r = &((GameState*)gState)->renderer;

    DynamicArray_Destroy(state->hovered_elems);
    vkDeviceWaitIdle(r->context->device);
    destroyUniformBuffers(r->context->device, state->graphicsPipeline.uniform.global.buffers, state->graphicsPipeline.uniform.global.buffersMemory);
    destroyUniformBuffers(r->context->device, state->graphicsPipeline.uniform.element.buffers, state->graphicsPipeline.uniform.element.buffersMemory);
    
    //TODO: move to appropriate system
    // destroyTextureSampler(r->context->device, state->grapgicsPipeline.textureSampler);
    // destroyTextureImageView(r->context->device, state->grapgicsPipeline.textureImageView);
    // destroyTextureImage(r->context->device, state->grapgicsPipeline.textureImage, state->grapgicsPipeline.textureImageMemory);
    //end todo

    destroyPipeline(r->context->device, &state->graphicsPipeline.ref, state->graphicsPipeline.pipelineLayout);
    destroyDescriptorPool(r->context->device, state->graphicsPipeline.descriptorPool);
    destroyDescriptorSetLayout(r->context->device, state->graphicsPipeline.descriptorSetLayout);
}

///////////////////////////////
///////////////////////////////
/////      INTERNAL     ///////
///////////////////////////////
///////////////////////////////
#define bindingsCount 3

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
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
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
    ubo.size = vec2_new(uiElement->style.width, uiElement->style.height);

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