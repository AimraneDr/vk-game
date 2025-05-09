#include "ecs/ecs.h"

#include <collections/DynamicArray.h>
#include <stdlib.h>
#include "core/debugger.h"
#include <string/str.h>
#include "core/events.h"

/// @brief the returned ComponentType needs to be stored so accessing its instance is possible
/// @param scene
/// @param size
/// @return
void ecs_register_component(Scene *scene, const char* name, u16 size)
{
    scene = scene? scene : ecs_get_active_scene();

    if(scene->componentTypesCount >= MAX_COMPONENT_TYPES){
        LOG_ERROR("cannot register component, you hit the max component types count");
        return;
    }
    ComponentPool* pool = &scene->pools[scene->componentTypesCount];
    pool->components = __DynamicArray_create(DARRAY_INITIAL_CAPACITY, size);
    pool->componentSize = size;
    pool->dense = DynamicArray_Create(EntityID);
    pool->sparse = malloc(sizeof(u16) * MAX_ENTITIES);
    pool->type = 1 << scene->componentTypesCount;
    for(u16 i=0; i < MAX_ENTITIES; i++){
        pool->sparse[i]= INVALID_ENTITY;
    }
    scene->componentNames[scene->componentTypesCount] = str_new(name);
    scene->componentTypesCount++;
}

ComponentType ecs_get_component_type(Scene* scene, const char* name) {
    scene = scene? scene : ecs_get_active_scene();
    
    for (u8 i = 0; i < scene->componentTypesCount; i++) {
        if (str_equals_val(scene->componentNames[i], name)) {
            return (u64)1 << i;
        }
    }
    return 0;
}

void *ecs_get_component(Scene* s, EntityID e, ComponentType t)
{
    s = s ? s : ecs_get_active_scene();

    if (!ecs_entity_has_component(s, e, t))
        return 0;
    ComponentPool* p = &s->pools[__builtin_ffsll(t) - 1];
    u16 index = p->sparse[e];
    if(index == INVALID_ENTITY || index >= DynamicArray_Length(p->components))
        return 0;
    return (char*)p->components + (index * p->componentSize);
};

void* ecs_add_component(Scene* s, EntityID e, ComponentType t, void *component)
{
    s = s ? s : ecs_get_active_scene();

    if((s->EntitiesSignatures[e] & t) == t){
        LOG_ERROR("entity already has component");
        return 0;
    }
    ComponentPool* p = &s->pools[__builtin_ffsll(t) - 1];
    p->sparse[e] = DynamicArray_Length(p->components);
    DynamicArray_Push(p->dense, e);
    p->components = __DynamicArray_push(p->components, component);
    s->oldEntitiesSignatures[e] = s->EntitiesSignatures[e];
    s->EntitiesSignatures[e] |= t;
    emit_event(EVENT_TYPE_COMPONENT_ADDED, (EventContext){.u64[0] = e, .u64[1] = t}, s);
    return (char*)p->components + (p->sparse[e] * p->componentSize);
}

void ecs_remove_component(Scene* s, EntityID e, ComponentType t)
{
    s = s ? s : ecs_get_active_scene();

    ComponentPool* p = &s->pools[__builtin_ffsll(t) - 1];
    DynamicArray_PopAt(p->dense, p->sparse[e], 0);
    DynamicArray_PopAt(p->components, p->sparse[e], 0);
    s->oldEntitiesSignatures[e] = s->EntitiesSignatures[e];
    if(ecs_entity_has_component(s, e, t)){
        s->EntitiesSignatures[e] ^= t;
        emit_event(EVENT_TYPE_COMPONENT_ADDED, (EventContext){.u64[0] = e, .u64[1] = t}, s);
    }
    p->sparse[e] = -1;
}

void ecs_remove_all_components(Scene* s, EntityID e)
{
    s = s ? s : ecs_get_active_scene();

    Mask m = s->EntitiesSignatures[e];
    while (m) {
        // find the first 1 bit from the right
        int pos = __builtin_ffsll(m) - 1;

        Mask c = (Mask)1 << pos;
        ecs_remove_component(s, e, c);

        // Clear this bit from signature
        m &= ~c;
    }
    s->EntitiesSignatures[e] = m;
}