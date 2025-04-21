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
#include "assets/loaders/font_loader.h"

#include "renderer/details/vertexBuffer.h"
#include "renderer/details/indexBuffer.h"

#include <string/str.h>

// TODO: temporarry
#include <math/mat.h>
#include <math/vec2.h>
#include <math/vec4.h>
#include <math/trigonometry.h>
#include "game.h"

#include <collections/DynamicArray.h>

typedef struct UI_renderer_system_state_t
{
    const Renderer *r;
    Pipeline graphicsPipeline;
} UI_renderer_system_state;

typedef struct UI_renderer_element_state_t
{
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;
    u32 indicesCount;
} UI_renderer_element_state;

static void start(void *_state, void *gState);
static void start_entity(void *_state, void *gState, EntityID e);
static void pre_update(void *_state, void *gState);
static void pre_update_entity(void *_state, void *gState, EntityID e);
static void update(void *_state, void *gState);
static void update_entity(void *_state, void *gState, EntityID e);
static void destroy(void *_state, void *gState);
static void destroy_entity(void *_state, void *gState, EntityID e);

VkVertexInputBindingDescription ui_getVertexInputBindingDescription();
void ui_getVertexInputAttributeDescriptions(u32 *outCount, VkVertexInputAttributeDescription **outAttribDescs);

static void createDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout *out);
static void createDescriptorPool(VkDevice device, VkDescriptorPool *out);
static void createDescriptorSets(VkDevice device, VkDescriptorSetLayout setLayout, VkDescriptorPool pool, VkDescriptorSet *outDescriptorSets);
static void updateGlobalUniformBuffer(uint32_t currentImage, void **uniformBuffersMapped, f64 deltatime, Camera *camera);
static void updateElementUniformBuffer(void *uniformBufferMapped, f64 deltatime, UI_Element *uiElement, u32 alignIndex, VkDeviceSize alignedUboSize);
static void updateUBOsDescriptorSets(VkDevice device, VkBuffer *globalUBOs, VkBuffer *elementUBOs, VkDeviceSize elementAlignmentSize, VkDescriptorSet *outDescriptorSets);
static void uploadElementTextures(VkDevice device, VkDescriptorSet *descriptorSet, UI_Element *elem);

System UI_renderer_get_system_ref(Scene *scene, Renderer *r)
{
    UI_renderer_system_state *s = malloc(sizeof(UI_renderer_system_state));
    s->graphicsPipeline = (Pipeline){0};
    s->r = r;
    return (System){
        .Signature = COMPONENT_TYPE(scene, Transform2D) | COMPONENT_TYPE(scene, UI_Element),
        .properties = SYSTEM_PROPERTY_HIERARCHY_UPDATE,
        .state = s,
        .callbacks = {
            .start = start,
            .startEntity = start_entity,
            .preUpdate = pre_update,
            .preUpdateEntity = pre_update_entity,
            .update = update,
            .updateEntity = update_entity,
            .destroy = destroy,
            .destroyEntity = destroy_entity}};
}

/////////////////////////////////////
/////////////////////////////////////
/////      IMPLEMENTATION     ///////
/////////////////////////////////////
/////////////////////////////////////

void start(void *_state, void *gState)
{
    // initialize the pipeline
    UI_renderer_system_state *state = _state;

    Renderer *r = &((GameState *)gState)->renderer;
    createDescriptorSetLayout(r->context->device, &state->graphicsPipeline.descriptorSetLayout);

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
        .subpass_index = 1};
    create_graphics_pipeline(
        r->context->device,
        "shaders/default/ui.vert.spv", "shaders/default/ui.frag.spv",
        r->context->swapchainExtent,
        r->context->msaaSamples,
        r->renderPass,
        state->graphicsPipeline.descriptorSetLayout,
        &config,
        &state->graphicsPipeline.pipelineLayout,
        &state->graphicsPipeline.ref);
    createUniformBuffers(r->context->gpu, r->context->device, sizeof(UI_Global_UBO), state->graphicsPipeline.uniform.global.buffers, state->graphicsPipeline.uniform.global.buffersMemory, state->graphicsPipeline.uniform.global.buffersMapped);
    createDynamicOffsetUniformBuffers(r->context->gpu, r->context->device, sizeof(UI_Element_UBO), state->graphicsPipeline.uniform.element.buffers, state->graphicsPipeline.uniform.element.buffersMemory, state->graphicsPipeline.uniform.element.buffersMapped, &state->graphicsPipeline.uniform.element.alignedUboSize);

    // TODO: move to appropriate system
    Asset *texture = load_asset("./../resources/textures/image.jpeg", "image");
    Texture *t = (Texture *)texture->data;
    state->graphicsPipeline.textureImage = t->image;
    state->graphicsPipeline.textureImageMemory = t->memory;
    state->graphicsPipeline.textureImageView = t->imageView;
    state->graphicsPipeline.textureSampler = t->sampler;

    createDescriptorPool(r->context->device, &state->graphicsPipeline.descriptorPool);
    createDescriptorSets(r->context->device, state->graphicsPipeline.descriptorSetLayout, state->graphicsPipeline.descriptorPool, state->graphicsPipeline.descriptorSets);
    updateUBOsDescriptorSets(r->context->device, state->graphicsPipeline.uniform.global.buffers, state->graphicsPipeline.uniform.element.buffers, state->graphicsPipeline.uniform.element.alignedUboSize, state->graphicsPipeline.descriptorSets);
}

void start_entity(void *_state, void *gState, EntityID e)
{
    // this is called on the start of the program , when it should be called on the creating of an entity
    UI_Element *elem = GET_COMPONENT(0, e, UI_Element);
    if (!elem->state)
    {
        elem->state = malloc(sizeof(UI_renderer_element_state));
        if (!elem->state)
        {
            LOG_ERROR("UI_renderer : failed to allocate memory for UI element state");
            return;
        }
    }
}

void pre_update(void *_state, void *gState)
{
}

void pre_update_entity(void *_state, void *gState, EntityID e)
{
    UI_renderer_system_state *state = _state;
    Renderer *r = &((GameState *)gState)->renderer;

    UI_Element *elem = GET_COMPONENT(0, e, UI_Element);
    uploadElementTextures(r->context->device, state->graphicsPipeline.descriptorSets, elem);
}

static u32 elementCounter;
void update(void *_state, void *gState)
{
    UI_renderer_system_state *state = _state;
    Renderer *r = &((GameState *)gState)->renderer;
    f32 dt = ((GameState *)gState)->clock.deltaTime;

    updateGlobalUniformBuffer(r->currentFrame, state->graphicsPipeline.uniform.global.buffersMapped, dt, &((GameState *)gState)->camera);

    vkCmdNextSubpass(r->context->commandBuffers[r->currentFrame], VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(r->context->commandBuffers[r->currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, state->graphicsPipeline.ref);
    vkCmdBindDescriptorSets(r->context->commandBuffers[r->currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, state->graphicsPipeline.pipelineLayout, 0, 1, &state->graphicsPipeline.descriptorSets[r->currentFrame], 1, (u32[]){0});
    elementCounter = 0;
}

void update_entity(void *_state, void *gState, EntityID e)
{
    UI_renderer_system_state *state = _state;
    Renderer *r = &((GameState *)gState)->renderer;
    Scene *scene = &((GameState *)gState)->scene;
    f32 dt = ((GameState *)gState)->clock.deltaTime;

    UI_Element *elem = GET_COMPONENT(scene, e, UI_Element);
    Transform2D *t = GET_COMPONENT(scene, e, Transform2D);
    Hierarchy *h = GET_COMPONENT(scene, e, Hierarchy);
    Transform2D *parentT = 0;

    UI_renderer_element_state *elem_state = elem->state;

    if (elem->state_is_dirty)
    {
        elem->state_is_dirty = false;
        if (elem->meshdata.vertices && elem->meshdata.indices)
        {
            createVertexBuffer(
                r->context->gpu,
                r->context->device,
                r->context->queue.graphics,
                r->context->commandPool,
                elem->meshdata.verticesCount, (void *)elem->meshdata.vertices, sizeof(UI_Vertex),
                &elem_state->vertexBuffer,
                &elem_state->vertexBufferMemory);
            createIndexBuffer(
                r->context->gpu,
                r->context->device,
                r->context->queue.graphics,
                r->context->commandPool,
                elem->meshdata.indicesCount, elem->meshdata.indices,
                &elem_state->indexBuffer,
                &elem_state->indexBufferMemory);
            elem_state->indicesCount = elem->meshdata.indicesCount;
            elem_state->vertexBufferMemory = elem_state->vertexBufferMemory;
            free(elem->meshdata.vertices);
            free(elem->meshdata.indices);
            elem->meshdata.vertices = 0;
            elem->meshdata.indices = 0;
            elem->meshdata.verticesCount = 0;
            elem->meshdata.indicesCount = 0;
        }
        else
        {
            LOG_ERROR("UI_renderer : system wants to update a UI element but no data are set");
        }
    }

    if (elem->id != e)
        elem->id = e;

    if (!elem_state || elem_state->indicesCount == 0 || !elem_state->vertexBuffer || !elem_state->indexBuffer)
        return;

    if (h && h->parent != INVALID_ENTITY)
        parentT = GET_COMPONENT(scene, h->parent, Transform2D);
    if (parentT)
    {
        Mat4 m = mat4_mul(parentT->mat, parentT->__local_mat);
        transform2D_update(t, &m);
    }
    else
    {
        transform2D_update(t, 0);
    }

    UI_PushConstant pc = {
        .model = mat4_mul(t->mat, t->__local_mat)};
    VkBuffer vertexBuffers[] = {elem_state->vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdPushConstants(
        r->context->commandBuffers[r->currentFrame],
        state->graphicsPipeline.pipelineLayout,
        VK_SHADER_STAGE_VERTEX_BIT,
        0,
        sizeof(UI_PushConstant),
        &pc);

    u32 dynamicOffset = elementCounter * state->graphicsPipeline.uniform.element.alignedUboSize;
    vkCmdBindDescriptorSets(r->context->commandBuffers[r->currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, state->graphicsPipeline.pipelineLayout, 0, 1, &state->graphicsPipeline.descriptorSets[r->currentFrame], 1, &dynamicOffset);

    updateElementUniformBuffer(state->graphicsPipeline.uniform.element.buffersMapped[r->currentFrame], dt, elem, elementCounter++, state->graphicsPipeline.uniform.element.alignedUboSize);

    vkCmdBindVertexBuffers(r->context->commandBuffers[r->currentFrame], 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(r->context->commandBuffers[r->currentFrame], elem_state->indexBuffer, 0, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(r->context->commandBuffers[r->currentFrame], elem_state->indicesCount, 1, 0, 0, 0);
}

void destroy(void *_state, void *gState)
{
    UI_renderer_system_state *state = _state;
    Renderer *r = &((GameState *)gState)->renderer;

    vkDeviceWaitIdle(r->context->device);
    destroyUniformBuffers(r->context->device, state->graphicsPipeline.uniform.global.buffers, state->graphicsPipeline.uniform.global.buffersMemory);
    destroyUniformBuffers(r->context->device, state->graphicsPipeline.uniform.element.buffers, state->graphicsPipeline.uniform.element.buffersMemory);

    destroyPipeline(r->context->device, &state->graphicsPipeline.ref, state->graphicsPipeline.pipelineLayout);
    destroyDescriptorPool(r->context->device, state->graphicsPipeline.descriptorPool);
    destroyDescriptorSetLayout(r->context->device, state->graphicsPipeline.descriptorSetLayout);
}
static void destroy_entity(void *_state, void *gState, EntityID e)
{
    // UI_renderer_system_state* state = _state;
    Renderer *r = &((GameState *)gState)->renderer;
    Scene *scene = &((GameState *)gState)->scene;

    UI_Element *elem = GET_COMPONENT(scene, e, UI_Element);
    if (elem->state)
    {
        UI_renderer_element_state *elem_state = elem->state;
        vkDeviceWaitIdle(r->context->device);
        destroyVertexBuffer(r->context->device, elem_state->vertexBuffer, elem_state->vertexBufferMemory);
        destroyIndexBuffer(r->context->device, elem_state->indexBuffer, elem_state->indexBufferMemory);
        free(elem_state);
        elem->state = 0;
    }
}
///////////////////////////////
///////////////////////////////
/////      INTERNAL     ///////
///////////////////////////////
///////////////////////////////

#define samplersCount 100
#define bindingsCount 3

static void addTextureToDescriptorSet(
    VkDevice device,
    VkDescriptorSet *descriptorSet,
    Texture *texture)
{
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        VkDescriptorImageInfo imageInfo = {
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            .imageView = texture->imageView,
            .sampler = texture->sampler};

        VkWriteDescriptorSet write = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = descriptorSet[i],
            .dstBinding = 2,
            .dstArrayElement = texture->Idx,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo = &imageInfo};

        vkUpdateDescriptorSets(device, 1, &write, 0, 0);
    }
}
void createDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout *out)
{
    VkDescriptorSetLayoutBinding global_uboLayoutBinding = {
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .pImmutableSamplers = 0};
    VkDescriptorSetLayoutBinding element_uboLayoutBinding = {
        .binding = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
    };
    VkDescriptorSetLayoutBinding samplerLayoutBinding = {
        .binding = 2,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = samplersCount,
        .pImmutableSamplers = 0,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT};

    VkDescriptorSetLayoutBinding bindings[bindingsCount] = {global_uboLayoutBinding, element_uboLayoutBinding, samplerLayoutBinding};

    VkDescriptorSetLayoutCreateInfo layoutInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = bindingsCount,
        .pBindings = bindings,
        .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT};

    VkResult res = vkCreateDescriptorSetLayout(device, &layoutInfo, 0, out);
    if (res != VK_SUCCESS)
    {
        LOG_FATAL("failed to create descriptor set layout!");
    }
}

void createDescriptorPool(VkDevice device, VkDescriptorPool *out)
{
    VkDescriptorPoolSize poolSizes[bindingsCount] = {
        // 0 frame UBO
        {
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = MAX_FRAMES_IN_FLIGHT},
        // 1 - element UBO
        {
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
            .descriptorCount = MAX_FRAMES_IN_FLIGHT},
        // 2 - samplers
        {
            .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = samplersCount * MAX_FRAMES_IN_FLIGHT}};

    VkDescriptorPoolCreateInfo poolInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .poolSizeCount = bindingsCount,
        .pPoolSizes = poolSizes,
        .maxSets = MAX_FRAMES_IN_FLIGHT,
        .flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT};

    VkResult res = vkCreateDescriptorPool(device, &poolInfo, 0, out);
    if (res != VK_SUCCESS)
    {
        LOG_FATAL("failed to create descriptor pool!");
    }
}

void updateUBOsDescriptorSets(VkDevice device, VkBuffer *globalUBOs, VkBuffer *elementUBOs, VkDeviceSize elementAlignmentSize, VkDescriptorSet *outDescriptorSets)
{
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        VkDescriptorBufferInfo globalBuffInfo = {
            .buffer = globalUBOs[i],
            .offset = 0,
            .range = sizeof(UI_Global_UBO)};
        VkDescriptorBufferInfo elementBuffInof = {
            .buffer = elementUBOs[i],
            .offset = 0,
            .range = elementAlignmentSize};
        VkWriteDescriptorSet descriptorWrites[2] = {};
        descriptorWrites[0] = (VkWriteDescriptorSet){
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = outDescriptorSets[i],
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
            .pBufferInfo = &globalBuffInfo};
        descriptorWrites[1] = (VkWriteDescriptorSet){
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = outDescriptorSets[i],
            .dstBinding = 1,
            .dstArrayElement = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
            .descriptorCount = 1,
            .pBufferInfo = &elementBuffInof};

        vkUpdateDescriptorSets(device, 2, descriptorWrites, 0, 0);
    }
}

void createDescriptorSets(VkDevice device, VkDescriptorSetLayout setLayout, VkDescriptorPool pool, VkDescriptorSet *outDescriptorSets)
{
    VkDescriptorSetLayout layouts[MAX_FRAMES_IN_FLIGHT];
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        layouts[i] = setLayout;
    }

    VkDescriptorSetAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = pool,
        .descriptorSetCount = MAX_FRAMES_IN_FLIGHT,
        .pSetLayouts = layouts};

    VkResult res = vkAllocateDescriptorSets(device, &allocInfo, outDescriptorSets);
    if (res != VK_SUCCESS)
    {
        LOG_FATAL("failed to allocate descriptor sets!");
    }
}

// TODO: temp, find a more logical place
static bool loaded[samplersCount] = {0};
void uploadElementTextures(VkDevice device, VkDescriptorSet *descriptorSet, UI_Element *elem)
{
    // upload material textures if not uploaded already
    if (elem->properties.font)
    {
        GlyphSet *set = get_glyphset(elem->properties.font, elem->style.text.size);
        if (!set)
        {
            LOG_ERROR("UI_renderer : font is not loaded");
            return;
        }
        if (!loaded[((Texture *)set->atlas)->Idx] || ((Texture*)set->atlas)->is_dirty)
        {
                ((Texture*)set->atlas)->is_dirty = false;
                addTextureToDescriptorSet(device, descriptorSet, set->atlas);
                loaded[((Texture *)set->atlas)->Idx] = true;
        }
    }
}

void updateGlobalUniformBuffer(uint32_t currentImage, void **uniformBuffersMapped, f64 deltatime, Camera *camera)
{
    UI_Global_UBO ubo;

    // View matrix remains identity for screen-space UI
    Vec2 virtualCanvas = vec2_new(camera->viewRect.w, camera->viewRect.h);
    ubo.proj = mat4_orthographic(
        0.0f,
        virtualCanvas.x, // Left to Right = canvas width
        virtualCanvas.y, // Top to Bottom = canvas height (Vulkan Y-down)
        0.0f,
        -1.0f,
        1.0f);
    memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}

void updateElementUniformBuffer(void *uniformBufferMapped, f64 deltatime, UI_Element *uiElement, u32 alignIndex, VkDeviceSize alignedUboSize)
{
    UI_Element_UBO ubo;
    ubo.is_text = uiElement->type == UI_ELEMENT_TYPE_TEXT ? 1 : 0;
    if (ubo.is_text)
    {
        Font* font = get_asset(uiElement->style.text.fontName, ASSET_TYPE_FONT)->data;
        u32 atlas_index = get_glyphset(font, uiElement->style.text.size)->id;
        ubo.FontID = atlas_index;
    }
    else
    {
        ubo.FontID = -1;
    }
    ubo.color = !uiElement->hovered ? uiElement->style.background.color : uiElement->style.background.hoverColor;
    ubo.size = ui_element_get_final_size(uiElement);
    ubo.borderColor = !uiElement->hovered ? uiElement->style.border.color : uiElement->style.border.hoverColor;
    ubo.borderWidth = uiElement->style.border.thickness;

    VkDeviceSize offset = alignIndex * alignedUboSize;
    memcpy((void *)uniformBufferMapped + offset, &ubo, sizeof(ubo));
}

VkVertexInputBindingDescription ui_getVertexInputBindingDescription()
{
    return (VkVertexInputBindingDescription){
        .binding = 0,
        .stride = sizeof(UI_Vertex),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX};
}

void ui_getVertexInputAttributeDescriptions(u32 *outCount, VkVertexInputAttributeDescription **outAttribDescs)
{
    *outCount = 2;
    *outAttribDescs = (VkVertexInputAttributeDescription *)malloc(sizeof(VkVertexInputAttributeDescription) * (*outCount));
    (*outAttribDescs)[0].binding = 0;
    (*outAttribDescs)[0].location = 0;
    (*outAttribDescs)[0].format = VK_FORMAT_R32G32_SFLOAT;
    (*outAttribDescs)[0].offset = offsetof(UI_Vertex, pos);

    (*outAttribDescs)[1].binding = 0;
    (*outAttribDescs)[1].location = 1;
    (*outAttribDescs)[1].format = VK_FORMAT_R32G32_SFLOAT;
    (*outAttribDescs)[1].offset = offsetof(UI_Vertex, texCoord);
}